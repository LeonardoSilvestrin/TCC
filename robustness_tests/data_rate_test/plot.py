import csv
import matplotlib.pyplot as plt

def split_list(list):
    splits,negatives,temp = list()
    for num in list:
        if num < 0:
            if temp:
                splits.append(temp)
                temp = []
            negatives.append(num)
        else:
            temp.append(num)

    if temp:
        splits.append(temp)
    return (splits,negatives)



transmitter_data = []
receiver_data = []
x_values = []
y_values = []
with open('teste_1v1_perda_de_pacotes.csv', 'r') as csv_file:
    csv_reader = csv.reader(csv_file)
    i = 0
    for line in csv_reader:
        if i != 0:
            transmitter_data.append(int(line[0]))
            receiver_data.append(int(line[1]))
        else:
            i+=1
missed = 0
for pack in transmitter_data:
    if pack not in receiver_data:
        missed +=1

print(missed)
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
