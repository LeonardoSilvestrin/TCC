import serial.tools.list_ports

# Get a list of all available serial ports
ports = serial.tools.list_ports.comports()

# Iterate through each port and try reading data_list
ser_list = []
data_list = []
for port in ports:
    ser_list.append(serial.Serial(port.device))
# Read and print data_list from the serial port
try:
    # Open the serial port
    while True:
        try:
            for ser in ser_list:
                data_list.append((ser.readline().decode('utf-8', errors='ignore').strip(),ser.port))
            for data in data_list:
                print(data[0])
            print(10*'-')
        except UnicodeDecodeError:
            # Handle UnicodeDecodeError if necessary
            pass 
except serial.SerialException:
    print(f"Error opening serial port {port.device}")
