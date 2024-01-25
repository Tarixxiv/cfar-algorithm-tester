#include <iostream>
#include <string>
#include <cMath>
#include <queue>
#include <fstream>

#define LEFT 0
#define RIGHT 1
#define CENTER 2
#define NUMBER_OF_CFAR_TYPES 3

using namespace std;

class cTableOfFloats
{
public:
    float* table_pointer;
    unsigned int length;
    cTableOfFloats(unsigned int length)
    {
        this->length = length;
        this->table_pointer = new float[length];
    }
    ~cTableOfFloats()
    {
        delete table_pointer;
    }
};
class cTwoDimensionalTableOfFloats
{
public:
    float** table_pointer;
    unsigned int number_of_columns, number_of_rows;
    bool destructor_lock = 0;
    cTwoDimensionalTableOfFloats(unsigned int number_of_rows, unsigned int number_of_columns)
    {
        this->number_of_columns = number_of_columns;
        this->number_of_rows = number_of_rows;
        this->table_pointer = new float* [number_of_rows];
        for (int row_number = 0; row_number < number_of_rows; row_number++)
        {
            table_pointer[row_number] = new float[number_of_columns];
        }
    }
    ~cTwoDimensionalTableOfFloats()
    {
        if (destructor_lock)
        {
            destructor_lock = false;
            return;
        }
        for (unsigned int row_number = 0; row_number < number_of_rows; row_number++)
        {
            delete table_pointer[row_number];
        }
        delete table_pointer;
    }
};

class cCFAR
{
private:
    unsigned int uiThresholdFactorTableSize = 0;
    float* pfThresholdFactorTable;
    int number_of_guard_cells = 0;
    int number_of_training_cells = 0;
public:
    enum eCFAR_Types
    {
        CA = 0,
        GOCA = 1,
        SOCA = 2,
        end_of_enum = 3
    };

    cCFAR()
    {
        
    }

    cCFAR(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        this->number_of_guard_cells = number_of_guard_cells;
        this->number_of_training_cells = number_of_training_cells;
        uiThresholdFactorTableSize = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - (float)uiThresholdFactorTableSize >= 0.5)
            uiThresholdFactorTableSize++;
        pfThresholdFactorTable = new float[uiThresholdFactorTableSize];
        for (int iIndex = 0; iIndex < uiThresholdFactorTableSize; iIndex++)
        {
            pfThresholdFactorTable[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }

    }

    ~cCFAR()
    {
        delete(pfThresholdFactorTable);
    }

