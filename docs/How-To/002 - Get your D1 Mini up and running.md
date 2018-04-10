How-to...
# Get your D1 Mini up and running

The D1 Mini uses the CH341 chipset for serial over USB communication. If you've never before connected a device using this particular chipset, you might have to install drivers first by following these steps for your operating system:

### Windows
1. Connect your D1 Mini to a USB port of your PC using a micro USB cable.
2. The driver should install automatically, however it is not uncommon to fail. To verify the driver installation, start the "Device Manager" by pressing "Windows+R", entering "devmgmt.msc" and hitting "OK".
3. If you see a device named "USB2.0-Serial" (or similar) with a exclamation mark, the installation most likely failed. Right click that item and click "Update driver". Then select "Search automatically for updated driver software". This might take a while. When done, click "Close".
4. If the exclamation mark is now gone and you see a device name similar to "USB-SERIAL CH340 (COMX)" under "Ports (COM & LPT)" everything is fine and you can proceed to the section "All operating systems". If the driver installation is still faulty, disconnect your D1 Mini, download and install the drivers from http://www.wch.cn/download/CH341SER_ZIP.html and restart with step 1.

### Mac
1. Connect your D1 Mini to a USB port of your Mac using a micro USB cable.
2. Open the terminal (Press "CMD+Space" to open spotlight search, type "terminal" and hit "return"), then enter "ls /dev/tty*" (without the quotes, of course) and hit return. If the following list contains an item similar to "/dev/tty.wchusbserial" the drivers are correctly installed and you can proceed to the section "All operating systems".
3. If the item is missing, disconnect your D1 Mini, then download and install the drivers from http://www.wch.cn/download/CH341SER_MAC_ZIP.html and restart with step 1.

### Linux
TBD

### All operating systems
1. With the D1 Mini being connected to your computer launch the Arduino IDE (if it's not running already).
2. Navigate to "Tools / Port" and select the Port your D1 Mini is connected to.
3. Navigate to "Tools / Board" and select "WeMos D1 R2 & mini".
4. Navigate to "Tools" and verify (or correct) the following settings
	- Flash Size: 4M (3M SPIFFS)
	- Debug Port: "Disabled"
	- Debug Level: "None"
	- IwIP Variant: v2 Lower Memory
	- CPU Frequency: 80 MHz
	- Upload Speed: 115200
	- Erase Flash: "Only Sketch"
5. Navigate to "File / Examples / ESP8266 / Blink" to load a sample sketch (might open in a new instance).
6. Click the "Upload" button or hit "Ctrl+U" if you're using windows or "Cmd+U" on a Mac respectively. The sample sketch should now compile and being uploaded to the board. If everything worked out as it should, the LED on your board should start blinking (one second on; two seconds off).

That's it. 