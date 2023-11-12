import numpy as np

FLOAT_SIZE = 8


class SignalGenerator:
    def __init__(self, sample_count, file_path):
        self.sample_count = sample_count
        self.file_path = file_path

    def read_noise_from_bin(self, start_line, finish_line):
        output = []
        with open(self.file_path, "r") as file:
            output.append(np.fromfile(file,
                                      count=self.sample_count,
                                      offset=FLOAT_SIZE * start_line * self.sample_count))
            for i in range(finish_line - start_line - 1):
                output.append(np.fromfile(file, count=self.sample_count))
        return output
