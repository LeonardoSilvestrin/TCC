import csv
import matplotlib.pyplot as plt
import re

class teste:
    def __init__(self, delay):
        self.delay = delay
        self.data_per_num = dict()
    def append_data(self,data,type):
        if data[0]+type in self.data_per_num.keys():
            self.data_per_num[data[0] + type].extend(data)
        else:
            self.data_per_num[data[0] + type] = data;

def generate_avg_loss(testes):
    x_values, y_values = list(), list()
    for teste_iter in testes:
        media_do_teste = []
        x_values.append(teste_iter.delay)
        for i in range(10):
            key_r = f'{i}r'
            key_t = f'{i}t'
            if (key_r in teste_iter.data_per_num.keys() and key_t in teste_iter.data_per_num.keys()):
                perdas = (len(teste_iter.data_per_num[f'{i}r'])+1)/(len(teste_iter.data_per_num[f'{i}t']))
                if perdas > 1:
                    perdas = 1/perdas
                media_do_teste.append(perdas)
            else:
                media_do_teste.append(1)
            print(teste_iter.delay,media_do_teste)
        y_values.append(1-sum(media_do_teste)/len(media_do_teste))
    print(x_values,'\n',y_values)
    return(x_values, y_values)

def get_test_data_from_csv(file):
    testes = []
    with open(file, 'r') as csv_file:
        csv_reader = csv.reader(csv_file)
        for line in csv_reader:
            if 'delay' in line[0]:
                microseconds = int(re.findall(r'\d+',line[0])[0])
                teste_atual = teste(microseconds)
                testes.append(teste_atual)
            if line[1] == '/dev/ttyUSB1' and 'delay' not in line[0] and 'Teste' not in line[0]:
                dados = line[0].split(',')[:-1]
                if dados != []:
                    tipo = 't'
                    teste_atual.append_data(dados,tipo)
            
            if line[1] == '/dev/ttyUSB0':
                dados = line[0].split(',')[:-1]
                if dados != []:
                    tipo = 'r'
                    teste_atual.append_data(dados,tipo)
        return testes
def print_results(x_values,y_values):
    
    for i in range(len(y_values)):
        y_values[i]*=100
    average = sum(y_values) / len(y_values)
    # Plotting the bar graph
    fig, ax = plt.subplots(figsize=(15, 7))
    plt.suptitle(fig_name)
    plt.plot(x_values, y_values,marker = 'o', color='#696969A0')
    # Adding titles and labels
    plt.title("Taxa de Perda de Pacotes", fontsize='35')
    plt.xlabel("Delay Entre Mensagens (µS)", fontsize='15')
    plt.ylabel("Total de Pacotes Perdidos (%)", fontsize='15')

    plt.text(x_values[0]+1, average + 0.2 * (y_values[-1] - y_values[0]),
                f"Média: {round(average, 2)}%", ha='right', fontsize='12')
    plt.grid(axis='y', color='k')
    # Adding average as a text annotation
    
    for x, y in zip(x_values, y_values):
        if y != 0:
            plt.text(x, y+0.1, f"{round(y,2)}%", ha='center', va='bottom', fontsize='10')
        else:
            plt.text(x, y, f"{round(y,2)}%", ha='center', va='bottom', fontsize='10')
        

    plt.axhline(average, color='#FF0000FF', linestyle='--')
    ax.set_ylim(0, 1)
    # Set a name or title for the figure



file = r'/home/leo/Documents/TCC/robustness_tests/data_rate_test/test_1v1_avg_pack_loss.csv'
fig_name = 'Average Package Loss'

testes_feitos = get_test_data_from_csv(file)
x,y = generate_avg_loss(testes_feitos)
print_results(x,y)
plt.show()
# Display all the figures
#plt.show()
