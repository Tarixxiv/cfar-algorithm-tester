import numpy as np

FLOAT_SIZE = 8
SAMPLE_COUNT = 2048

class BinFileManager:
    def __init__(self, file_path):
        self.file_path = file_path
    
    def clear_file(self):
        with open(self.file_path, "w"):
            pass

    def read_file(self, start_line, finish_line):
        output = []
        with open(self.file_path, "r") as file:
            output.append(np.fromfile(file,
                                      count=SAMPLE_COUNT,
                                      offset=FLOAT_SIZE * start_line * SAMPLE_COUNT))
            for i in range(finish_line - start_line - 1):
                output.append(np.fromfile(file, count=SAMPLE_COUNT))
        return output

    def append_to_file(self, lines):
        with open(self.file_path, "a") as file:
            for line in lines:
                line.tofile(file)
