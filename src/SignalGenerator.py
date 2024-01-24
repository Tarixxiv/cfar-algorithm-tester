import random
from cmath import sqrt

class SignalGenerator:
    def __init__(self, SNR, sigma, index_path):
        self.SNR = SNR
        self.sigma = sigma
        self.index_path = index_path

    def calculate_amplitude(self):
        # SNR = 10*log(A^2/sigma^2)
        return sqrt(pow(10, self.SNR / 10)*pow(self.sigma, 2)).real

    def append_signal_to_noise(self, noise):
        indexes = []
        for i in range(len(noise)):
            index = random.randint(0, len(noise[0]) - 1)
            indexes.append(index)
            noise[i][index] += self.calculate_amplitude()
        return noise, indexes
