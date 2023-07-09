import csv
import matplotlib.pyplot as plt
import re

class teste:
    def __init__(self, delay):
        self.delay = delay
        self.data = list()
        self.num = list()
        self.type = None
    def append_data(self,data,type):
        if data[0] not in num:
            num.append(data[0])
        else:

        self.data = data
        self.type = type




def print_avg_test(file, fig_name):
    transmitter_data = dict()
    receiver_data = dict()
    x_values = list()
    y_values = list()
    testes = []
    with open(file, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        teste_atual = False
        for line in csv_reader:
            if 'delay' in line[0]:
                microseconds = int(re.findall(r'\d+',line[0])[0])
                teste_atual = teste(microseconds)
                testes.append(teste_atual)
            if(teste_atual):
                    if line[1] == '/dev/ttyUSB1' and 'delay' not in line[0] and 'Teste' not in line[0]:
                        dados = line[0].split(',')[:-1]
                        numero = line[0][0]
                        tipo = 't'
                        print(len(dados),len(numero),len(tipo))
                        teste_atual.append_data(dados,numero,tipo)
                        print(teste_atual.data)
    '''
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
    plt.ylabel("Total de Pacotes Perdidos (%)", fontsize='15')

    plt.grid(axis='y', color='k')
    # Adding average as a text annotation
    plt.text(x_values[-1] + 0.09 * (x_values[-1] - x_values[0]), average - 0.02 * (y_values[-1] - y_values[0]),
             f"Média: {round(average, 2)}%", ha='right', fontsize='12')

    plt.axhline(average, color='#FF0000FF', linestyle='--')
    ax.set_ylim(0, 15)
    # Set a name or title for the figure


    '''
files = ['test_1v1_avg_pack_loss.csv']
fig_names = ['Average Package Loss']

for file, fig_name in zip(files, fig_names):
    print_avg_test(file, fig_name)

# Display all the figures
#plt.show()
