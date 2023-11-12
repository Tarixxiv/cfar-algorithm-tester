

class CFAR_Output_Data:
    def __init__(self):
        self.threshold_value = 0;
        self.detected = False;

class CFAR_CA:
    def __init__(self, number_of_guard_cells, number_of_training_cells, threshold_factor):
        self.number_of_guard_cells = number_of_guard_cells;
        self.number_of_training_cells = number_of_training_cells;
        self.threshold_factor = threshold_factor;
        data=0;


    def _choose_criteria(self, average_left, average_right):
        return (average_left + average_right) / 2;

    def find_objects(self, data):
        output_data = [];
        count_left = 0;
        count_right = self.number_of_training_cells;
        last_right_training_cell_number = int(min(self.number_of_guard_cells / 2 + self.number_of_training_cells, len(data)));
        last_right_guard_cell_number = int(self.number_of_guard_cells / 2);
        first_left_training_cell_number = int(-self.number_of_guard_cells / 2 - self.number_of_training_cells / 2);
        first_left_guard_cell_number = int(-self.number_of_guard_cells / 2);
        sum_left = 0;
        sum_right = sum(data[last_right_guard_cell_number : last_right_training_cell_number]);

        for cell_under_test_number in range(len(data)):
            if(first_left_training_cell_number - 1 >= 0):
                sum_left -= data[first_left_training_cell_number - 1];
            if(first_left_guard_cell_number > 0):
                sum_left += data[first_left_guard_cell_number - 1];
            if(last_right_training_cell_number < len(data)):
                sum_right += data[last_right_training_cell_number];
            if(last_right_guard_cell_number < len(data)):
                sum_right -= data[last_right_guard_cell_number];

            count_left = max(0, first_left_guard_cell_number) - max(0, first_left_training_cell_number);
            count_right = min(last_right_training_cell_number, len(data)) - min(last_right_guard_cell_number, len(data));

            if(count_left > 0):
                average_left = sum_left / count_left;
            if(count_right > 0):
                average_right = sum_right /count_right;
            if(count_left <= 0):
                threshold = average_right * self.threshold_factor;
            elif(count_right <= 0):
                threshold = average_left * self.threshold_factor;
            else:
                threshold = self._choose_criteria(average_left, average_right) *self.threshold_factor;

            output_data.append(CFAR_Output_Data());
            output_data[cell_under_test_number].threshold_value = threshold;
            if(threshold<data[cell_under_test_number]):
                output_data[cell_under_test_number].detected = True;
            else:
                output_data[cell_under_test_number].detected = False;
            last_right_training_cell_number +=1;
            last_right_guard_cell_number += 1;
            first_left_training_cell_number += 1;
            first_left_guard_cell_number += 1;
            if(first_left_training_cell_number <= 0 and first_left_guard_cell_number > 0):
                last_right_training_cell_number -= 1;
                sum_right -= data[last_right_training_cell_number];
            if(last_right_training_cell_number > len(data) and last_right_guard_cell_number <= len(data)):
                first_left_training_cell_number -= 1;
                sum_left += data[first_left_training_cell_number - 1];

        return output_data;
            
class CFAR_GOCA(CFAR_CA):
    def _choose_criteria(self, average_left, average_right):
        return max(average_left, average_right);

class CFAR_SOCA(CFAR_CA):
    def _choose_criteria(self, average_left, average_right):
        return min(average_left, average_right);
