/* 
  1-9 channel PWM controller for esp8266

  Copyright (c) 2018 Volker Schulz. All rights reserved.
  This file is part of the following GitHub project:
  https://github.com/volkerschulz/D1-Mini-Wi-Fi-RGBW-LED-Controller
 
  This sketch is free software. Please see the LICENSE file in the 
  root directory of the repository mentioned above for terms and 
  conditions.
*/
#include <ESP8266WiFi.h>

#define DEBUG 1
#define DEVICE "D1-Mini-Wi-Fi-RGBW-LED"
#define VERSION "1.0.0"
#define MAX_REMOTE_CLIENTS 2
#define TMR_COUNT 326
#define TCP_PORT 23

char ssid[40];
char password[70];

WiFiServer server(TCP_PORT);
WiFiClient server_clients[MAX_REMOTE_CLIENTS];

char command_buffer[255];
uint8_t command_index=0;

volatile byte pwm_counter=0;
volatile byte channels[] = {
  D0,  D1,  D2,  D3,  D4,  D5,  D6
};
volatile byte channel_values[] = {
  0,   0,   0,   0,   0,   0,   0
};

void ICACHE_RAM_ATTR pwmHandler (void){
  uint8_t i=0;
  pwm_counter++;
  
  if(pwm_counter==0) {
    for(i=0; i<sizeof(channels); i++) {
      if(channel_values[i]!=0) 
        digitalWrite(channels[i], HIGH);
      else
        digitalWrite(channels[i], LOW);
    }
  } else if(pwm_counter<255) {
    for(i=0; i<sizeof(channels); i++) {
      if(channel_values[i]==pwm_counter)
        digitalWrite(channels[i], LOW);
    }
  }
  
  timer0_write(ESP.getCycleCount() + TMR_COUNT);
}

void setup() {
  for(uint8_t i=0; i<sizeof(channels); i++) {
    pinMode(channels[i], OUTPUT);
  }
  
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(pwmHandler);
  interrupts();
    
  Serial.begin(115200);
  delay(10);
  
  printShortStatus(255);
  if(connectWiFi(false)) startServer();
}

void loop() {
  uint8_t i;
  //Check for connection attempts
  if (server.hasClient()){
    for(i = 0; i < MAX_REMOTE_CLIENTS; i++){
      if (!server_clients[i] || !server_clients[i].connected()){
        if(server_clients[i]) server_clients[i].stop();
        server_clients[i] = server.available();
        if(DEBUG) Serial.println("Client connected");
        printShortStatus(i);
        break;
      }
    }
    if(i == MAX_REMOTE_CLIENTS) {
      WiFiClient serverClient = server.available();
      serverClient.stop();
      if(DEBUG) Serial.println("Client could not connect, MAX_REMOTE_CLIENTS reached");
    }
  }
  //Check TCP/IP for data
  for(i = 0; i < MAX_REMOTE_CLIENTS; i++){
    if (server_clients[i] && server_clients[i].connected()){
      if(server_clients[i].available()){
        while(server_clients[i].available()) {
          byte incoming = server_clients[i].read();
          if(incoming==13 || incoming==10) {
            if(command_index>0) {
              parseCommand(i);
            }
            clearCommandBuffer();
          } else {
            command_buffer[command_index] = incoming;
            command_index++;
          }
        }
      }
    }
  }
  //check UART for data
  if(Serial.available()){
    size_t len = Serial.available();
    uint8_t sbuf[len];
    Serial.readBytes(sbuf, len);
    for(i=0; i<len; i++) {
      if(sbuf[i]==13 || sbuf[i]==10) {
        if(command_index>0) {
          parseCommand(255);
        }
        clearCommandBuffer();
      } else {
        command_buffer[command_index] = sbuf[i];
        command_index++;
      }
    }
  }
}

