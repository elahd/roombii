# Roombii

## Pins

| **Pin**    | **Factory**             | **Use**                   | **Note**                                                | **ADC Channel** |
| ---------- | ----------------------- | ------------------------- | ------------------------------------------------------- | --------------- |
| **A0/26**  |                         |                           |                                                         | 2               |
| **A1/25**  |                         | Front PCB - NeoPixels     |                                                         | 2               |
| **A2/34**  |                         |                           | Input Only                                              | 1               |
| **A3/39**  |                         |                           | Input Only                                              | 1               |
| **A4/36**  |                         |                           | Input Only                                              | 1               |
| **A5/4**   |                         | Front PCB - Button        |                                                         | 2               |
| **A13/35** | Feather Battery Voltage | Feather Battery Voltage   | Input Only. Not exposed. Voltage divided by 2.          |                 |
| **5**      | SCK                     | Front PCB - Speaker       |                                                         |                 |
| **12**     |                         | Bin Sensor                | Internal pulldown used for booting. Use as output only. | 2               |
| **13**     |                         | Bump Sensor               |                                                         | 2               |
| **14**     |                         | Speed Sensor - Left       |                                                         | 2               |
| **15**     |                         |                           |                                                         | 2               |
| **16**     | RX                      | RX                        |                                                         |                 |
| **17**     | TX                      | TX                        |                                                         |                 |
| **18**     | MOSI                    |                           |                                                         |                 |
| **19**     | MISO                    |                           |                                                         |                 |
| **21**     |                         | Wheel Lift Sensor - Left  |                                                         |                 |
| **22**     | SCL                     | SCL                       |                                                         |                 |
| **23**     | SDA                     | SDA                       |                                                         |                 |
| **27**     |                         | Wheel Lift Sensor - Right |                                                         |                 |
| **32**     |                         | Speed Sensor - Right      |                                                         | 1               |
| **33**     |                         |                           |                                                         | 1               |

ADC Channel 2: Can't take analog readings while Wi-Fi is on.

## Voltage Divider

- R1 = 100kΩ
- R2 = 10kΩ

vref = 1128ma measured 1135ma burned
battery dead at ~9V
TODO: Kill bluetooth controller code in loop when OTA.