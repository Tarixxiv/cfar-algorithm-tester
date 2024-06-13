import matplotlib.pyplot
import matplotlib.pyplot as plt
import csv
import os


class CreatePlots:

    def __init__(self):
        self.inputs_list = []
        self.number_of_algorithms = 4*2
        self.algorithms_names = ['CFAR CA', 'CFAR GOCA', 'CFAR SOCA', 'CFAR OS']

        for i in range(3*self.number_of_algorithms):
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
            for row in data[1:25]:
                for i in range(3):
                    self.inputs_list[which_row*3+i].append(row[i])

            for i in range(3):
                self.inputs_list[i+3*which_row] = [eval(j) for j in self.inputs_list[i+3*which_row]]

    def draw_plots(self):
        fig, axs = plt.subplots(self.number_of_algorithms-4, 2)
        fig.suptitle('Threshold value = 0                                                                                   Threshold value = 5', fontsize=14, fontweight='bold')
        #fig.suptitle('Threshold value = 10                                                                                 Threshold value = 15', fontsize=14, fontweight='bold')
        #fig.suptitle('Threshold value = 20                                                                                 Threshold value = 25', fontsize=14, fontweight='bold')

        plt.tight_layout()

        for i in range(self.number_of_algorithms-4):
            self.draw_pd_distance_plots(axs, i, 0, 0)
            self.draw_pd_distance_plots(axs, i, 1, 1)

            # self.draw_probabilities_threshold_plots(axs, i, 0)
            # self.draw_pd_pfd_plots(axs, i, 1)

    def draw_probabilities_threshold_plots(self, axs, i, which_column):
        axs[i, which_column].semilogy(self.inputs_list[3 * i], self.inputs_list[3 * i + 1], color='green',
                           label='probability of detection', nonpositive='clip')
        axs[i, which_column].semilogy(self.inputs_list[3 * i], self.inputs_list[3 * i + 2], color='red',
                           label='probability of false detection', nonpositive='clip')
        axs[i, which_column].set_title(self.algorithms_names[i], fontsize=13)
        axs[i, which_column].legend(loc="lower left", title='Probabilities in %')
        axs[i, which_column].grid(True)
        axs[i, which_column].set_ylim(0.00000001, 1)
        axs[i, which_column].set_xlabel('threshold')

    def draw_pd_distance_plots(self, axs, i, which_column, series):

        axs[i, which_column].plot(self.inputs_list[3 * (i+4*series)], self.inputs_list[3 * (i+4*series) + 1], color='green',
                        label='probability of detection')
        axs[i, which_column].plot(self.inputs_list[3 * i], self.inputs_list[3 * i + 2], color='red',
                        label='probability of false detection')
        axs[i, which_column].set_title(self.algorithms_names[i], fontsize=13)
        axs[i, which_column].legend(loc="lower left", title='Probabilities')
        axs[i, which_column].grid(True)
        axs[i, which_column].set_ylim(0, 1)
        axs[i, which_column].set_xlabel('distance [number of cells]')

    def draw_pd_pfd_plots(self, axs, i, which_column):
        axs[i, which_column].loglog(self.inputs_list[3 * i + 1], self.inputs_list[3 * i + 2], color='black')
        axs[i, which_column].set_title(self.algorithms_names[i], fontsize=15)
        axs[i, which_column].grid(True)
        axs[i, which_column].set_xlabel('Probability of false detection')
        axs[i, which_column].set_ylabel('Probability of detection')


cp = CreatePlots()
cp.read_csv(0, "../output/output0CA.csv")
cp.read_csv(1, "../output/output0GOCA.csv")
cp.read_csv(2, "../output/output0SOCA.csv")
cp.read_csv(3, "../output/output0Median.csv")
cp.read_csv(4, "../output/output5CA.csv")
cp.read_csv(5, "../output/output5GOCA.csv")
cp.read_csv(6, "../output/output5SOCA.csv")
cp.read_csv(7, "../output/output5Median.csv")

# #
# cp.read_csv(0, "../output/output10CA.csv")
# cp.read_csv(1, "../output/output10GOCA.csv")
# cp.read_csv(2, "../output/output10SOCA.csv")
# cp.read_csv(3, "../output/output10Median.csv")
# cp.read_csv(4, "../output/output15CA.csv")
# cp.read_csv(5, "../output/output15GOCA.csv")
# cp.read_csv(6, "../output/output15SOCA.csv")
# cp.read_csv(7, "../output/output15Median.csv")


# cp.read_csv(0, "../output/output20CA.csv")
# cp.read_csv(1, "../output/output20GOCA.csv")
# cp.read_csv(2, "../output/output20SOCA.csv")
# cp.read_csv(3, "../output/output20Median.csv")
# cp.read_csv(4, "../output/output25CA.csv")
# cp.read_csv(5, "../output/output25GOCA.csv")
# cp.read_csv(6, "../output/output25SOCA.csv")
# cp.read_csv(7, "../output/output25Median.csv")
cp.draw_plots()

matplotlib.pyplot.show()
