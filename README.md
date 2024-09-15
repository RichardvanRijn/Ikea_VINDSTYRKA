# Ikea VINDSTYRKA sensor analysis

This repo is a collection off all information I collected about the communication 

# I2C Bus data

The microcontroller communicates every second with the SEN54 sensor using an I2C bus.

##Bus data

The communications sequence is the same every time. 
<img src="I2C_Vindstyrka_PulseView_All.PNGg" width="500px">

The following data is send over the bus:

#####Stasrtup
0x69 write
0x00
0x21

#####Every second
0x69 write 
0x02
0x02

0x69 read
3 Byte read

0x69 write 
0x03
0xc4

0x69 read
21 Byte read

0x69 write 
0x03
0xD2

0x69 read
12 Byte read

0x69 write
0x03
0xF5

0x69 read
9 Byte read

## Data explanation

The sequence start with a write action folowed by a read actions
The write actions set the read pointer, the read actions retreives the data.

0x0021
Start Measurement

Write 0x0202
Set address pointer to Read Data-Ready Flag 

Read 3 Byte
Byte # Description
0 unused, always 0x00
1 Data-Ready Flag
0x00: no new measurements available
0x01: new measurements ready to read
2 Checksum for bytes 0, 1

Write 0x03C4 
Set address pointer to Read Measured Values 

Read 21 Byte
| Byte#  | Datatype                  | Scale factor | Description                          |
|--------|---------------------------|--------------|--------------------------------------|
| 0..1   | big-endian, uint16        | 10           | Mass Concentration PM1.0 [μg/m³]      |
| 2      | Checksum for bytes 0, 1   |              |                                      |
| 3..4   | big-endian, uint16        | 10           | Mass Concentration PM2.5 [μg/m³]     |
| 5      | Checksum for bytes 3, 4   |              |                                      |
| 6..7   | big-endian, uint16        | 10           | Mass Concentration PM4.0 [μg/m³]     |
| 8      | Checksum for bytes 6, 7   |              |                                      |
| 9..10  | big-endian, uint16        | 10           | Mass Concentration PM10 [μg/m³]      |
| 11     | Checksum for bytes 9, 10  |              |                                      |
| 12..13 | big-endian, int16         | 100          | Compensated Ambient Humidity [%RH]   |
| 14     | Checksum for bytes 12, 13 |              |                                      |
| 15..16 | big-endian, int16         | 200          | Compensated Ambient Temperature [°C] |
| 17     | Checksum for bytes 15, 16 |              |                                      |
| 18..19 | big-endian, int16         | 10           | VOC Index                            |
| 20     | Checksum for bytes 18, 19 |              |                                      |
| 21..22 | big-endian, int16         | 10           | NOx Index                            |
| 23     | Checksum for bytes 21, 22 |              |                                      |
(Byte 21-23 are not read because these are only usen in the SEN55 sensor)

Write 0x03D2
Unknown this address is not described in the publicly avalible datasheet
read 12 Byte 

Write 0x03F5
Unknown this address is not described in the publicly avalible datasheet
read 9 Byte 

The last two commands and reads are not in the publicly avalible datasheet, the datasheet described the SEN54-SDN-T and the IKEA VINDSTYRKA has the sensor labeled with SEN54-PON-T.

#STM32 / python code


# Contense of the data dump
#####I2C_Vindstyrka.sr
PulseView file of a I2C Packet captured with an osciloscope

#####I2C_Vindstyrka_PulseView_All.png
Picture of the I2C communication that occures every second

#####I2C_Vindstyrka_PulseView_Scope.PNG
Image of one packet in pulseview captured with an osciloscope

#####I2C_Vindstyrka_PulseView_Packet/png
Image of one packet in pulseview captured with an logic analyzer

#####Ikea_VINDSTYRKA_I2C.sal
Capture of of the I2C communication that occures every second in Saleae software

#####VINDSTYRKA_Pulseview.txt
Export of I2C data from PulseView