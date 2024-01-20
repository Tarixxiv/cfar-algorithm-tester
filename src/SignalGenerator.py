import random
from cmath import sqrt


class SignalGenerator:
    def __init__(self, db, sigma, index_path):
        self.db = db
        self.sigma = sigma
        self.index_path = index_path

    def calculate_amplitude(self):
        # dB = 10*log(A^2/sigma^2)
        return sqrt(pow(10, self.db / 10)*pow(self.sigma, 2)).real

    def append_signal_to_noise(self, noise):
        indexes = []
        for i in range(len(noise)):
            index = random.randint(0, len(noise[0]) - 1)
            indexes.append(index)
            noise[i][index] += self.calculate_amplitude()
        return noise, indexes

    def write_indexes_to_file(self, index_line_list):
        indexes_as_strings = [str(x) + ',' for x in index_line_list]
        with open(self.index_path, "a") as file:
            for i in indexes_as_strings:
                file.writelines(i)
                file.writelines('\n')
        with open(self.index_path, 'r') as file:
            l = file.readlines()
        lists = []
        for string in l:
            lists.append(string.split(","))
