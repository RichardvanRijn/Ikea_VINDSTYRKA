# Ikea VINDSTYRKA sensor analysis

This repo is a collection off all information and observations I gathered about the Ikea VINDSTYRKA sensor.

It is mostly focused on the I2C bus and the sensor.

# I2C Bus data

The microcontroller communicates every second with the SEN54 sensor using an I2C bus.

## Bus data

The communications sequence is the same every time. 

<img src="Data_Dump\I2C_Vindstyrka_PulseView_All.PNG" width="500px">

The following data is send during a normal sequence:

### At Startup
0x69 write
0x00
0x21

### Every second
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

If there are no new measurements avalible the rest of the sequence is not transmitted and the controller waits for the next second.

Write 0x03C4 
Set address pointer to Read Measured Values. 

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

(Byte 21-23 are not read because these are only used in the SEN55 sensor)

Write 0x03D2
Unknown this address is not described in the publicly avalible datasheet.
read 12 Byte 

Write 0x03F5
Unknown this address is not described in the publicly avalible datasheet.
read 9 Byte 

The last two commands and reads are not in the publicly avalible datasheet, the datasheet described the SEN54-SDN-T and the IKEA VINDSTYRKA has the sensor labeled with SEN54-PON-T.

## STM32 / python code

I use a nucleo-F303RE because that is what i have avalible to me right now and i have previous experience with STM32 chips. it also has a build in programmer / debugger with a VCP directly available.

The STM32 code uses Pin PA8(SDA) and PA9(SCL) in external interupt mode to function as the inputs for the software I2C sniffer.
I made the software I2C sniffer specificly for the Ikea VINDSTYRKA, this means is operate on several assumption:

1. During a normal start bit the SCL line goes from high to low the SDA line should be low, in the VINDSTYRKA this is not the case. The SDA line is already rising when the SCL line is falling.
2. The last byte of a read transmission is ended with a nack followed by the stop bit
3. Repeated start is not used so i did not impliment it
4. Write blocks are ignored

The python software oppens the com port and receives the data. there is no propper synchronization between any of the serial communication.
Python receives the data and calculates the values for the Measured Values packet and prints the values of all received packets
The first byte received is asumed to be the size byte and receives the packed based on this size.
If the python code starts during a packet transmissing the receive wil g oout of sync an receive useless data.

## My observations
Byte 0 - 5 of the 12 unknown bytes are always the same as Byte 0 - 5 of the 9 unknown bytes.
Byte 9 - 11 of the unknown 12 Byte packet always seem to be 0xFF 0xFF 0xAC

During my testing i also noted that the temperature and the Humidity on the display do not always correspond with the values i get from the sensor. first i thought this might be some read erros because i use a questionable software implimentation of a I2C sniffer, but I get the same results with my logic analyzer.

after turning the VINDSTYRKA of and back on again the values correspond again. I also do use a heatgun to make the values change quite quickly.

After the reseting the senor the values on the screen and de analyzer drift appart rapidly (within one minute)

# Contense of the data dump
#### I2C_Vindstyrka.sr
PulseView file of a I2C Packet captured with an osciloscope

#### I2C_Vindstyrka_PulseView_All.png
Picture of the I2C communication that occures every second

#### I2C_Vindstyrka_PulseView_Scope.PNG
Image of one packet in pulseview captured with an osciloscope

#### I2C_Vindstyrka_PulseView_Packet/png
Image of one packet in pulseview captured with an logic analyzer

#### Ikea_VINDSTYRKA_I2C.sal
Capture of of the I2C communication that occures every second in Saleae software

#### VINDSTYRKA_Pulseview.txt
Export of I2C data from PulseView

# Used equipment/Software

[Generic USB Logic analyzer](https://sigrok.org/wiki/128axc-based_USBee_AX-Pro_clone "Logic Analyzer") (Looks like this)

[Rigol DS1054Z oscilloscope](https://eu.rigol.com/products/detail/DS1000Z.html "Rigol DS1054Z")

[STM32 Nucleo-F303RE](https://www.st.com/en/evaluation-tools/nucleo-f303re.html "STM32-Nulceo-F303RE")

[PulseView](https://sigrok.org/wiki/Main_Page "PulseView") (Great free signal analysis software)

[Python](https://www.python.org/ "Python") (Great scripting language)

[Saleae](https://www.saleae.com/ "Saleaeaeaeaeaeae") Logic 2 (Somehow the USB analyzer works with the saleae software, shoutout to saleae for making amazing software. If you ever need a professional logic analyzer buy a saleae Logic pro 8 easily the best logic analyzer I ever used.)

# ToDO

Improve the communication between the Nucleo and Python

