class SignalProperties:
    # There will be more properties, signal's shadow for example
    def __init__(self, index_path):
        # Michal wants it as list
        self.index = 0
        self.index_path = index_path

    def read_indexes_from_file(self):
        with open(self.index_path, 'r') as file:
            list = file.readlines()
        lists = []
        for string in list:
            lists.append(string.split(","))
        self.index = lists
