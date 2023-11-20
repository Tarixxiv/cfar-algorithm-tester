import random

class SignalGenerator:
    def __init__(self, db, sigma):
        self.db = db
        self.sigma = sigma

    def calculate_amplitude(self):
        # 6 [dB] = 10*log(A^2/sigma^2)
        # A = sqrt(10^0.6)
        return 2

    def append_signal_to_noise(self, noise):
        indexes = []
        for i in range(len(noise)):
            index = random.randint(0, len(noise))
            indexes.append(index)
            noise[i][index] += self.calculate_amplitude()
        return noise, indexes
