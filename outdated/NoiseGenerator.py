import numpy as np

NOISE_FILE_PATH = "../data/noise.bin"


class NoiseGenerator:

    def __init__(self, sample_count, sigma, file_path):
        self.sample_count = sample_count
        self.sigma = sigma
        self.file_path = file_path

    def generate_noise_line(self):
        return np.random.normal(size=self.sample_count, scale=self.sigma)
