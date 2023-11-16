import numpy as np
import random

FLOAT_SIZE = 8


class SignalGenerator:
    def __init__(self, sample_count, file_path, db,line_count, signal_path):
        self.sample_count = sample_count
        self.noise_path = file_path
        self.db = db
        self.line_count = line_count
        self.signal_path = signal_path

    def read_noise_from_bin(self, start_line, finish_line):
        output = []
        with open(self.noise_path, "r") as file:
            output.append(np.fromfile(file,
                                      count=self.sample_count,
                                      offset=FLOAT_SIZE * start_line * self.sample_count))
            for i in range(finish_line - start_line - 1):
                output.append(np.fromfile(file, count=self.sample_count))
        return output

    def calculate_amplitude(self):
        # 6 [dB] = 10*log(A^2/sigma^2)
        # A = sqrt(10^0.6)
        return 2

    def append_signal_to_noise(self, noise):
        for i in range(self.line_count):
            index = random.randint(0, len(noise))
            noise[i][index] += self.calculate_amplitude()

        return noise

    def append_signal_to_bin(self, signal):
        with open(self.signal_path, "a") as file:
            for line in signal:
                #line = np.random.normal(size=self.sample_count, scale=self.sigma)
                #print(line)
                line.tofile(file)

    def clear_signal_bin(self):
        with open(self.signal_path, "w"):
            pass
