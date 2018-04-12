# Thermal design considerations
It is strongly suggested to thermally couple all power MOS-FETs along with a thermal fuse (F2 in schematic). For output currents in excess of 3A (for any particular channel) a proper heatsink may be required.

Measured temperatures on case of Q with a duty cycle of 98% @ 500Hz in free air.

| I<sub>load</sub> @12V (A) | T<sub>ambient</sub> (°C) | T<sub>Q</sub> (°C) | T<sub>diff</sub> (°K) |
|                      ---: |                     ---: |               ---: |                  ---: |
|                         1 |                     22,8 |               38,9 |                  16,1 |
|                         3 |                     22,8 |               55,5 |                  32,7 |
|                         4 |                     22,9 |               67,7 |                  44,8 |
|                         5 |                     22,9 |               83,0 |                  60,1 |
|                         6 |                     23,0 |              105,1 |                  82,1 |

If you choose not to thermally couple the FETs, one thermal fuse for each FET (tied to its body) between source and load shall be used. Same applies if the current rating of the fuse exceeds the total maximum output current of all channels combined.

In the unlikely event the gate of any FET becomes disconnected from both the port-pin and the pull-down resistor (due to a malfunction) and is therefore operating in an unknown state (i.e. above GND but below the FET's threshold voltage), it may have to dissipate a lot of heat. So the thermal fuse (F2) is recommended even for low current output ratings.