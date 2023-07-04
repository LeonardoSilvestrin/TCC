import csv
import matplotlib.pyplot as plt

transmitter_data = []
receiver_data = []
x_values = []
y_values = []
with open('aaa.csv', 'r') as csv_file:
    csv_reader = csv.reader(csv_file)
    for line in csv_reader:
        x_values.append(int(line[0].split(',')[1][:-1]))
        y_values.append(int(line[1].split(',')[0][3:-1])/int(line[0].split(',')[0][2:]))

#for x,y in zip(transmitter_data, receiver_data):
#    x_values.append(int(x.split(',')[1]))
#    y_values.append(int(y)/int(x.split(',')[0])*100)
#    print(x,y)
#    print(x_values[-1],y_values[-1])
### Reverse the order of the data
x_values = x_values[::-1]
y_values = y_values[::-1]

# Plot the data
plt.plot(x_values, y_values, marker='o')
plt.xlabel('Time (microseconds)')
plt.ylabel('Package Loss (%)')
plt.title('Package Loss Percentage over Time')
plt.grid(True)
plt.show()
