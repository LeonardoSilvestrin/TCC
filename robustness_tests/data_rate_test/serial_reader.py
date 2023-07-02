import serial.tools.list_ports

# Get a list of all available serial ports
ports = serial.tools.list_ports.comports()

# Iterate through each port and try reading data
try:
    # Open the serial port
    ser1 = serial.Serial(ports[0].device)
    ser2 = serial.Serial(ports[1].device)
    
    # Read and print data from the serial port
    while True:
        try:
            data1 = ser1.readline().decode('utf-8', errors='ignore').strip()
            data2 = ser2.readline().decode('utf-8', errors='ignore').strip()
            if data1 or data2:
                print(f"Data received from {ports[0].device}: {data1} and {ports[1].device}: {data2}")
        
        except UnicodeDecodeError:
            # Handle UnicodeDecodeError if necessary
            print(f"UnicodeDecodeError occurred while decoding data from {port.device}")

except serial.SerialException:
    print(f"Error opening serial port {port.device}")
