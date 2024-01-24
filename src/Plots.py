import matplotlib.pyplot
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
            file_path = "../output/output.csv"
        file_exists = os.path.exists(file_path)

        if file_exists:
            file = open(file_path, "r")
            data = list(csv.reader(file, delimiter=","))
            file.close()

            which_row = which_cfar
            for row in data[1:]:
                for i in range(3):
                    self.inputs_list[which_row*3+i].append(row[i])

            for i in range(3):
                self.inputs_list[i+3*which_row] = [eval(j) for j in self.inputs_list[i+3*which_row]]

    def draw_plots(self):
        fig, axs = plt.subplots(3, 2)
        algorithms_names = ['CFAR CA', 'CFAR GOCA', 'CFAR SOCA']

        for i in range(3):

            axs[i, 0].semilogy(self.inputs_list[3 * i], self.inputs_list[3 * i + 1], color='green',
                        label='probability of detection', nonpositive='clip')
            axs[i, 0].semilogy(self.inputs_list[3 * i], self.inputs_list[3 * i + 2], color='red',
                        label='probability of false detection', nonpositive='clip')
            axs[i, 0].set_title(algorithms_names[i], fontsize=15)
            axs[i, 0].legend(loc="lower left", title='Probabilities in %')
            axs[i, 0].grid(True)
            axs[i, 0].set_ylim(0.00000001, 1)
            axs[i, 0].set_xlabel('threshold value')

            axs[i, 1].loglog(self.inputs_list[3 * i + 1], self.inputs_list[3 * i + 2], color='black')
            axs[i, 1].set_title(algorithms_names[i], fontsize=15)
            axs[i, 1].grid(True)
            axs[i, 1].set_xlabel('Probability of false detection')
            axs[i, 1].set_ylabel('Probability of detection')

        fig.tight_layout()


cp = CreatePlots()
cp.read_csv(0, "../output/outputCA.csv")
cp.read_csv(1, "../output/outputGOCA.csv")
cp.read_csv(2, "../output/outputSOCA.csv")
cp.draw_plots()
matplotlib.pyplot.show()
