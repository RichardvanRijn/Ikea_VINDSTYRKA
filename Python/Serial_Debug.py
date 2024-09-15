import serial

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

                for x in range(packet_length):
                    # Read the rest of the packet based on the length
                    packet_data = ser.read(1)
                    # Append received data to the array
                    data_array.append(packet_data)

                print(f"Current data array: {data_array}")

                PM10 = (( int.from_bytes(data_array[1], byteorder='big') + (int.from_bytes(data_array[0], byteorder='big') * 256) )/ 10)
                print(f"PM1.0: {PM10}")
                
                PM25 = ((int.from_bytes(data_array[4], byteorder='big') + (int.from_bytes(data_array[3], byteorder='big') * 256) ) / 10)
                print(f"PM2.5: {PM25}")
                
                PM40 = ((int.from_bytes(data_array[7], byteorder='big') + (int.from_bytes(data_array[6], byteorder='big') * 256) ) / 10)
                print(f"PM4.0: {PM40}")
                
                PM100 = ((int.from_bytes(data_array[10], byteorder='big') + (int.from_bytes(data_array[9], byteorder='big') * 256) ) / 10)
                print(f"PM10: {PM100}")
                
                HM = ((int.from_bytes(data_array[13], byteorder='big') + (int.from_bytes(data_array[12], byteorder='big') * 256) ) / 100)
                print(f"Humidity: {HM}")

                TEMP = ((int.from_bytes(data_array[16], byteorder='big') + (int.from_bytes(data_array[15], byteorder='big') * 256) ) / 200)
                print(f"Temperature: {TEMP}")

                VOC = ((int.from_bytes(data_array[19], byteorder='big') + (int.from_bytes(data_array[18], byteorder='big') * 256) ) / 10)
                print(f"VOC: {VOC}")

                print(f"")
                clear_data_array()

    except serial.SerialException as e:
        print(f"Serial error: {e}")
    except KeyboardInterrupt:
        print("Program terminated by user.")

# Ensure that you provide the correct port for your system (e.g., COM3 on Windows or /dev/ttyUSB0 on Linux)
read_serial_packet(port='COM6', baudrate=115200, timeout=1)
