import csv
import matplotlib.pyplot as plt

def print_avg_test(file, fig_name):
    transmitter_data = dict()
    receiver_data = dict()
    x_values = list()
    y_values = list()

    with open(file, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        for line in csv_reader:
            if (line[0] == ''):
                pass
            else:
                if line[1] == '/dev/ttyUSB0':
                    pacotes_enviados = [int(num) for num in line[0].split(',')[:-1]]
                    transmitter_data[pacotes_enviados[0]]=len(pacotes_enviados)
                if line[1] == '/dev/ttyUSB1':
                    pacotes_recebidos = [int(num) for num in line[0].split(',')[:-1]]
                    receiver_data[pacotes_recebidos[0]] = len(pacotes_recebidos)+1


    x_values = list(transmitter_data.keys())
    for value in x_values:
        y_values.append((1-receiver_data[value]/transmitter_data[value])*100)

    # Calculate the average
    average = sum(y_values) / len(y_values)

    # Plotting the bar graph
    fig, ax = plt.subplots(figsize=(15, 7))
    plt.suptitle(fig_name)
    plt.bar(x_values, y_values, color='#696969A0')
    # Adding titles and labels
    plt.title("Taxa de Perda de Pacotes", fontsize='35')
    plt.xlabel("Testes Realizados", fontsize='15')
    plt.ylabel("Total de Pacotes Recebidos (%)", fontsize='15')

    plt.grid(axis='y', color='k')
    padding = 0.1 * (y_values[-1]-y_values[0])  # Adjust the padding as needed

    # Adding average as a text annotation
    plt.text(x_values[-1] + 0.09 * (x_values[-1] - x_values[0]), average - 0.02 * (y_values[-1] - y_values[0]),
             f"Média: {round(average, 2)}%", ha='right', fontsize='12')
    plt.text(x_values[-1] - 0.05 * (x_values[-1] - x_values[0]), 80,
             f"Intervalo de envio de mensagens: {fig_name}", ha='right', fontsize='12')

    plt.axhline(average, color='#FF0000FF', linestyle='--')
    ax.set_ylim(0, 10)
    # Set a name or title for the figure


files = ['test_1v1_avg_pack_loss_5ms.csv', 'test_1v1_avg_pack_loss_50us.csv',
         'test_1v1_avg_pack_loss_500ms.csv', 'test_1v1_avg_pack_loss_5us.csv']
fig_names = ['5ms', '50us', '500ms', '5us']

for file, fig_name in zip(files, fig_names):
    print_avg_test(file, fig_name)

# Display all the figures
plt.show()
