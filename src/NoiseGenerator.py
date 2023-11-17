import numpy as np

FLOAT_SIZE = 8
NOISE_FILE_PATH = "../data/noise.bin"


class NoiseGenerator:

    def __init__(self, sample_count, sigma, file_path):
        self.sample_count = sample_count
        self.sigma = sigma
        self.file_path = file_path

    def append_noise_to_bin(self, line_count):
        with open(self.file_path, "a") as file:
            for _ in range(line_count):
                line = np.random.normal(size=self.sample_count, scale=self.sigma)
                print(line)
                line.tofile(file)

    def clear_noise_bin(self):
        with open(self.file_path, "w"):
            pass
