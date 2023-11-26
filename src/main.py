import json

from NoiseGenerator import NoiseGenerator
from SignalProperties import SignalProperties
from SignalGenerator import SignalGenerator
from BinFileManager import BinFileManager
from time import time

if __name__ == '__main__':
    dict = {}
    with open("../data/consts.json", 'r') as file:
        dict = json.load(file)
    locals().update(dict)

    noise = NoiseGenerator(SAMPLE_COUNT, SIGMA, NOISE_FILE_PATH)
    signal = SignalGenerator(DB, SIGMA, SIGNAL_INDEX_PATH)
    noiseFileManager = BinFileManager(NOISE_FILE_PATH)
    signalFileManager = BinFileManager(SIGNAL_FILE_PATH)
    signalProperties = SignalProperties(SIGNAL_INDEX_PATH)



    end_tme = time() + 60 * 60 * 2
    line_cursor = 0
    while time() < end_tme:
        noise_list = []
        for i in range(LINE_COUNT):
            line = noise.generate_noise_line()
            print(line)
            noise_list.append(line)
        noiseFileManager.append_to_file(noise_list)
        noise_list = noiseFileManager.read_file(line_cursor, line_cursor + LINE_COUNT)
        noise_and_signal, index_line_list = signal.append_signal_to_noise(noise_list)
        signal.write_indexes_to_file(index_line_list)





        line_cursor += LINE_COUNT