    float _calculate_threshold(float average_left, float average_right, float average_center, eCFAR_Types cfar_type)
    {
        if(cfar_type == CA)
            return (average_center);
        if (isnan(average_left))
            return average_right;
        else if (isnan(average_right))
            return average_left;
        switch (cfar_type)
        {
        case GOCA:
            return max(average_left, average_right);
        case SOCA:
            return min(average_left, average_right);
        default:
            return nanf("");
        }
    }
    cTwoDimensionalTableOfFloats calculate_all_thresholds_single(float signal[], unsigned int signal_length, float threshold_factor)
    {
        cTwoDimensionalTableOfFloats thresholds(NUMBER_OF_CFAR_TYPES, signal_length);
        cTwoDimensionalTableOfFloats means = calculate_means(signal, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                thresholds.table_pointer[algorithm_type][cell_under_test_number] = _calculate_threshold(means.table_pointer[LEFT][cell_under_test_number], means.table_pointer[RIGHT][cell_under_test_number], means.table_pointer[CENTER][cell_under_test_number], algorithm_type) * threshold_factor;
            }
        thresholds.destructor_lock = true;
        return thresholds;
    }
    cTwoDimensionalTableOfFloats find_objects(float signal[], unsigned int signal_length, queue<int> object_indexes)
    {
        cTwoDimensionalTableOfFloats means = calculate_means(signal, signal_length);
        cTwoDimensionalTableOfFloats detects_and_false_detects_count(2 * NUMBER_OF_CFAR_TYPES, object_indexes.size());

        for (eCFAR_Types algorithm_type; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
        {
            for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
            {
                for (int threshold_factor_index = 0; threshold_factor_index < signal_length; threshold_factor_index++)
                {
                    float threshold = (pfThresholdFactorTable[threshold_factor_index] * _calculate_threshold(means.table_pointer[LEFT][cell_under_test_number], means.table_pointer[RIGHT][cell_under_test_number], means.table_pointer[CENTER][cell_under_test_number], algorithm_type) * pfThresholdFactorTable[threshold_factor_index]);

                    if (threshold < signal[cell_under_test_number])
                        if (cell_under_test_number == object_indexes.front())
                        {
                            detects_and_false_detects_count.table_pointer[algorithm_type][threshold_factor_index] += 1;
                            object_indexes.pop();
                        }
                        else
                            detects_and_false_detects_count.table_pointer[3 + algorithm_type][threshold_factor_index] += 1;
                }
            }
        }
        detects_and_false_detects_count.destructor_lock = true;
        return detects_and_false_detects_count;
    }
    cTwoDimensionalTableOfFloats calculate_means(float signal[], int signal_length)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            if (signal[cell_number] < 0)
                signal[cell_number] = -signal[cell_number];
        }
        int last_right_training_cell_number = min(number_of_guard_cells / 2 + number_of_training_cells, signal_length - 1);
        int last_right_guard_cell_number = number_of_guard_cells / 2;
        int first_left_training_cell_number = -number_of_guard_cells / 2 - number_of_training_cells / 2;
        int first_left_guard_cell_number = -number_of_guard_cells / 2;
        int count_left;
        int count_right;
        float sum_left = 0;
        float sum_right = 0;
        for (int cell_number = last_right_guard_cell_number; cell_number < last_right_training_cell_number; cell_number++)
            sum_right += signal[cell_number];
        cTwoDimensionalTableOfFloats means(3, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
        {
            if (first_left_training_cell_number - 1 >= 0)
                sum_left -= signal[first_left_training_cell_number - 1];
            if (first_left_guard_cell_number > 0)
                sum_left += signal[first_left_guard_cell_number - 1];
            if(last_right_training_cell_number < signal_length)
                sum_right += signal[last_right_training_cell_number];
            if (last_right_guard_cell_number < signal_length)
                sum_right -= signal[last_right_guard_cell_number];

            count_left = max(0, first_left_guard_cell_number) - max(0, first_left_training_cell_number);
            count_right = min(last_right_training_cell_number + 1, signal_length) - min(last_right_guard_cell_number + 1, signal_length);

            if (count_left > 0)
                means.table_pointer[LEFT][cell_under_test_number] = sum_left / count_left;
            else
                means.table_pointer[LEFT][cell_under_test_number] = nanf("");
            if (count_right > 0)
                means.table_pointer[RIGHT][cell_under_test_number] = (sum_right / count_right);
            else
                means.table_pointer[RIGHT][cell_under_test_number] = nanf("");
            means.table_pointer[CENTER][cell_under_test_number] = (sum_right + sum_left) / (count_left + count_right);

            last_right_training_cell_number += 1;
            last_right_guard_cell_number += 1;
            first_left_training_cell_number += 1;
            first_left_guard_cell_number += 1;
            if (first_left_training_cell_number <= 0 && 0 < first_left_guard_cell_number)
            {
                last_right_training_cell_number = min(last_right_training_cell_number - 1, signal_length - 1);
                sum_right -= signal[last_right_training_cell_number];
            }
            if (last_right_training_cell_number >= signal_length && signal_length > last_right_guard_cell_number && first_left_training_cell_number - 1 >= 0)
            {
                first_left_training_cell_number -= 1;
                sum_left += signal[first_left_training_cell_number - 1];
            }
        }
        means.destructor_lock = true;
        return means;
    }
};
int main()
{
    float a[19] = { 1,2,3,4,5,6,7,8,9,10,9,8,7,6,5,4,3,2,1 };
    cCFAR cfar(2, 6, 0, 0, 1);
    cTwoDimensionalTableOfFloats b = cfar.calculate_all_thresholds_single(a, 19, 2);
    std::cout << a[0];
}