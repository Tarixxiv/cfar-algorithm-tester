import matplotlib.pyplot
import numpy as np
import matplotlib.pyplot as plt
import csv
import os


class CreatePlots:

    def __init__(self):
        self.inputs_list = []

        for i in range(9):
            self.inputs_list.append([])

    def read_csv(self, which_cfar=0, file_path=None):
        if file_path is None:
            # file_path = "../output/output_data_default_name.csv
            file_path = "../output/output.csv"
        file_exists = os.path.exists(file_path)

        if file_exists:
            file = open(file_path, "r")
            data = list(csv.reader(file, delimiter=","))
            file.close()

            # number_of_row = 0
            which_row = which_cfar
            for row in data[1:]:
                # which_row = number_of_row % 3
                for i in range(3):
                    self.inputs_list[which_row*3+i].append(row[i])
                # number_of_row = which_row+1

            for i in range(9):
                # self.inputs_list[i] = [round(eval(j), 3) for j in self.inputs_list[i]]
                self.inputs_list[i] = [eval(j) for j in self.inputs_list[i]]
                if i % 3 != 0:
                    for j in range(len(self.inputs_list[i])):
                        self.inputs_list[i][j] = self.inputs_list[i][j] * 100

    def draw_plots(self):
        fig, axs = plt.subplots(3)
        algorithms_names = ['CFAR CA', 'CFAR GOCA', 'CFAR SOCA']

        for i in range(3):
            print(self.inputs_list[3 * i])
            print(self.inputs_list[3 * i + 1])
            print(self.inputs_list[3 * i + 2])

            axs[i].plot(self.inputs_list[3 * i], self.inputs_list[3 * i + 1], color='green',
                        label='probability of detection')
            axs[i].plot(self.inputs_list[3 * i], self.inputs_list[3 * i + 2], color='red',
                        label='probability of false detection')
            axs[i].set_title(algorithms_names[i], fontsize=15)
            axs[i].legend(loc="upper right", title='Probabilities in %')
            axs[i].grid(True)
            axs[i].set_ylim(0, 100)
        fig.tight_layout()


cp = CreatePlots()
cp.read_csv()
cp.draw_plots()
matplotlib.pyplot.show()
