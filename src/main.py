import concurrent
import json
from concurrent.futures import as_completed
from time import time

from CFAR import CFAR
from NoiseGenerator import NoiseGenerator
from ProbabilitiesForMultipleThresholdFactors import ProbabilitiesForMultipleThresholdFactors
from SignalGenerator import SignalGenerator

SAMPLE_COUNT = 0,
SIGMA = 0
SNR = 0
EXECUTION_TIME_LIMIT = 0
WORKER_COUNT = 0
LINE_COUNT = 0
SIGNAL_INDEX_PATH = ""
OUTPUT_FILE_PATH = ""

def add_element_wise(list_a,list_b):
    if not list_a:
        return list_b
    elif not list_b:
        return list_a
    else:
        return [sum(x) for x in zip(list_a, list_b)]

def generate_noise_and_signal_sample(noise_generator, signal_generator):
    noise_list = []
    line = noise_generator.generate_noise_line()
    noise_list.append(line)
    return signal_generator.append_signal_to_noise(noise_list)

def worker_task(time_limit, sample_count, sigma, SNR):
    noise = NoiseGenerator(sample_count, sigma)
    signal = SignalGenerator(SNR, sigma, SIGNAL_INDEX_PATH)
    cfar = CFAR()

    total_detects_count_CA = []
    total_false_detects_count_CA = []
    total_detects_count_GOCA = []
    total_false_detects_count_GOCA = []
    total_detects_count_SOCA = []
    total_false_detects_count_SOCA = []

    start_time = time()
    execution_count = 0
    while(time() < start_time + time_limit):
        noise_and_signal, index_line_list = generate_noise_and_signal_sample(noise, signal)
        (detects_count_CA, false_detects_count_CA,
         detects_count_GOCA, false_detects_count_GOCA,
        detects_count_SOCA, false_detects_count_SOCA) = (cfar.find_objects_for_multiple_threshold_offsets(noise_and_signal[0],
                                                                           [index_line_list[0]]))
        total_detects_count_CA = add_element_wise(total_detects_count_CA, detects_count_CA)
        total_false_detects_count_CA = add_element_wise(total_false_detects_count_CA, false_detects_count_CA)
        total_detects_count_GOCA = add_element_wise(total_detects_count_GOCA, detects_count_GOCA)
        total_false_detects_count_GOCA = add_element_wise(total_false_detects_count_GOCA, false_detects_count_GOCA)
        total_detects_count_SOCA = add_element_wise(total_detects_count_SOCA,detects_count_SOCA)
        total_false_detects_count_SOCA = add_element_wise(total_false_detects_count_SOCA,false_detects_count_SOCA)
        execution_count += 1

    print("process finished in:", time() - start_time, "s")
    return (total_detects_count_CA, total_false_detects_count_CA, total_detects_count_GOCA,
            total_false_detects_count_GOCA, total_detects_count_SOCA, total_false_detects_count_SOCA, execution_count)


if __name__ == '__main__':
    with open("../data/consts.json", 'r') as file:
        dict = json.load(file)
        locals().update(dict)

    with open(OUTPUT_FILE_PATH, "w"):
        pass
    with open(SIGNAL_INDEX_PATH, 'w'):
        pass

    total_detects_count_CA = []
    total_false_detects_count_CA = []
    total_detects_count_GOCA = []
    total_false_detects_count_GOCA = []
    total_detects_count_SOCA = []
    total_false_detects_count_SOCA = []

    futures = []
    total_execution_count = 0

    with concurrent.futures.ProcessPoolExecutor(max_workers=WORKER_COUNT) as executor:
        for i in range(WORKER_COUNT):
            futures.append(executor.submit(worker_task, *(EXECUTION_TIME_LIMIT,SAMPLE_COUNT, SIGMA, SNR)))
        for future in as_completed(futures):
            (detects_count_CA, false_detects_count_CA,
             detects_count_GOCA, false_detects_count_GOCA,
             detects_count_SOCA, false_detects_count_SOCA, execution_count) = future.result()

            total_detects_count_CA = add_element_wise(total_detects_count_CA, detects_count_CA)
            total_false_detects_count_CA = add_element_wise(total_false_detects_count_CA, false_detects_count_CA)
            total_detects_count_GOCA = add_element_wise(total_detects_count_GOCA, detects_count_GOCA)
            total_false_detects_count_GOCA = add_element_wise(total_false_detects_count_GOCA, false_detects_count_GOCA)
            total_detects_count_SOCA = add_element_wise(total_detects_count_SOCA, detects_count_SOCA)
            total_false_detects_count_SOCA = add_element_wise(total_false_detects_count_SOCA, false_detects_count_SOCA)
            total_execution_count += execution_count

    algorithm_names = ["CA", "GOCA", "SOCA"]
    total_detect_list = [total_detects_count_CA, total_detects_count_GOCA, total_detects_count_SOCA]
    total_false_detect_list = [total_false_detects_count_CA,
                               total_false_detects_count_GOCA, total_false_detects_count_SOCA]

    for name, detects_count, false_detects_count in zip(algorithm_names, total_detect_list, total_false_detect_list):
        output_to_file = ProbabilitiesForMultipleThresholdFactors()
        data_len = SAMPLE_COUNT * total_execution_count
        output_to_file.calculate_probabilities(detects_count, false_detects_count, data_len, total_execution_count)
        output_to_file.export_to_csv("../output/output" + name + ".csv")

    print(total_execution_count, "simulations made")