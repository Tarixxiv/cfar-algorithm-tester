import json
import numpy as np


class CFAR:

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
        else:
            threshold_factor_min = threshold_factor_min
        if threshold_factor_max is None:
            threshold_factor_max = default_settings["CFAR"]["threshold_factor_max"]
        else:
            threshold_factor_max = threshold_factor_max
        if threshold_factor_delta is None:
            threshold_factor_delta = default_settings["CFAR"]["threshold_factor_delta"]
        else:
            threshold_factor_delta = threshold_factor_delta
        self.threshold_factor_list = np.arange(threshold_factor_min,
                                               threshold_factor_max + threshold_factor_delta,
                                               threshold_factor_delta)
        self._mean_left = []
        self._mean_right = []

    def _choose_criteria_CA(self, average_left, average_right):
        return (average_left + average_right) / 2

    def _choose_criteria_SOCA(self, average_left, average_right):
        return min(average_left, average_right)

    def _choose_criteria_GOCA(self, average_left, average_right):
        return max(average_left, average_right)

    def calculate_thresholds_single(self, signal, threshold_factor):
        threshold_CA = []
        threshold_GOCA = []
        threshold_SOCA = []
        self._calculate_means(signal)

        for cell_under_test_number in range(len(self._mean_left)):
            if self._mean_left[cell_under_test_number] is None:
                threshold_CA.append(self._mean_right[cell_under_test_number] * threshold_factor)
                threshold_SOCA.append(self._mean_right[cell_under_test_number] * threshold_factor)
                threshold_GOCA.append(self._mean_right[cell_under_test_number] * threshold_factor)
            elif self._mean_right[cell_under_test_number] is None:
                threshold_CA.append(self._mean_left[cell_under_test_number] * threshold_factor)
                threshold_SOCA.append(self._mean_left[cell_under_test_number] * threshold_factor)
                threshold_GOCA.append(self._mean_left[cell_under_test_number] * threshold_factor)
            else:
                threshold_CA.append(self._choose_criteria_CA(
                    self._mean_left[cell_under_test_number],
                    self._mean_right[cell_under_test_number]) * threshold_factor)
                threshold_GOCA.append(self._choose_criteria_GOCA(
                    self._mean_left[cell_under_test_number],
                    self._mean_right[cell_under_test_number]) * threshold_factor)
                threshold_SOCA.append(self._choose_criteria_SOCA(
                    self._mean_left[cell_under_test_number],
                    self._mean_right[cell_under_test_number]) * threshold_factor)
        return threshold_CA, threshold_GOCA, threshold_SOCA

    def find_objects(self, signal, object_indexes):
        self._calculate_means(signal)
        detects_count_CA = [0] * len(self.threshold_factor_list)
        false_detects_count_CA = [0] * len(self.threshold_factor_list)
        detects_count_GOCA = [0] * len(self.threshold_factor_list)
        false_detects_count_GOCA = [0] * len(self.threshold_factor_list)
        detects_count_SOCA = [0] * len(self.threshold_factor_list)
        false_detects_count_SOCA = [0] * len(self.threshold_factor_list)

        for cell_under_test_number in range(len(signal)):
            index = 0
            for threshold_factor in self.threshold_factor_list:
                if self._mean_left[cell_under_test_number] is None:
                    threshold_CA = self._mean_right[cell_under_test_number] * threshold_factor
                    threshold_SOCA = self._mean_right[cell_under_test_number] * threshold_factor
                    threshold_GOCA = self._mean_right[cell_under_test_number] * threshold_factor
                elif self._mean_right[cell_under_test_number] is None:
                    threshold_CA = self._mean_left[cell_under_test_number] * threshold_factor
                    threshold_SOCA = self._mean_left[cell_under_test_number] * threshold_factor
                    threshold_GOCA = self._mean_left[cell_under_test_number] * threshold_factor
                else:
                    threshold_CA = self._choose_criteria_CA(
                        self._mean_left[cell_under_test_number],
                        self._mean_right[cell_under_test_number]) * threshold_factor
                    threshold_GOCA = self._choose_criteria_GOCA(
                        self._mean_left[cell_under_test_number],
                        self._mean_right[cell_under_test_number]) * threshold_factor
                    threshold_SOCA = self._choose_criteria_SOCA(
                        self._mean_left[cell_under_test_number],
                        self._mean_right[cell_under_test_number]) * threshold_factor

                if threshold_CA < signal[cell_under_test_number]:
                    if cell_under_test_number in object_indexes:
                        detects_count_CA[index] += 1
                    else:
                        false_detects_count_CA[index] += 1
                if threshold_GOCA < signal[cell_under_test_number]:
                    if cell_under_test_number in object_indexes:
                        detects_count_GOCA[index] += 1
                    else:
                        false_detects_count_GOCA[index] += 1
                if threshold_SOCA < signal[cell_under_test_number]:
                    if cell_under_test_number in object_indexes:
                        detects_count_SOCA[index] += 1
                    else:
                        false_detects_count_SOCA[index] += 1
                index += 1

        return (detects_count_CA, false_detects_count_CA, detects_count_GOCA, false_detects_count_GOCA,
                detects_count_SOCA, false_detects_count_SOCA)

    def _calculate_means(self, signal):
        signal = [abs(element) for element in signal]
        last_right_training_cell_number = int(min(self.number_of_guard_cells / 2
                                                  + self.number_of_training_cells, len(signal) - 1))
        last_right_guard_cell_number = int(self.number_of_guard_cells / 2)
        first_left_training_cell_number = int(-self.number_of_guard_cells / 2 - self.number_of_training_cells / 2)
        first_left_guard_cell_number = int(-self.number_of_guard_cells / 2)
        sum_left = 0
        sum_right = sum(signal[last_right_guard_cell_number: last_right_training_cell_number])
        self._mean_left = []
        self._mean_right = []
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
                self._mean_left.append(sum_left / count_left)
            else:
                self._mean_left.append(None)
            if count_right > 0:
                self._mean_right.append(sum_right / count_right)
            else:
                self._mean_right.append(None)

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

        return
