import json

from NoiseGenerator import NoiseGenerator
from SignalProperties import SignalProperties
from SignalGenerator import SignalGenerator
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
    OUTPUT_FILE_PATH = ""

    with open("../data/consts.json", 'r') as file:
        dict = json.load(file)
        locals().update(dict)

    noise = NoiseGenerator(SAMPLE_COUNT, SIGMA, NOISE_FILE_PATH)
    signal = SignalGenerator(DB, SIGMA, SIGNAL_INDEX_PATH)
    signalProperties = SignalProperties(SIGNAL_INDEX_PATH)
    cfar = CFAR_GOCA()
    output_to_file = FinalOutput()
    start_time = time()

    with open(OUTPUT_FILE_PATH, "w"):
        pass
    with open(SIGNAL_INDEX_PATH, 'w'):
        pass

    noise_list = []
    for i in range(LINE_COUNT):
        line = noise.generate_noise_line()
        noise_list.append(line)

    noise_and_signal, index_line_list = signal.append_signal_to_noise(noise_list)

    time_per_parameter = 80  # 2 hrs / 90 parameter values

    print(time() - start_time, "generation done")
    for threshold_factor in range(10, 101):
        cfar.threshold_factor = threshold_factor * 0.1
        loop_start_time = time()
        for signal_line, index_line in zip(noise_and_signal, index_line_list):
            signalProperties.index = [index_line]
            output_to_file.input_signal_properties = signalProperties
            output_to_file.output_from_CFAR, trash = cfar.find_objects(signal_line)
            output_to_file.analyze_data()
            if loop_start_time + time_per_parameter <= time():
                print("ended due to time")
                break

        output_to_file.export_to_csv(OUTPUT_FILE_PATH)
        output_to_file.reset()
        print(time() - start_time)
