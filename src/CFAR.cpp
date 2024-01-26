#include <iostream>
#include <string>
#include <cMath>
#include <queue>
#include <fstream>
#include <chrono>

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
    float** table_pointer = NULL;
    unsigned int number_of_columns = 0, number_of_rows = 0;
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

class TwoDimensionalTableOfUInts
{
public:
    unsigned int** table_pointer = NULL;
    unsigned int number_of_columns = 0, number_of_rows = 0;
    bool destructor_lock = 0;
    TwoDimensionalTableOfUInts()
    {

    }
    TwoDimensionalTableOfUInts(unsigned int number_of_rows, unsigned int number_of_columns)
    {
        this->number_of_columns = number_of_columns;
        this->number_of_rows = number_of_rows;
        this->table_pointer = new unsigned int* [number_of_rows];
        for (int row_number = 0; row_number < number_of_rows; row_number++)
        {
            table_pointer[row_number] = new unsigned int[number_of_columns];
        }
    }
    ~TwoDimensionalTableOfUInts()
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
    unsigned int tested_parameters_table_size = 0;
    float* tested_parameters_table = NULL;
    int number_of_guard_cells = 0;
    int number_of_training_cells = 0;
    void init(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        this->number_of_guard_cells = number_of_guard_cells;
        this->number_of_training_cells = number_of_training_cells;
        tested_parameters_table_size = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)tested_parameters_table_size) >= 0.5)
            tested_parameters_table_size++;
        tested_parameters_table = new float[tested_parameters_table_size];
        for (int iIndex = 0; iIndex < tested_parameters_table_size; iIndex++)
        {
            tested_parameters_table[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }
    }
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
        std::ifstream parameters_file("config/CFAR_parameters.json", std::ifstream::binary);
        //Json::Value parameters;
        //parameters_file >> parameters;
        parameters_file.close();


    }

    CFAR(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        init(number_of_guard_cells, number_of_training_cells, threshold_factor_min, threshold_factor_max, threshold_factor_delta);
    }

    ~CFAR()
    {
        delete[] tested_parameters_table;
    }

    float calculate_threshold(float average_left, float average_right, float average_center, eCFAR_Types cfar_type)
    {
        if (cfar_type == CA)
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
        TwoDimensionalTableOfFloats* means = calculate_means(signal, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                thresholds.table_pointer[algorithm_type][cell_under_test_number] = calculate_threshold(means->table_pointer[LEFT][cell_under_test_number], means->table_pointer[RIGHT][cell_under_test_number], means->table_pointer[CENTER][cell_under_test_number], algorithm_type) * threshold_factor;
            }
        thresholds.destructor_lock = true;
        delete means;
        return thresholds;
    }

    TwoDimensionalTableOfUInts find_objects_for_multiple_threshold_factors(float signal[], unsigned int signal_length, queue<int> object_indexes)
    {
        TwoDimensionalTableOfFloats* means = calculate_means(signal, signal_length);
        TwoDimensionalTableOfUInts detects_and_false_detects_count(2 * NUMBER_OF_CFAR_TYPES, tested_parameters_table_size);
        for (int threshold_factor_index = 0; threshold_factor_index < tested_parameters_table_size; threshold_factor_index++)
        {
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                detects_and_false_detects_count.table_pointer[algorithm_type][threshold_factor_index] = 0;
                detects_and_false_detects_count.table_pointer[algorithm_type + 3][threshold_factor_index] = 0;
            }
        }
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
        {
        
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                for (int threshold_factor_index = 0; threshold_factor_index < tested_parameters_table_size; threshold_factor_index++)
                {
                    float threshold = (calculate_threshold(means->table_pointer[LEFT][cell_under_test_number], means->table_pointer[RIGHT][cell_under_test_number], means->table_pointer[CENTER][cell_under_test_number], algorithm_type) * tested_parameters_table[threshold_factor_index]);

                    if (threshold < signal[cell_under_test_number])
                    {
                        if (!object_indexes.empty() && cell_under_test_number == object_indexes.front())
                        {
                            detects_and_false_detects_count.table_pointer[algorithm_type][threshold_factor_index] += 1;
                        }
                        else
                        {
                            detects_and_false_detects_count.table_pointer[3 + algorithm_type][threshold_factor_index] += 1;
                        }
                    }
                    else
                        break;
                }
            }
            if (!object_indexes.empty() && cell_under_test_number == object_indexes.front())
            {
                object_indexes.pop();
            }
        }
        delete means;
        detects_and_false_detects_count.destructor_lock = true;
        return detects_and_false_detects_count;
    }
    TwoDimensionalTableOfUInts find_objects_for_multiple_offsets(float signal[], unsigned int signal_length, queue<int> object_indexes, float threshold_factor = 1)
    {
        TwoDimensionalTableOfFloats* means = calculate_means(signal, signal_length);
        TwoDimensionalTableOfUInts detects_and_false_detects_count(2 * NUMBER_OF_CFAR_TYPES, tested_parameters_table_size);
        for (int threshold_offset_index = 0; threshold_offset_index < tested_parameters_table_size; threshold_offset_index++)
        {
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                detects_and_false_detects_count.table_pointer[algorithm_type][threshold_offset_index] = 0;
                detects_and_false_detects_count.table_pointer[algorithm_type + 3][threshold_offset_index] = 0;
            }
        }
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
        {
            for (eCFAR_Types algorithm_type = (eCFAR_Types)0; algorithm_type < end_of_enum; algorithm_type = (eCFAR_Types)((int)algorithm_type + 1))
            {
                for (int threshold_offset_index = 0; threshold_offset_index < tested_parameters_table_size; threshold_offset_index++)
                {
                    float threshold = threshold_factor * calculate_threshold(means->table_pointer[LEFT][cell_under_test_number], means->table_pointer[RIGHT][cell_under_test_number], means->table_pointer[CENTER][cell_under_test_number], algorithm_type) + tested_parameters_table[threshold_offset_index];

                    if (threshold < signal[cell_under_test_number])
                    {
                        if (!object_indexes.empty() && cell_under_test_number == object_indexes.front())
                        {
                            detects_and_false_detects_count.table_pointer[algorithm_type][threshold_offset_index] += 1;
                            //object_indexes.pop();
                        }
                        else
                        {
                            detects_and_false_detects_count.table_pointer[3 + algorithm_type][threshold_offset_index] += 1;
                        }
                    }
                    else
                        break;
                }
            }
        }
        detects_and_false_detects_count.destructor_lock = true;
        return detects_and_false_detects_count;
    }
    TwoDimensionalTableOfFloats* calculate_means(float signal[], int signal_length)
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
        TwoDimensionalTableOfFloats* means = new TwoDimensionalTableOfFloats(3, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
        {
            if (first_left_training_cell_number - 1 >= 0)
                sum_left -= signal[first_left_training_cell_number - 1];
            if (first_left_guard_cell_number > 0)
                sum_left += signal[first_left_guard_cell_number - 1];
            if (last_right_training_cell_number < signal_length)
                sum_right += signal[last_right_training_cell_number];
            if (last_right_guard_cell_number < signal_length)
                sum_right -= signal[last_right_guard_cell_number];

            count_left = max(0, first_left_guard_cell_number) - max(0, first_left_training_cell_number);
            count_right = min(last_right_training_cell_number + 1, signal_length) - min(last_right_guard_cell_number + 1, signal_length);

            if (count_left > 0)
                means->table_pointer[LEFT][cell_under_test_number] = sum_left / count_left;
            else
                means->table_pointer[LEFT][cell_under_test_number] = nanf("");
            if (count_right > 0)
                means->table_pointer[RIGHT][cell_under_test_number] = (sum_right / count_right);
            else
                means->table_pointer[RIGHT][cell_under_test_number] = nanf("");
            means->table_pointer[CENTER][cell_under_test_number] = (sum_right + sum_left) / (count_left + count_right);

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
        return means;
    }
};

