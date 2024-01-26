#include <iostream>
#include <string>
#include <cMath>
#include <queue>
#include <fstream>

#include <random>

#define LEFT 0
#define RIGHT 1
#define CENTER 2
#define NUMBER_OF_CFAR_TYPES 3

using namespace std;

class TableOfFloats
{
public:
    float* table_pointer;
    unsigned int length;
    TableOfFloats(unsigned int length)
    {
        this->length = length;
        this->table_pointer = new float[length];
    }
    ~TableOfFloats()
    {
        delete[] table_pointer;
    }
};
class TwoDimensionalTableOfFloats
{
public:
    float** table_pointer;
    unsigned int number_of_columns, number_of_rows;
    bool destructor_lock = 0;
    TwoDimensionalTableOfFloats(unsigned int number_of_rows, unsigned int number_of_columns)
    {
        this->number_of_columns = number_of_columns;
        this->number_of_rows = number_of_rows;
        this->table_pointer = new float* [number_of_rows];
        for (int row_number = 0; row_number < number_of_rows; row_number++)
        {
            table_pointer[row_number] = new float[number_of_columns];
        }
    }
    ~TwoDimensionalTableOfFloats()
    {
        if (destructor_lock)
        {
            destructor_lock = false;
            return;
        }
        for (unsigned int row_number = 0; row_number < number_of_rows; row_number++)
        {
            delete[] table_pointer[row_number];
        }
        delete[] table_pointer;
    }
};
class TwoDimensionalTableOfDoubles
{
public:
    double** table_pointer = NULL;
    unsigned int number_of_columns = 0, number_of_rows = 0;
    bool destructor_lock = 0;
    TwoDimensionalTableOfDoubles()
    {

    }
    TwoDimensionalTableOfDoubles(unsigned int number_of_rows, unsigned int number_of_columns)
    {
        this->number_of_columns = number_of_columns;
        this->number_of_rows = number_of_rows;
        this->table_pointer = new double* [number_of_rows];
        for (int row_number = 0; row_number < number_of_rows; row_number++)
        {
            table_pointer[row_number] = new double[number_of_columns];
        }
    }
    ~TwoDimensionalTableOfDoubles()
    {
        if (destructor_lock)
        {
            destructor_lock = false;
            return;
        }
        for (unsigned int row_number = 0; row_number < number_of_rows; row_number++)
        {
            delete[] table_pointer[row_number];
        }
        delete[] table_pointer;
    }
};

class CFAR
{
private:
    unsigned int threshold_factor_table_size = 0;
    float* pfThresholdFactorTable = NULL;
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

    CFAR()
    {
        
    }

    CFAR(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        this->number_of_guard_cells = number_of_guard_cells;
        this->number_of_training_cells = number_of_training_cells;
        threshold_factor_table_size = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)threshold_factor_table_size) >= 0.5)
            threshold_factor_table_size++;
        pfThresholdFactorTable = new float[threshold_factor_table_size];
        for (int iIndex = 0; iIndex < threshold_factor_table_size; iIndex++)
        {
            pfThresholdFactorTable[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }
    }

    ~CFAR()
    {
        delete[] pfThresholdFactorTable;
    }

    float calculate_threshold(float average_left, float average_right, float average_center, eCFAR_Types cfar_type)
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
    TwoDimensionalTableOfFloats calculate_all_thresholds_single(float signal[], unsigned int signal_length, float threshold_factor)
    {
        TwoDimensionalTableOfFloats thresholds(NUMBER_OF_CFAR_TYPES, signal_length);
        TwoDimensionalTableOfFloats means = calculate_means(signal, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                thresholds.table_pointer[algorithm_type][cell_under_test_number] = calculate_threshold(means.table_pointer[LEFT][cell_under_test_number], means.table_pointer[RIGHT][cell_under_test_number], means.table_pointer[CENTER][cell_under_test_number], algorithm_type) * threshold_factor;
            }
        thresholds.destructor_lock = true;
        means.destructor_lock = false;
        return thresholds;
    }

    TwoDimensionalTableOfDoubles find_objects(float signal[], unsigned int signal_length, queue<int> object_indexes)
    {
        TwoDimensionalTableOfFloats means = calculate_means(signal, signal_length);
        TwoDimensionalTableOfDoubles detects_and_false_detects_count(2 * NUMBER_OF_CFAR_TYPES, threshold_factor_table_size);
        for (int threshold_factor_index = 0; threshold_factor_index < threshold_factor_table_size; threshold_factor_index++)
        {
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                detects_and_false_detects_count.table_pointer[algorithm_type][threshold_factor_index] = 0;
                detects_and_false_detects_count.table_pointer[algorithm_type+3][threshold_factor_index] = 0;
                for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
                {

                    float threshold = (pfThresholdFactorTable[threshold_factor_index] * calculate_threshold(means.table_pointer[LEFT][cell_under_test_number], means.table_pointer[RIGHT][cell_under_test_number], means.table_pointer[CENTER][cell_under_test_number], algorithm_type) * pfThresholdFactorTable[threshold_factor_index]);

                    if (threshold < signal[cell_under_test_number])
                    {
                        if (!object_indexes.empty() && cell_under_test_number == object_indexes.front())
                        {
                            detects_and_false_detects_count.table_pointer[algorithm_type][threshold_factor_index] += 1;
                            //object_indexes.pop();
                        }
                        else
                        {
                            detects_and_false_detects_count.table_pointer[3 + algorithm_type][threshold_factor_index] += 1;
                        }
                    }
                }
            }
        }
        means.destructor_lock = false;
        detects_and_false_detects_count.destructor_lock = true;
        return detects_and_false_detects_count;
    }
    TwoDimensionalTableOfFloats calculate_means(float signal[], int signal_length)
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
        TwoDimensionalTableOfFloats means(3, signal_length);
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

