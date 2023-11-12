from NoiseGenerator import NoiseGenerator
from SignalGenerator import SignalGenerator

SIGMA = 1
SAMPLE_COUNT = 10
NOISE_FILE_PATH = "../data/noise.bin"

if __name__ == '__main__':
    noise = NoiseGenerator(2048, 1, NOISE_FILE_PATH)
    signal = SignalGenerator(2048, NOISE_FILE_PATH)

    noise.append_noise_to_bin(10)
    print("")
    print(signal.read_noise_from_bin(0, 10))
    noise.clear_noise_bin()
