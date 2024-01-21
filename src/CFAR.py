import json
import numpy as np
import enum


class CFAR:
    class CFAR_Types(enum.IntEnum):
        CA = 0
        GOCA = 1
        SOCA = 2

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
            threshold_factor_min = default_settings["CFAR"]["threshold_factor_min"]
        if threshold_factor_max is None:
            threshold_factor_max = default_settings["CFAR"]["threshold_factor_max"]
        if threshold_factor_delta is None:
            threshold_factor_delta = default_settings["CFAR"]["threshold_factor_delta"]
        self.threshold_factor_list = np.arange(threshold_factor_min, threshold_factor_max +
                                               threshold_factor_delta, threshold_factor_delta)

    def _calculate_threshold(self, average_left, average_right, cfar_type):
        if average_left is None:
            return average_right
        elif average_right is None:
            return average_left
        else:
            if cfar_type == self.CFAR_Types.CA:
                return (average_left + average_right) / 2
            elif cfar_type == self.CFAR_Types.GOCA:
                return max(average_left, average_right)
            elif cfar_type == self.CFAR_Types.SOCA:
                return min(average_left, average_right)

    def calculate_all_thresholds_single(self, signal, threshold_factor):
        thresholds = []
        mean_left, mean_right = self.calculate_means(signal)

        for algorithm_type in self.CFAR_Types:
            thresholds.append([])
            for cell_under_test_number in range(len(mean_left)):
                thresholds[algorithm_type].append(self._calculate_threshold(
                    mean_left[cell_under_test_number], mean_right[cell_under_test_number], algorithm_type)
                                                  * threshold_factor)

        return thresholds

    def find_objects(self, signal, object_indexes):
        mean_left, mean_right = self.calculate_means(signal)
        detects_count = []
        false_detects_count = []
        for algorithm_type in self.CFAR_Types:
            detects_count.append([0] * len(self.threshold_factor_list))
            false_detects_count.append([0] * len(self.threshold_factor_list))

            for cell_under_test_number in range(len(signal)):
                index = 0
                for threshold_factor in self.threshold_factor_list:
                    threshold = (threshold_factor * self._calculate_threshold(
                        mean_left[cell_under_test_number], mean_right[cell_under_test_number], algorithm_type)
                                 * threshold_factor)

                    if threshold < signal[cell_under_test_number]:
                        if cell_under_test_number in object_indexes:
                            detects_count[algorithm_type][index] += 1
                        else:
                            false_detects_count[algorithm_type][index] += 1
                    index += 1

        return detects_count, false_detects_count

    def calculate_means(self, signal):
        signal = [abs(element) for element in signal]
        last_right_training_cell_number = int(min(self.number_of_guard_cells / 2
                                                  + self.number_of_training_cells, len(signal) - 1))
        last_right_guard_cell_number = int(self.number_of_guard_cells / 2)
        first_left_training_cell_number = int(-self.number_of_guard_cells / 2 - self.number_of_training_cells / 2)
        first_left_guard_cell_number = int(-self.number_of_guard_cells / 2)
        sum_left = 0
        sum_right = sum(signal[last_right_guard_cell_number: last_right_training_cell_number])
        mean_left = []
        mean_right = []
        for cell_under_test_number in range(len(signal)):
            if first_left_training_cell_number - 1 >= 0:
                sum_left -= signal[first_left_training_cell_number - 1]
            if first_left_guard_cell_number > 0:
                sum_left += signal[first_left_guard_cell_number - 1]
            if last_right_training_cell_number < len(signal):
                sum_right += signal[last_right_training_cell_number]
            if last_right_guard_cell_number < len(signal):
                sum_right -= signal[last_right_guard_cell_number]

            count_left = max(0, first_left_guard_cell_number) - max(0, first_left_training_cell_number)
            count_right = min(last_right_training_cell_number + 1, len(signal)) - min(last_right_guard_cell_number + 1,
                                                                                      len(signal))

            if count_left > 0:
                mean_left.append(sum_left / count_left)
            else:
                mean_left.append(None)
            if count_right > 0:
                mean_right.append(sum_right / count_right)
            else:
                mean_right.append(None)

            last_right_training_cell_number += 1
            last_right_guard_cell_number += 1
            first_left_training_cell_number += 1
            first_left_guard_cell_number += 1
            if first_left_training_cell_number <= 0 < first_left_guard_cell_number:
                last_right_training_cell_number = min(last_right_training_cell_number - 1, len(signal) - 1)
                sum_right -= signal[last_right_training_cell_number]
            if (last_right_training_cell_number >= len(signal) > last_right_guard_cell_number and
                    first_left_training_cell_number - 1 >= 0):
                first_left_training_cell_number -= 1
                sum_left += signal[first_left_training_cell_number - 1]

        return mean_left, mean_right
