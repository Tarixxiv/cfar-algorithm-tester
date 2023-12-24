import concurrent
import json
import multiprocessing
from concurrent.futures import as_completed

from NoiseGenerator import NoiseGenerator
from SignalGenerator import SignalGenerator
from time import time
from ProbabilitiesForMultipleThresholdFactors import ProbabilitiesForMultipleThresholdFactors

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

    execution_time_limit = 10
    print(time() - start_time, "generation done")

    loop_start_time = time()
    results = []
    detects_count = []
    false_detects_count = []
    with concurrent.futures.ProcessPoolExecutor(max_workers=multiprocessing.cpu_count()) as executor:
        cfar = CFAR_GOCA()
        futures = []
        for signal_line, index_line in zip(noise_and_signal, index_line_list):
            futures.append(executor.submit(cfar.find_objects,
                                           *(signal_line, [index_line])
                                           ))
        for future in as_completed(futures):
            sample_detect_indexes, sample_detects_count, sample_false_detects_count = future.result()
            if not detects_count:
                detects_count = sample_detects_count
            else:
                detects_count = [sum(x) for x in zip(detects_count, sample_detects_count)]
            if not false_detects_count:
                false_detects_count = sample_false_detects_count
            else:
                false_detects_count = [sum(x) for x in zip(false_detects_count, sample_false_detects_count)]

            if loop_start_time + execution_time_limit >= time():
                print("ended due to time", time() - loop_start_time)
                executor.shutdown()
                print("child processes killed", time() - loop_start_time)
                break

    output_to_file = ProbabilitiesForMultipleThresholdFactors()
    output_to_file.calculate_probabilities(detects_count, false_detects_count, len(signal_line) * LINE_COUNT)
    output_to_file.export_to_csv("../output/output.csv")
    print(time() - start_time)
