import serial
import struct

# data packets from C are __attribute__((packed)) meaning there are no padding bytes
#21 byte
format21 = '>HBHBHBHBHBHBHB'
#12 byte
format12 = '>HBHBHBHB'
#9 bye
format9 = '>HBHBHB'
#3byte
format3 = '>HB'

def read_serial_packet(port='COM6', baudrate=115200, timeout=1):

    def clear_data_array():
#        Clear the data array.
        nonlocal data_array
        data_array = []
              
    data_array = []  # Initialize an empty array to store received packets

    try:
        # Open serial port
        ser = serial.Serial(port, baudrate, timeout=timeout)
        
        print(f"Listening on {port} with baudrate {baudrate}...")

        while True:
            if ser.in_waiting > 0:
                # Read the first byte which gives the length of the packet
                packet_length = ser.read(1)
                
                # Convert the byte to an integer
                packet_length = int.from_bytes(packet_length, byteorder='big')
                
                print(f"Expected packet length: {packet_length} bytes")
                # Read the rest of the packet based on the length
                ReadData = ser.read(packet_length)

                if packet_length == 21:
                    # Unpack the binary data to a tuple
                    size = struct.calcsize(format21)
                    unpacked_data = struct.unpack(format21, ReadData)
                    unpacked_data = (unpacked_data[0] / 10, unpacked_data[1], unpacked_data[2] / 10, unpacked_data[3], unpacked_data[4] / 10, unpacked_data[5], unpacked_data[6] / 10, unpacked_data[7], unpacked_data[8] / 100, unpacked_data[9], unpacked_data[10] / 200, unpacked_data[11], unpacked_data[12] / 10, unpacked_data[13])
                elif packet_length == 12:
                    # Unpack the binary data to a tuple
                    size = struct.calcsize(format12)
                    unpacked_data = struct.unpack(format12, ReadData)
                    unpacked_data = (unpacked_data[0], unpacked_data[1], unpacked_data[2], unpacked_data[3], unpacked_data[4], unpacked_data[5], unpacked_data[6], unpacked_data[7])
                elif packet_length == 9:
                    # Unpack the binary data to a tuple
                    size = struct.calcsize(format9)
                    unpacked_data = struct.unpack(format9, ReadData)
                    unpacked_data = (unpacked_data[0], unpacked_data[1], unpacked_data[2], unpacked_data[3], unpacked_data[4], unpacked_data[5])
                elif packet_length == 3:
                    # Unpack the binary data to a tuple
                    size = struct.calcsize(format3)
                    unpacked_data = struct.unpack(format3, ReadData)
                    unpacked_data = (unpacked_data[0], unpacked_data[1])

                print(unpacked_data)
                print(f"")
                clear_data_array()

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("Program terminated by user.")

# Ensure that you provide the correct port for your system (e.g., COM3 on Windows or /dev/ttyUSB0 on Linux)
read_serial_packet(port='COM6', baudrate=115200, timeout=1)