class Probabilities
{
public:
    double probability_of_detection = 0;
    double probability_of_false_detection = 0;
    int total_cells_number = 0;
    int total_objects_number = 0;

    void calculate_probabilities(double detects_count, double false_detects_count, int data_len, int number_of_objects = 1)
    {
        probability_of_detection = probability_of_detection * total_objects_number + detects_count;
        probability_of_false_detection = probability_of_false_detection * total_cells_number + false_detects_count;
        total_cells_number += data_len;
        total_objects_number += number_of_objects;
        probability_of_detection /= (double)total_objects_number;
        probability_of_false_detection /= (double)total_cells_number;
    }
};


class ProbabilitiesForMultipleThresholdFactors
{
    int uiThresholdFactorTableSize;
    float* pfThresholdFactorTable = NULL;
    Probabilities* probabilities;
public:
    
    string header = "tf, Pd, Pfa\n";

    ProbabilitiesForMultipleThresholdFactors(float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        uiThresholdFactorTableSize = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)uiThresholdFactorTableSize) >= 0.5)
            uiThresholdFactorTableSize++;
        pfThresholdFactorTable = new float[uiThresholdFactorTableSize];
        for (int iIndex = 0; iIndex < uiThresholdFactorTableSize; iIndex++)
        {
            pfThresholdFactorTable[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }
        probabilities = new Probabilities[uiThresholdFactorTableSize];
    }
    ~ProbabilitiesForMultipleThresholdFactors()
    {
        delete[] probabilities;
        delete[] pfThresholdFactorTable;
    }
    void export_to_csv(string filepath)
    {
        ofstream output(filepath);
        output << header;
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            output << pfThresholdFactorTable[index] << ", " << probabilities[index].probability_of_detection << ", " << probabilities[index].probability_of_false_detection << "\n";
        }
    }
    void calculate_probabilities(double detects_count[], double false_detects_count[], int data_len, int number_of_objects = 1)
    {
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].calculate_probabilities(detects_count[index], false_detects_count[index],
                data_len, number_of_objects);
        }
    }
};

class Signal
{
public:
    queue<int> object_index;
    int length;
    float* signal = NULL;

    void signal_generation(float sigma, int length, float snr_dB)
    {
        delete[] signal;
        signal = new float[length];
        this->length = length;
        default_random_engine generator(time(NULL));
        normal_distribution<double> dist(0, sigma);
        srand(time(NULL));
        float signal_from_object = sqrt(pow(10, snr_dB / 10) * sigma * sigma);
        while (!object_index.empty())
            object_index.pop();
        object_index.push(rand() % length);
        for (int index = 0; index < length; index++)
        {
            signal[index] = dist(generator);
        }
        signal[object_index.front()] += signal_from_object;
    }
};



int main()
{
    CFAR cfar(2, 20, 0, 10, 0.1);
    Signal signal;
    ProbabilitiesForMultipleThresholdFactors probabilitiesCA(0, 10, 0.1);
    ProbabilitiesForMultipleThresholdFactors probabilitiesGOCA(0, 10, 0.1);
    ProbabilitiesForMultipleThresholdFactors probabilitiesSOCA(0, 10, 0.1);
    
    for (int i = 0; i < 10000; i++)
    {
        signal.signal_generation(1, 2048, 6);
        TwoDimensionalTableOfDoubles cfar_output = cfar.find_objects(signal.signal, signal.length, signal.object_index);
        cfar_output.destructor_lock = false;
        
        probabilitiesCA.calculate_probabilities(cfar_output.table_pointer[0], cfar_output.table_pointer[3], 2048);
        probabilitiesGOCA.calculate_probabilities(cfar_output.table_pointer[1], cfar_output.table_pointer[4], 2048);
        probabilitiesSOCA.calculate_probabilities(cfar_output.table_pointer[2], cfar_output.table_pointer[5], 2048);
    }
    probabilitiesCA.export_to_csv("output/outputCA.csv");
    probabilitiesGOCA.export_to_csv("output/outputGOCA.csv");
    probabilitiesSOCA.export_to_csv("output/outputSOCA.csv");
}