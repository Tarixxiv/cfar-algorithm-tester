import json

from NoiseGenerator import NoiseGenerator
from SignalGenerator import SignalGenerator
from BinFileManager import BinFileManager


SAMPLE_COUNT = 2048
LINE_COUNT = 10
DB = 6
SIGMA = 1
NOISE_FILE_PATH = "data/noise.bin"
SIGNAL_FILE_PATH = "data/signal.bin"
SIGNAL_INDEX_PATH = "data/signal_objects.json"

if __name__ == '__main__':
    noise = NoiseGenerator(SAMPLE_COUNT, SIGMA, NOISE_FILE_PATH)
    signal = SignalGenerator(DB, SIGMA)
    noiseFileManager = BinFileManager(NOISE_FILE_PATH)
    signalFileManager = BinFileManager(SIGNAL_FILE_PATH)

    lines = []
    for i in range(LINE_COUNT):
        line = noise.generate_noise_line()
        print(line)
        lines.append(line)
    noiseFileManager.append_to_file(lines)
    print("")

    noise_list = noiseFileManager.read_file(0, LINE_COUNT)


    noise_and_signal, index_line_list = signal.append_signal_to_noise(noise_list)
    with open(SIGNAL_INDEX_PATH, "a") as file:
        for i in index_line_list:
            json.dump(i, file)

    signalFileManager.append_to_file(noise_and_signal)

    noiseFileManager.clear_file()
    signalFileManager.clear_file()
