import json

from NoiseGenerator import NoiseGenerator
from SignalProperties import SignalProperties
from SignalGenerator import SignalGenerator
from time import time
from Output import ProbabilitiesForMultipleThresholdFactors

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
    output_to_file = ProbabilitiesForMultipleThresholdFactors()
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

    parameter_value_range = [x * 0.1 for x in range(10, 101)]
    execution_time_limit = 2 * 3600
    time_per_parameter = execution_time_limit / len(parameter_value_range)

    print(time() - start_time, "generation done")

    loop_start_time = time()
    for signal_line, index_line in zip(noise_and_signal, index_line_list):
        trash, detects_count, false_detects_count = cfar.find_objects(signal_line, [index_line])
        output_to_file.calculate_probabilities(detects_count, false_detects_count, len(signal_line))
        if loop_start_time + time_per_parameter <= time():
            print("ended due to time")
            break

    output_to_file.export_to_csv("OUTPUT_FILE_PATH")
    print(time() - start_time)