class Probabilities
{
public:
    double probability_of_detection = 0;
    double probability_of_false_detection = 0;
    int number_of_detected_objects = 0;
    long long int number_of_false_detects = 0;
    int total_cells_number = 0;
    int total_objects_number = 0;
    void add(unsigned int detects_count, unsigned int false_detects_count, int data_len, int number_of_objects = 1)
    {
        number_of_detected_objects += detects_count;
        number_of_false_detects +=false_detects_count;
        total_cells_number +=data_len;
        total_objects_number +=number_of_objects;
    }
    void calculate_probabilities()
    {
        probability_of_detection = (float)number_of_detected_objects / (float)total_objects_number;
        probability_of_false_detection = (float)number_of_false_detects / (float)total_cells_number;
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
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].calculate_probabilities();
        }
        ofstream output(filepath);
        output << header;
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            output << pfThresholdFactorTable[index] << ", " << probabilities[index].probability_of_detection << ", " << probabilities[index].probability_of_false_detection << "\n";
        }
    }

    void add(unsigned int detects_count[], unsigned int false_detects_count[], unsigned int data_len, unsigned int number_of_objects = 1)
    {
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].add(detects_count[index], false_detects_count[index], data_len, number_of_objects);
        }
    }
};

class Signal
{
public:
    queue<int> object_index;
    int length = 0;
    float* signal = NULL;
    default_random_engine* generator;
    Signal()
    {
        generator = new default_random_engine((time(NULL)));
    }
    void signal_generation(float sigma, int length, float snr_dB)
    {
        delete[] signal;
        signal = new float[length];
        this->length = length;
        normal_distribution<float> dist(0, sigma);
        srand(time(NULL));
        float signal_from_object = sqrt(pow(10, snr_dB / 10) * sigma * sigma);
        while (!object_index.empty())
            object_index.pop();
        object_index.push(rand() % length);
        for (int index = 0; index < length; index++)
        {
            signal[index] = dist(*generator);
        }
        signal[object_index.front()] += signal_from_object;
    }

    ~Signal()
    {
        delete generator;
        delete[] signal;
    }
};

int main()
{
    CFAR cfar(2, 20, 0, 10, 0.1);
    Signal signal;
    ProbabilitiesForMultipleThresholdFactors probabilitiesCA(0, 10, 0.1);
    ProbabilitiesForMultipleThresholdFactors probabilitiesGOCA(0, 10, 0.1);
    ProbabilitiesForMultipleThresholdFactors probabilitiesSOCA(0, 10, 0.1);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int i = 0; i < 100000; i++)
    {
        signal.signal_generation(10, 2048, 3);
        TwoDimensionalTableOfUInts cfar_output = cfar.find_objects_for_multiple_threshold_factors(signal.signal, signal.length, signal.object_index);
        cfar_output.destructor_lock = false;

        probabilitiesCA.add(cfar_output.table_pointer[0], cfar_output.table_pointer[3], 2048);
        probabilitiesGOCA.add(cfar_output.table_pointer[1], cfar_output.table_pointer[4], 2048);
        probabilitiesSOCA.add(cfar_output.table_pointer[2], cfar_output.table_pointer[5], 2048);
    }

    probabilitiesCA.export_to_csv("output/outputCA.csv");
    probabilitiesGOCA.export_to_csv("output/outputGOCA.csv");
    probabilitiesSOCA.export_to_csv("output/outputSOCA.csv");

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    cin.get();
}