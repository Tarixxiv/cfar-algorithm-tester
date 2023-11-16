from NoiseGenerator import NoiseGenerator
from SignalGenerator import SignalGenerator

SIGMA = 1
SAMPLE_COUNT = 2048
LINE_COUNT = 10
NOISE_FILE_PATH = "../data/noise.bin"
SIGNAL_FILE_PATH = "../data/signal.bin"

if __name__ == '__main__':
    noise = NoiseGenerator(SAMPLE_COUNT, 1, NOISE_FILE_PATH)
    signal = SignalGenerator(SAMPLE_COUNT, NOISE_FILE_PATH, 6, LINE_COUNT,SIGNAL_FILE_PATH)

    noise.append_noise_to_bin(LINE_COUNT)
    print("")
    noise_list = signal.read_noise_from_bin(0, LINE_COUNT)
    noise_and_signal = signal.append_signal_to_noise(noise_list)
    signal.append_signal_to_bin(noise_and_signal)
    noise.clear_noise_bin()
    signal.clear_signal_bin()

