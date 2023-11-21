import json

from NoiseGenerator import NoiseGenerator
from SignalProperties import SignalProperties
from SignalGenerator import SignalGenerator
from BinFileManager import BinFileManager

if __name__ == '__main__':
    dict = {}
    with open("../data/consts.json", 'r') as file:
        dict = json.load(file)
    locals().update(dict)

    noise = NoiseGenerator(SAMPLE_COUNT, SIGMA, NOISE_FILE_PATH)
    signal = SignalGenerator(DB, SIGMA, SIGNAL_INDEX_PATH)
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
    strings = [str(x)+',' for x in index_line_list]

    sp = SignalProperties(SIGNAL_INDEX_PATH)
    sp.read_indexes_from_file()
    x = sp.index[0]
    x1 = sp.index[1]
    signalFileManager.append_to_file(noise_and_signal)

    noiseFileManager.clear_file()
    signalFileManager.clear_file()
