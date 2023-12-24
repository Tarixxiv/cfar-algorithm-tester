import json
import numpy as np


class CFAR_CA:
    def __init__(self, number_of_guard_cells=None, number_of_training_cells=None, threshold_factor_min=None,
                 threshold_factor_max=None, threshold_factor_delta=None):
        filepath = "../data/CFAR_parameters.json"
        with open(filepath, "r") as read_file:
            default_settings = json.load(read_file)
        if number_of_guard_cells is None:
            self.number_of_guard_cells = default_settings["CFAR"]["guard_cells"]
        else:
            self.number_of_guard_cells = number_of_guard_cells
        if number_of_training_cells is None:
            self.number_of_training_cells = default_settings["CFAR"]["training_cells"]
        else:
            self.number_of_training_cells = number_of_training_cells
        if threshold_factor_min is None:
            self.threshold_factor_min = default_settings["CFAR"]["threshold_factor_min"]
        else:
            self.threshold_factor_min = threshold_factor_min
        if threshold_factor_max is None:
            self.threshold_factor_max = default_settings["CFAR"]["threshold_factor_max"]
        else:
            self.threshold_factor_max = threshold_factor_max
        if threshold_factor_max is None:
            self.threshold_factor_delta = default_settings["CFAR"]["threshold_factor_delta"]
        else:
            self.threshold_factor_delta = threshold_factor_delta

    def _choose_criteria(self, average_left, average_right):
        return (average_left + average_right) / 2

    def find_objects(self, data, object_indexes):
        data = [abs(element) for element in data]
        last_right_training_cell_number = int(min(self.number_of_guard_cells / 2
                                                  + self.number_of_training_cells, len(data) - 1))
        last_right_guard_cell_number = int(self.number_of_guard_cells / 2)
        first_left_training_cell_number = int(-self.number_of_guard_cells / 2 - self.number_of_training_cells / 2)
        first_left_guard_cell_number = int(-self.number_of_guard_cells / 2)
        sum_left = 0
        sum_right = sum(data[last_right_guard_cell_number: last_right_training_cell_number])
        average_right = 0
        average_left = 0
        # threshold_value = [] # Can be used if you want to show threshold values on a plot
        detected = [[] for _ in range(int(round((self.threshold_factor_max - self.threshold_factor_min)
                                      / self.threshold_factor_delta) + 1))]
        detects_count = [0] * (int(round((self.threshold_factor_max - self.threshold_factor_min) /
                               self.threshold_factor_delta) + 1))
        false_detects_count = [0] * (int(round((self.threshold_factor_max - self.threshold_factor_min) /
                                     self.threshold_factor_delta) + 1))

        for cell_under_test_number in range(len(data)):
            if first_left_training_cell_number - 1 >= 0:
                sum_left -= data[first_left_training_cell_number - 1]
            if first_left_guard_cell_number > 0:
                sum_left += data[first_left_guard_cell_number - 1]
            if last_right_training_cell_number < len(data):
                sum_right += data[last_right_training_cell_number]
            if last_right_guard_cell_number < len(data):
                sum_right -= data[last_right_guard_cell_number]

            count_left = max(0, first_left_guard_cell_number) - max(0, first_left_training_cell_number)
            count_right = min(last_right_training_cell_number, len(data)) - min(last_right_guard_cell_number, len(data))

            if count_left > 0:
                average_left = sum_left / count_left
            if count_right > 0:
                average_right = sum_right / count_right
            for threshold_factor in np.arange(self.threshold_factor_min,
                                          self.threshold_factor_max + self.threshold_factor_delta,
                                          self.threshold_factor_delta):
                if count_left <= 0:
                    threshold = average_right * threshold_factor
                elif count_right <= 0:
                    threshold = average_left * threshold_factor
                else:
                    threshold = self._choose_criteria(average_left, average_right) * threshold_factor

                # threshold_value.append(threshold)
                if threshold < data[cell_under_test_number]:
                    detected[int((threshold_factor - self.threshold_factor_min) /
                                 self.threshold_factor_delta)].append(cell_under_test_number)
                    if cell_under_test_number in object_indexes:
                        detects_count[int((threshold_factor - self.threshold_factor_min) /
                                          self.threshold_factor_delta)] += 1
                    else:
                        false_detects_count[int((threshold_factor - self.threshold_factor_min) /
                                                self.threshold_factor_delta)] += 1

            last_right_training_cell_number += 1
            last_right_guard_cell_number += 1
            first_left_training_cell_number += 1
            first_left_guard_cell_number += 1
            if first_left_training_cell_number <= 0 < first_left_guard_cell_number:
                last_right_training_cell_number = min(last_right_training_cell_number - 1, len(data) - 1)
                sum_right -= data[last_right_training_cell_number]
            if (last_right_training_cell_number > len(data) >= last_right_guard_cell_number and
                    first_left_training_cell_number - 1 >= 0):
                first_left_training_cell_number -= 1
                sum_left += data[first_left_training_cell_number - 1]

        return detected, detects_count, false_detects_count


class CFAR_GOCA(CFAR_CA):
    def _choose_criteria(self, average_left, average_right):
        return max(average_left, average_right)


class CFAR_SOCA(CFAR_CA):
    def _choose_criteria(self, average_left, average_right):
        return min(average_left, average_right)
