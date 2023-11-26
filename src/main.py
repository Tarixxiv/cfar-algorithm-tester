import json

from NoiseGenerator import NoiseGenerator
from SignalProperties import SignalProperties
from SignalGenerator import SignalGenerator
from BinFileManager import BinFileManager
from time import time
from Output import FinalOutput

from src.CFAR import CFAR_GOCA

if __name__ == '__main__':
    SAMPLE_COUNT = 0,
    SIGMA = 0
    DB = 0
    LINE_COUNT = 0
    NOISE_FILE_PATH = ""
    SIGNAL_INDEX_PATH = ""
    SIGNAL_FILE_PATH = ""

    with open("../data/consts.json", 'r') as file:
        dict = json.load(file)
        locals().update(dict)


    noise = NoiseGenerator(SAMPLE_COUNT, SIGMA, NOISE_FILE_PATH)
    signal = SignalGenerator(DB, SIGMA, SIGNAL_INDEX_PATH)
    noiseFileManager = BinFileManager(NOISE_FILE_PATH)
    signalFileManager = BinFileManager(SIGNAL_FILE_PATH)
    signalProperties = SignalProperties(SIGNAL_INDEX_PATH)
    cfar = CFAR_GOCA()

    start_time = time()
    end_tme = start_time + 60 * 60 * 2
    line_cursor = 0
    output_to_file = FinalOutput()
    for j in range(1000):
        noise_list = []
        for i in range(LINE_COUNT):
            line = noise.generate_noise_line()
            noise_list.append(line)
        noiseFileManager.append_to_file(noise_list)
        # noise_list = noiseFileManager.read_file(line_cursor, line_cursor + LINE_COUNT)
        noise_and_signal, index_line_list = signal.append_signal_to_noise(noise_list)
        signalFileManager.append_to_file(noise_and_signal)
        signal.write_indexes_to_file(index_line_list)
        print(j)
        print(time()-start_time)


    # while time() < end_tme:
    #     noise_list = []
    #     for i in range(LINE_COUNT):
    #         line = noise.generate_noise_line()
    #         noise_list.append(line)
    #     noiseFileManager.append_to_file(noise_list)
    #     # noise_list = noiseFileManager.read_file(line_cursor, line_cursor + LINE_COUNT)
    #     noise_and_signal, index_line_list = signal.append_signal_to_noise(noise_list)
    #     signalFileManager.append_to_file(noise_and_signal)
    #     signal.write_indexes_to_file(index_line_list)
    #
    #     for signal_line, index_line in zip(noise_and_signal, index_line_list):
    #         signalProperties.index = [index_line]
    #         output_to_file.input_signal_properties = signalProperties
    #         output_to_file.output_from_CFAR, trash = cfar.find_objects(signal_line)
    #         output_to_file.analyze_data()
    #         output_to_file.export_to_csv("../output/two_hour_simulation_results")
    #         output_to_file.reset()
    #
    #     line_cursor += LINE_COUNT
    #     print(line_cursor)