bool connectWiFi(bool is_reconnect) {
  uint8_t i = 0;
  if(is_reconnect) {
    if(server.status()) {
      if(DEBUG) Serial.println("Stopping clients");
      for(i = 0; i < MAX_REMOTE_CLIENTS; i++){
        if(server_clients[i]) server_clients[i].stop();
      }
      if(DEBUG) Serial.println("Stopping server");
      server.stop();
      if(DEBUG) Serial.println("Closing server");
      server.close();
    }
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(100);
  } else {
    if(WiFi.SSID().length() == 0) {
      if(DEBUG) Serial.println("No SSID specified");
      return false;
    }
    WiFi.SSID().toCharArray(ssid, WiFi.SSID().length()+1);
    WiFi.psk().toCharArray(password, WiFi.psk().length()+1);
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(DEBUG) Serial.print("\nConnecting to "); Serial.println(ssid);
  ESP.wdtDisable();
  while (WiFi.status() != WL_CONNECTED && i++ < 40) {
    delay(500);
    if(DEBUG && i%2 == 0) Serial.print(".");
    ESP.wdtFeed();
  }
  Serial.println(".");
  ESP.wdtEnable(15000);
  if(WiFi.status() != WL_CONNECTED) {
    if(DEBUG) Serial.print("Could not connect to "); Serial.println(ssid);
    return false;
  }
  if(DEBUG) Serial.print("Connected to "); Serial.println(WiFi.SSID());
  return true;
}

void startServer() {
  server.begin();
  server.setNoDelay(true);
  
  if(DEBUG) Serial.print("Ready for connections to ");
  if(DEBUG) Serial.print(WiFi.localIP());
  if(DEBUG) Serial.println(":" + String(TCP_PORT));
}

void clearCommandBuffer() {
  for(uint8_t i=0; i<sizeof(command_buffer); i++) {
    command_buffer[i]=0;
  }
  command_index=0;
}

void parseCommand(uint8_t sender) {
  char* command = strtok(command_buffer, "&");
  while (command != NULL) {
    if(DEBUG) Serial.print("Command received: "); Serial.println(command);
    char* separator = strchr(command, '=');
    if (separator != NULL) {
      *separator = 0;
      if(atoi(command) != 0 || strcmp("0", command) == 0) {
        byte channel_id = atoi(command);
        ++separator;
        byte channel_value = atoi(separator);
        if(DEBUG) Serial.print("Value is: ");  Serial.println(channel_value);
        if(channel_id < sizeof(channel_values) && (atoi(separator) != 0 || strcmp("0", separator) == 0)) {
          channel_values[channel_id]=channel_value;
          Serial.println("Channel " + String(channel_id) + " set to " + String(channel_value));
          tcpPrintLn(String(channel_id) + "=" + String(channel_value));
        } else {
          if(sender == 255) Serial.println("Illegal channel or value!");
          if(sender < sizeof(server_clients)) 
            server_clients[sender].println("ERROR=ILLEGAL_CHANNEL_OR_VALUE");
        }
      } else {
        ++separator;
        if(strcmp("ssid", command) == 0) {
          strcpy(ssid, separator);
          if(sender == 255) Serial.println("New SSID set");
          if(sender < sizeof(server_clients)) 
            server_clients[sender].print("SSID="); server_clients[sender].println(ssid); 
        } else if(strcmp("password", command) == 0) {
          strcpy(password, separator);
          if(sender == 255) Serial.println("New password set");
          if(sender < sizeof(server_clients)) 
            server_clients[sender].print("PASSWORD="); server_clients[sender].println(password);
        } else {
          Serial.println("Unknown command!");
          if(sender < sizeof(server_clients)) 
            server_clients[sender].println("ERROR=UNKNOWN_COMMAND");
        }
      }
    } else {
      if(strcmp("connect", command) == 0) {
        if(connectWiFi(true)) startServer();
      } else if(strcmp("s", command) == 0) {
        printStatus(sender);
      } else if(strcmp("restart", command) == 0) {
        ESP.wdtDisable();
        while (1){};
      } else {
        Serial.println("Unknown command!");
        if(sender < sizeof(server_clients)) 
            server_clients[sender].println("ERROR=UNKNOWN_COMMAND");
      }
    }
    command = strtok(NULL, "&");
  }
}

void printShortStatus(uint8_t sender) {
  uint8_t i = 0;
  if(sender == 255) {
    Serial.print("DEVICE="); Serial.println(DEVICE);
    Serial.print("VERSION="); Serial.println(VERSION);
    Serial.print("SERIAL="); Serial.println(ESP.getChipId(), HEX); 
    for(i=0; i<sizeof(channels); i++) {
      Serial.print(i); Serial.print("="); Serial.println(channel_values[i]); 
    }   
  }
  if(sender < sizeof(server_clients)) {
    server_clients[sender].print("DEVICE="); server_clients[sender].println(DEVICE);
    server_clients[sender].print("VERSION="); server_clients[sender].println(VERSION);
    server_clients[sender].print("SERIAL="); server_clients[sender].println(ESP.getChipId(), HEX);    
    for(i=0; i<sizeof(channels); i++) {
      server_clients[sender].print(i); server_clients[sender].print("="); server_clients[sender].println(channel_values[i]); 
    }
  }
}

void printStatus(uint8_t sender) {
  printShortStatus(sender);
  if(sender == 255) {
    Serial.print("SSID="); Serial.println(ssid);
    Serial.print("SAVED_SSID="); Serial.println(WiFi.SSID());
    if(DEBUG) Serial.print("PASSWORD="); Serial.println(password);
    if(DEBUG) Serial.print("SAVED_PASSWORD="); Serial.println(WiFi.psk());
    Serial.print("WIFI=");
    switch(WiFi.status()) {
      case WL_CONNECTED:
        Serial.println("CONNECTED");
        Serial.print("IP="); Serial.println(WiFi.localIP());
        Serial.print("PORT="); Serial.println(TCP_PORT);
        Serial.print("SUBNET="); Serial.println(WiFi.subnetMask());
        Serial.print("GATEWAY="); Serial.println(WiFi.gatewayIP());
        Serial.print("DNS="); Serial.println(WiFi.dnsIP());
        Serial.print("HOSTNAME="); Serial.println(WiFi.hostname());
        Serial.print("BSSID="); Serial.println(String(WiFi.BSSIDstr()));
        Serial.print("RSSI="); Serial.println(WiFi.RSSI());
        break;
      case WL_NO_SSID_AVAIL:
        Serial.println("DISCONNECTED_UNREACH");
        break;
      case WL_CONNECT_FAILED:
        Serial.println("DISCONNECTED_FAILED");
        break;
      case WL_IDLE_STATUS:
        Serial.println("DISCONNECTED_IDLE");
        break;
      case WL_DISCONNECTED:
        Serial.println("DISCONNECTED");
        break;
      default:
        Serial.println("UNKNOWN");
        break;
    }
    Serial.print("AUTO_CONNECT="); WiFi.getAutoConnect() ? Serial.println("ON") : Serial.println("OFF");
  } else if(sender < sizeof(server_clients)) {
    server_clients[sender].print("SSID="); server_clients[sender].println(ssid);
    server_clients[sender].print("SAVED_SSID="); server_clients[sender].println(WiFi.SSID());
    server_clients[sender].print("WIFI=");
    switch(WiFi.status()) {
      case WL_CONNECTED:
        server_clients[sender].println("CONNECTED");
        server_clients[sender].print("IP="); server_clients[sender].println(WiFi.localIP());
        server_clients[sender].print("PORT="); server_clients[sender].println(TCP_PORT);
        server_clients[sender].print("SUBNET="); server_clients[sender].println(WiFi.subnetMask());
        server_clients[sender].print("GATEWAY="); server_clients[sender].println(WiFi.gatewayIP());
        server_clients[sender].print("DNS="); server_clients[sender].println(WiFi.dnsIP());
        server_clients[sender].print("HOSTNAME="); server_clients[sender].println(WiFi.hostname());
        server_clients[sender].print("BSSID="); server_clients[sender].println(String(WiFi.BSSIDstr()));
        server_clients[sender].print("RSSI="); server_clients[sender].println(WiFi.RSSI());
        break;
      case WL_NO_SSID_AVAIL:
        server_clients[sender].println("DISCONNECTED_UNREACH");
        break;
      case WL_CONNECT_FAILED:
        server_clients[sender].println("DISCONNECTED_FAILED");
        break;
      case WL_IDLE_STATUS:
        server_clients[sender].println("DISCONNECTED_IDLE");
        break;
      case WL_DISCONNECTED:
        server_clients[sender].println("DISCONNECTED");
        break;
      default:
        server_clients[sender].println("UNKNOWN");
        break;
    }
    server_clients[sender].print("AUTO_CONNECT="); WiFi.getAutoConnect() ? server_clients[sender].println("ON") : server_clients[sender].println("OFF");
  }
}

void tcpPrint(String ln) {
  uint8_t i = 0;
  for(i = 0; i < MAX_REMOTE_CLIENTS; i++){
    if (server_clients[i] && server_clients[i].connected()){
      server_clients[i].print(ln);
    }
  }
}

void tcpPrintLn(String ln) {
  uint8_t i = 0;
  for(i = 0; i < MAX_REMOTE_CLIENTS; i++){
    if (server_clients[i] && server_clients[i].connected()){
      server_clients[i].println(ln);
    }
  }
}


