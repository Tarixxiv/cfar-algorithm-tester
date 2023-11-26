import csv
import os


class FinalOutput:
    def __init__(self, output_data=None, input_signal_properties=None):
        if output_data is None:
            output_data = []
        self.output_from_CFAR = output_data
        self.input_signal_properties = input_signal_properties
        self.false_detects = 0
        self.correct_detects = 0
        self.undetected_objects = 0
        self.header = ["dataset_number", "false_detects", "correct_detects", "undetected_objects"]
        self.dataset_number = 0

    def reset(self):
        self.false_detects = 0
        self.correct_detects = 0
        self.undetected_objects = 0

    def export_to_csv(self, filepath=None):
        if filepath is None:
            filepath = "output_data_default_name.csv"
        file_exists = os.path.exists(filepath)
        row = [self.dataset_number, self.false_detects, self.correct_detects, self.undetected_objects]
        with open(filepath, 'a', encoding='UTF8') as csv_file:
            writer = csv.writer(csv_file)
            if not file_exists:
                writer.writerow(self.header)
            writer.writerow(row)

    def analyze_data(self):
        object_number = 0
        record_number = 0
        while record_number < len(self.output_from_CFAR) and object_number < len(self.input_signal_properties.index):
            if self.output_from_CFAR[record_number] > self.input_signal_properties.index[object_number]:
                self.undetected_objects += 1
                object_number += 1
            elif self.output_from_CFAR[record_number] < self.input_signal_properties.index[object_number]:
                self.false_detects += 1
                record_number += 1
            else:
                self.correct_detects += 1
                record_number += 1
                object_number += 1
        if record_number == len(self.output_from_CFAR):
            self.undetected_objects += len(self.input_signal_properties.index) - object_number
        elif object_number == len(self.input_signal_properties.index):
            self.false_detects += len(self.output_from_CFAR) - record_number
