import serial.tools.list_ports
import csv
import select

# Get a list of all available serial ports
ports = serial.tools.list_ports.comports()
csv_file = open('teste_1v1_perda_de_pacotes.csv', 'w', newline='')
csv_writer = csv.writer(csv_file)

# Create a list to hold the serial objects and data
ser_list = []
data_list = []

# Open the serial ports and add them to the list
for port in ports:
    ser = serial.Serial(port.device)
    ser_list.append(ser)

try:
    while True:
        # Create lists of input and output streams for select
        inputs = []
        outputs = []

        # Add all serial ports to the input list
        for ser in ser_list:
            inputs.append(ser)

        # Use select to check which ports have incoming data
        readable, _, _ = select.select(inputs, outputs, inputs)

        # Read data from the readable serial ports
        for ser in readable:
            try:
                data = ser.readline().decode('utf-8', errors='ignore').strip()
                data_list.append((data, ser.port))
                print(data_list[-1])
                csv_writer.writerow(data_list[-1])
            except UnicodeDecodeError:
                # Handle UnicodeDecodeError if necessary
                pass

except serial.SerialException as e:
    print(f"Error: {e}")

csv_file.flush()
csv_file.close()
