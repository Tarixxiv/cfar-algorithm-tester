import csv
import json
import os
import shutil
import numpy as np


class Probabilities:
    def __init__(self):
        self.probability_of_detection = 0
        self.probability_of_false_detection = 0
        self.total_cells_number = 0
        self.total_objects_number = 0
        self.header = ["number_of_cells", "number_of_objects", "detection_probability", "false_detection_probability"]

    def calculate_probabilities(self, detects_count, false_detects_count, data_len, number_of_objects=1):
        self.probability_of_detection = self.probability_of_detection * self.total_objects_number + detects_count
        self.probability_of_false_detection = (self.probability_of_false_detection * self.total_cells_number
                                               + false_detects_count)
        self.total_cells_number += data_len
        self.total_objects_number += number_of_objects
        self.probability_of_detection /= self.total_objects_number
        self.probability_of_false_detection /= self.total_cells_number


class ProbabilitiesForMultipleThresholdFactors:
    def __init__(self, threshold_factor_min=None, threshold_factor_max=None, threshold_factor_delta=None):
        filepath = "../data/CFAR_parameters.json"
        with open(filepath, "r") as read_file:
            default_settings = json.load(read_file)

        if threshold_factor_min is None:
            threshold_factor_min = default_settings["CFAR"]["threshold_factor_min"]
        if threshold_factor_max is None:
            threshold_factor_max = default_settings["CFAR"]["threshold_factor_max"]
        self._probabilities = []
        if threshold_factor_delta is None:
            threshold_factor_delta = default_settings["CFAR"]["threshold_factor_delta"]
        self.threshold_factor_list = np.arange(threshold_factor_min, threshold_factor_max +
                                               threshold_factor_delta, threshold_factor_delta)
        for i in self.threshold_factor_list:
            self._probabilities.append(Probabilities())
        self.header = ["threshold", "detection probability", "false detection probability"]

    def export_to_csv(self, filepath):
        if filepath is None:
            filepath = "../output/output_data_default_name.csv"
        file_exists = os.path.exists(filepath)

        if file_exists:
            copy_filepath = os.path.dirname(filepath) + os.path.splitext(os.path.basename(filepath))[0] + "copy.csv"
            shutil.copy2(filepath, copy_filepath)
        with open(filepath, 'w', encoding='UTF8') as csv_file:
            writer = csv.writer(csv_file, lineterminator='\n')
            writer.writerow(self.header)
            index = 0
            for probability in self._probabilities:
                row = [self.threshold_factor_list[index], probability.probability_of_detection,
                       probability.probability_of_false_detection]
                writer.writerow(row)
                index += 1

    def calculate_probabilities(self, detects_count, false_detects_count, data_len, number_of_objects=1):
        for index in range(len(self.threshold_factor_list)):
            self._probabilities[index].calculate_probabilities(detects_count[index], false_detects_count[index],
                                                               data_len, number_of_objects)
