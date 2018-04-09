# Thermal design considerations
It is strongly suggested to thermally couple all power MOS-FETs along with a thermal fuse (F2 in schematic). For output currents in excess of 3A (for one particular channel) a proper heatsink may be required.

Measured temperatures on case of Q at a duty cycle of 98% @ 500Hz

| I<sub>load</sub> @12V (A) | T<sub>ambient</sub> (°C) | T<sub>Q</sub> (°C) | T<sub>diff</sub> (°K) |
|                      ---: |                     ---: |               ---: |                  ---: |
|                         1 |                     22,8 |               38,9 |                  16,1 |
|                         3 |                     22,8 |               55,5 |                  32,7 |
|                         4 |                     22,9 |               67,7 |                  44,8 |
|                         5 |                     22,9 |               83,0 |                  60,1 |
|                         6 |                     23,0 |              105,1 |                  82,1 |
