import concurrent
import json
import multiprocessing
from concurrent.futures import as_completed

from NoiseGenerator import NoiseGenerator
from SignalGenerator import SignalGenerator
from time import time
from ProbabilitiesForMultipleThresholdFactors import ProbabilitiesForMultipleThresholdFactors
from CFAR import CFAR

SAMPLE_COUNT = 0,
SIGMA = 0
DB = 0
LINE_COUNT = 0
NOISE_FILE_PATH = ""
SIGNAL_INDEX_PATH = ""
SIGNAL_FILE_PATH = ""
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

def worker_task(time_limit, sample_count, sigma, db):
    noise = NoiseGenerator(sample_count, sigma, NOISE_FILE_PATH)
    signal = SignalGenerator(db, sigma, SIGNAL_INDEX_PATH)
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
        detects_count_SOCA, false_detects_count_SOCA) = (cfar.find_objects(noise_and_signal[0],
                                                                           [index_line_list[0]]))

        total_detects_count_CA = add_element_wise(total_detects_count_CA, detects_count_CA)
        total_false_detects_count_CA = add_element_wise(total_false_detects_count_CA, false_detects_count_CA)
        total_detects_count_GOCA = add_element_wise(total_detects_count_GOCA, detects_count_GOCA)
        total_false_detects_count_GOCA = add_element_wise(total_false_detects_count_GOCA, false_detects_count_GOCA)
        total_detects_count_SOCA = add_element_wise(total_detects_count_SOCA,detects_count_SOCA)
        total_false_detects_count_SOCA = add_element_wise(total_false_detects_count_SOCA,false_detects_count_SOCA)
        execution_count += 1

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

    execution_time_limit = 15

    total_detects_count_CA = []
    total_false_detects_count_CA = []
    total_detects_count_GOCA = []
    total_false_detects_count_GOCA = []
    total_detects_count_SOCA = []
    total_false_detects_count_SOCA = []

    futures = []
    total_execution_count = 0
    with concurrent.futures.ProcessPoolExecutor(max_workers=multiprocessing.cpu_count()) as executor:
        for i in range(multiprocessing.cpu_count()):
            futures.append(executor.submit(worker_task, *(execution_time_limit,SAMPLE_COUNT, SIGMA, DB)))
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
