import numpy as np


class NoiseGenerator:

    def __init__(self, sample_count, sigma):
        self.sample_count = sample_count
        self.sigma = sigma

    def generate_noise_line(self):
        return np.random.normal(size=self.sample_count, scale=self.sigma)
