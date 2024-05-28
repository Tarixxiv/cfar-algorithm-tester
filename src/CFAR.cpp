#include <iostream>
#include <string>
#include <cmath>
#include <queue>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include "Signal.cpp"

#define LEFT 0
#define RIGHT 1
#define CENTER 2

#define CFAR_GUARD_CELLS 8
#define CFAR_TRAINING_CELLS 36
#define CFAR_MIN_TESTED_VALUE 0
#define CFAR_MAX_TESTED_VALUE 30
#define CFAR_DALTA_TESTED_VALUE 0.2f

#define SIGMA 1
#define SNR_dB 12
#define OBJECT_DISTANCE 10
#define DATA_LENGTH 2048
#define TESTS_PER_THREAD 1000
#define NUMBER_OF_THREADS 5
#define SIGNAL_COUNT 2


//using namespace std;
struct CFAROutput
{
    std::vector<int> detectsCA;
    std::vector<int> detectsGOCA;
    std::vector<int> detectsSOCA;
    std::vector<int> false_detectsCA;
    std::vector<int> false_detectsGOCA;
    std::vector<int> false_detectsSOCA;
    CFAROutput(std::vector<std::vector<int>> results)
    {
        detectsCA = results[0];
        detectsGOCA = results[1];
        detectsSOCA = results[2];
        false_detectsCA = results[3];
        false_detectsGOCA = results[4];
        false_detectsSOCA = results[5];
    }
};
class CFAR
{
private:

    int number_of_guard_cells = 0;
    int number_of_training_cells = 0;

protected:
    void init(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        this->number_of_guard_cells = number_of_guard_cells;
        this->number_of_training_cells = number_of_training_cells;
        tested_parameters_table_size = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)tested_parameters_table_size) >= 0.5f)
            tested_parameters_table_size++;
        tested_parameters_table = std::vector<float>(tested_parameters_table_size);
        for (int iIndex = 0; iIndex < tested_parameters_table_size; iIndex++)
        {
            tested_parameters_table[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }
    }

    std::vector<std::vector<float>> calculate_means(std::vector<float> signal, int signal_length)
    {
        int last_right_training_cell_number = std::min(number_of_guard_cells / 2 + number_of_training_cells, signal_length - 1);
        int last_right_guard_cell_number = number_of_guard_cells / 2;
        int first_left_training_cell_number = -number_of_guard_cells / 2 - number_of_training_cells / 2;
        int first_left_guard_cell_number = -number_of_guard_cells / 2;
        int count_left;
        int count_right;
        float sum_left = 0;
        float sum_right = 0;
        for (int cell_number = last_right_guard_cell_number; cell_number < last_right_training_cell_number; cell_number++)
            sum_right += signal[cell_number];
        std::vector<std::vector<float>> means(3);
        for (int i = 0; i < means.size(); i++)
        {
            means[i] = std::vector<float>(signal_length);
        }
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

            count_left = std::max(0, first_left_guard_cell_number) - std::max(0, first_left_training_cell_number);
            count_right = std::min(last_right_training_cell_number + 1, signal_length) - std::min(last_right_guard_cell_number + 1, signal_length);

            if (count_left > 0)
                means[LEFT][cell_under_test_number] = sum_left / count_left;
            else
                means[LEFT][cell_under_test_number] = nanf("");
            if (count_right > 0)
                means[RIGHT][cell_under_test_number] = (sum_right / count_right);
            else
                means[RIGHT][cell_under_test_number] = nanf("");
            means[CENTER][cell_under_test_number] = (sum_right + sum_left) / (count_left + count_right);

            last_right_training_cell_number += 1;
            last_right_guard_cell_number += 1;
            first_left_training_cell_number += 1;
            first_left_guard_cell_number += 1;
            if (first_left_training_cell_number <= 0 && 0 < first_left_guard_cell_number)
            {
                last_right_training_cell_number = std::min(last_right_training_cell_number - 1, signal_length - 1);
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
    std::vector<std::vector<float>> calculate_means_with_zeros_on_both_sides(std::vector<float> signal, int signal_length)
    {
        int last_right_training_cell_number = std::min(number_of_guard_cells / 2 + number_of_training_cells / 2, signal_length - 1);
        int last_right_guard_cell_number = number_of_guard_cells / 2;
        int first_left_training_cell_number = -number_of_guard_cells / 2 - number_of_training_cells / 2;
        int first_left_guard_cell_number = -number_of_guard_cells / 2;
        int count_left;
        int count_right;
        float sum_left = 0;
        float sum_right = 0;
        count_left = number_of_training_cells / 2;
        count_right = number_of_training_cells / 2;
        for (int cell_number = last_right_guard_cell_number; cell_number < last_right_training_cell_number; cell_number++)
            sum_right += signal[cell_number];
        std::vector<std::vector<float>> means(3);
        for (int i = 0; i < means.size(); i++)
        {
            means[i] = std::vector<float>(signal_length);
        }
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

            if (count_left > 0)
                means[LEFT][cell_under_test_number] = sum_left / count_left;
            else
                means[LEFT][cell_under_test_number] = nanf("");
            if (count_right > 0)
                means[RIGHT][cell_under_test_number] = (sum_right / count_right);
            else
                means[RIGHT][cell_under_test_number] = nanf("");
            means[CENTER][cell_under_test_number] = (sum_right + sum_left) / (count_left + count_right);

            last_right_training_cell_number += 1;
            last_right_guard_cell_number += 1;
            first_left_training_cell_number += 1;
            first_left_guard_cell_number += 1;
            if (first_left_training_cell_number <= 0 && 0 < first_left_guard_cell_number)
            {
                last_right_training_cell_number = std::min(last_right_training_cell_number - 1, signal_length - 1);
            }
            if (last_right_training_cell_number >= signal_length && signal_length > last_right_guard_cell_number && first_left_training_cell_number - 1 >= 0)
            {
                first_left_training_cell_number -= 1;
            }
        }
        return means;
    }


    std::vector<float> add_number_to_the_median(std::vector<float> median_vector, int number)
    {
        median_vector.push_back(number);
        std::sort(median_vector.begin(), median_vector.end());

        return median_vector;
    }

    std::vector<float> remove_number_from_the_median(std::vector<float> median_vector, int number)
    {
        int index = 0;
        for (int i = 0; i < median_vector.size(); i++) {
            if (median_vector[i] == number) {
                index = i;
                break;
            }
        }
        median_vector.erase(median_vector.begin() + index);

        return median_vector;
    }

    std::vector<float> calculate_medians_with_zeros_on_both_sides(std::vector<float> signal, int signal_length)
    {
        int last_right_training_cell_number = number_of_training_cells - number_of_guard_cells + std::min(number_of_guard_cells / 2 + number_of_training_cells / 2, signal_length - 1);
        int last_right_guard_cell_number = number_of_training_cells - number_of_guard_cells + number_of_guard_cells / 2;
        int first_left_training_cell_number = number_of_training_cells - number_of_guard_cells - number_of_guard_cells / 2 - number_of_training_cells / 2;
        int first_left_guard_cell_number = number_of_training_cells - number_of_guard_cells - number_of_guard_cells / 2;

        std::vector<float> vector_for_median;
        std::vector<float> median;
        float median_value = 0;

        signal.insert(signal.begin(), number_of_training_cells - number_of_guard_cells, 0);
        signal.insert(signal.end(), number_of_training_cells - number_of_guard_cells + 1, 0);
        for (int i = 0; i < 2 * (number_of_training_cells - number_of_guard_cells) + 1; i++) {
            if (i != number_of_training_cells - number_of_guard_cells) {
                vector_for_median.push_back(signal[i]);
            }
        }
        std::sort(vector_for_median.begin(), vector_for_median.end());

        for (int cell_under_test_number = number_of_training_cells - number_of_guard_cells; cell_under_test_number < signal_length + number_of_training_cells - number_of_guard_cells; cell_under_test_number++)
        {
            if (vector_for_median.size() > 0) {
                if (vector_for_median.size() % 2 == 1) {
                    median_value = vector_for_median[vector_for_median.size() / 2 + 0.5];
                }
                else {
                    int help = vector_for_median.size() / 2;
                    median_value = (vector_for_median[vector_for_median.size() / 2 - 1] + vector_for_median[vector_for_median.size() / 2]) / 2;
                }
                median.push_back(median_value);
            }

            last_right_training_cell_number += 1;
            last_right_guard_cell_number += 1;
            first_left_training_cell_number += 1;
            first_left_guard_cell_number += 1;
            if (last_right_training_cell_number >= signal_length && signal_length > last_right_guard_cell_number && first_left_training_cell_number - 1 >= 0)
            {
                first_left_training_cell_number -= 1;
            }

            vector_for_median = remove_number_from_the_median(vector_for_median, signal[first_left_training_cell_number]);
            vector_for_median = add_number_to_the_median(vector_for_median, signal[first_left_guard_cell_number]);
            vector_for_median = add_number_to_the_median(vector_for_median, signal[last_right_training_cell_number - 1]);
            vector_for_median = remove_number_from_the_median(vector_for_median, signal[last_right_guard_cell_number - 1]);

        }
        return median;
    }

public:
    enum class CFAR_Types
    {
        CA = 0,
        GOCA = 1,
        SOCA = 2,
        end_of_enum = 3
    };

    enum class TestedValues
    {
        none = -1,
        threshold_factor = 0,
        threshold_offset = 1,
    };

    unsigned int tested_parameters_table_size = 0;
    std::vector<float> tested_parameters_table;
    TestedValues tested_parameter = TestedValues::none;

    CFAR()
    {
        init(CFAR_GUARD_CELLS, CFAR_TRAINING_CELLS, CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
    }

    CFAR(CFAR& cfar)
    {
        this->number_of_guard_cells = cfar.number_of_guard_cells;
        this->number_of_training_cells = cfar.number_of_training_cells;
        this->tested_parameters_table_size = cfar.tested_parameters_table_size;
        this->tested_parameters_table = std::vector<float>(tested_parameters_table_size);
        for (int index = 0; index < tested_parameters_table_size; index++)
        {
            this->tested_parameters_table[index] = cfar.tested_parameters_table[index];
        }
    }

    CFAR(unsigned int number_of_guard_cells, unsigned int number_of_training_cells, float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        init(number_of_guard_cells, number_of_training_cells, threshold_factor_min, threshold_factor_max, threshold_factor_delta);
    }

    ~CFAR()
    {

    }

    float calculate_threshold(float average_left, float average_right, float average_center, CFAR_Types cfar_type)
    {
        if (cfar_type == CFAR_Types::CA)
            return (average_center);
        if (std::isnan(average_left))
            return average_right;
        else if (std::isnan(average_right))
            return average_left;
        switch (cfar_type)
        {
        case CFAR_Types::GOCA:
            return std::max(average_left, average_right);
        case CFAR_Types::SOCA:
            return std::min(average_left, average_right);
        default:
            return nanf("");
        }
    }

    std::vector<std::vector<float>> calculate_all_thresholds_single(std::vector<float> signal, unsigned int signal_length, float threshold_factor)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            signal[cell_number] = signal[cell_number] * signal[cell_number];
        }
        std::vector<std::vector<float>> thresholds((int)CFAR_Types::end_of_enum);
        for (int i = 0; i < thresholds.size(); i++)
        {
            thresholds[i] = std::vector<float>(signal_length);
        }
        std::vector<std::vector<float>> means = calculate_means(signal, signal_length);
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
            for (CFAR_Types algorithm_type = (CFAR_Types)0; algorithm_type < CFAR_Types::end_of_enum; algorithm_type = (CFAR_Types)((int)algorithm_type + 1))
            {
                thresholds[(int)algorithm_type][cell_under_test_number] = calculate_threshold(means[LEFT][cell_under_test_number], means[RIGHT][cell_under_test_number], means[CENTER][cell_under_test_number], algorithm_type) * threshold_factor;
            }
        return thresholds;
    }

    CFAROutput find_objects(std::vector<float> signal, unsigned int signal_length, std::queue<int> object_indexes_queue, float threshold_factor = 1)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            signal[cell_number] = signal[cell_number] * signal[cell_number];
        }
        std::vector<std::vector<float>> means = calculate_means(signal, signal_length);
        std::vector< std::vector<int> > detects_and_false_detects_count(2 * (int)CFAR_Types::end_of_enum);
        for (int i = 0; i < detects_and_false_detects_count.size(); i++)
        {
            detects_and_false_detects_count[i] = std::vector<int>(tested_parameters_table_size);
        }
        for (int threshold_offset_index = 0; threshold_offset_index < tested_parameters_table_size; threshold_offset_index++)
        {
            for (CFAR_Types algorithm_type = (CFAR_Types)0; algorithm_type < CFAR_Types::end_of_enum; algorithm_type = (CFAR_Types)((int)algorithm_type + 1))
            {
                detects_and_false_detects_count[(int)algorithm_type][threshold_offset_index] = 0;
                detects_and_false_detects_count[(int)algorithm_type + 3][threshold_offset_index] = 0;
            }
        }
        for (int cell_under_test_number = 0; cell_under_test_number < signal_length; cell_under_test_number++)
        {
            for (CFAR_Types algorithm_type = (CFAR_Types)0; algorithm_type < CFAR_Types::end_of_enum; algorithm_type = (CFAR_Types)((int)algorithm_type + 1))
            {
                float threshold_base = calculate_threshold(means[LEFT][cell_under_test_number], means[RIGHT][cell_under_test_number], means[CENTER][cell_under_test_number], algorithm_type);
                for (int tested_values_index = 0; tested_values_index < tested_parameters_table_size; tested_values_index++)
                {
                    float threshold;
                    switch (tested_parameter)
                    {
                    case TestedValues::threshold_factor:
                        threshold = threshold_base * tested_parameters_table[tested_values_index];
                        break;
                    case TestedValues::threshold_offset:
                        threshold = threshold_factor * threshold_base + tested_parameters_table[tested_values_index];
                        break;
                    default:
                        threshold = threshold_base;
                        break;
                    }


                    if (threshold < signal[cell_under_test_number])
                    {
                        if (!object_indexes_queue.empty() && cell_under_test_number == object_indexes_queue.front())
                        {
                            detects_and_false_detects_count[(int)algorithm_type][tested_values_index] += 1;
                        }
                        else
                        {
                            detects_and_false_detects_count[3 + (int)algorithm_type][tested_values_index] += 1;
                        }
                    }
                    else
                        break;
                }
            }
            if (!object_indexes_queue.empty() && cell_under_test_number == object_indexes_queue.front())
            {
                object_indexes_queue.pop();
            }
        }
        CFAROutput output(detects_and_false_detects_count);
        return output;
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
        number_of_false_detects += false_detects_count;
        total_cells_number += data_len - number_of_objects;
        total_objects_number += number_of_objects;
    }
    void add(Probabilities& probabilities)
    {
        number_of_detected_objects += probabilities.number_of_detected_objects;
        number_of_false_detects += probabilities.number_of_false_detects;
        total_cells_number += probabilities.total_cells_number;
        total_objects_number += probabilities.total_objects_number;
    }
    void calculate_probabilities()
    {
        probability_of_detection = (float)number_of_detected_objects / (float)total_objects_number;
        probability_of_false_detection = (float)number_of_false_detects / (float)total_cells_number;
    }

};

class ProbabilitiesForMultipleThresholdFactors
{
    int tested_value_table_size = 0;
    std::vector<float> tested_parameter_table_pointer;
    std::vector<Probabilities> probabilities;
    void init(float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        tested_value_table_size = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)tested_value_table_size) >= 0.5f)
            tested_value_table_size++;
        tested_parameter_table_pointer = std::vector<float>(tested_value_table_size);
        for (int iIndex = 0; iIndex < tested_value_table_size; iIndex++)
        {
            tested_parameter_table_pointer[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex;
        }
        probabilities = std::vector<Probabilities>(tested_value_table_size);
    }
public:

    std::string header = "tf, Pd, Pfa\n";

    ProbabilitiesForMultipleThresholdFactors(float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        init(threshold_factor_min, threshold_factor_max, threshold_factor_delta);
    }

    ProbabilitiesForMultipleThresholdFactors()
    {
        init(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
    }

    ~ProbabilitiesForMultipleThresholdFactors()
    {
    }

    void export_to_csv(std::string filepath)
    {
        for (int index = 0; index < tested_value_table_size; index++)
        {
            probabilities[index].calculate_probabilities();
        }
        std::ofstream output(filepath);
        output << header;
        for (int index = 0; index < tested_value_table_size; index++)
        {
            output << tested_parameter_table_pointer[index] << ", " << probabilities[index].probability_of_detection << ", " << probabilities[index].probability_of_false_detection << "\n";
        }
    }

    void add(std::vector<int> detects_count, std::vector<int> false_detects_count, unsigned int data_len, unsigned int number_of_objects = 1)
    {
        for (int index = 0; index < tested_value_table_size; index++)
        {
            probabilities[index].add((unsigned int)detects_count[index], (unsigned int)false_detects_count[index], data_len, number_of_objects);
        }
    }
    void add(ProbabilitiesForMultipleThresholdFactors& results_to_add)
    {
        for (int index = 0; index < tested_value_table_size; index++)
        {
            probabilities[index].add(results_to_add.probabilities[index]);
        }
    }
};

class SimulationThread
{
public:
    CFAR cfar;
    Signal signal;
    ProbabilitiesForMultipleThresholdFactors probabilitiesCA;
    ProbabilitiesForMultipleThresholdFactors probabilitiesGOCA;
    ProbabilitiesForMultipleThresholdFactors probabilitiesSOCA;

    SimulationThread(CFAR& cfar, Signal& signal)
    {
        this->signal = Signal(signal);
        this->cfar = CFAR(cfar);
        probabilitiesCA = ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
        probabilitiesGOCA = ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
        probabilitiesSOCA = ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
    }

    void export_to_csv()
    {
        probabilitiesCA.export_to_csv("output/outputCA.csv");
        probabilitiesGOCA.export_to_csv("output/outputGOCA.csv");
        probabilitiesSOCA.export_to_csv("output/outputSOCA.csv");
    }

    void start_simulation(int number_of_tests, int number_of_thread = 0)
    {
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        std::cout << "thread " << number_of_thread << " started\n";
        for (int counter = 0; counter < number_of_tests; counter++)
        {
            signal.signal_and_noise_generation(OBJECT_DISTANCE);
            cfar.tested_parameter = CFAR::TestedValues::threshold_factor;
            CFAROutput cfar_output = cfar.find_objects(signal.samples, signal.length, signal.object_indexes_queue);

            probabilitiesCA.add(cfar_output.detectsCA, cfar_output.false_detectsCA, signal.length);
            probabilitiesGOCA.add(cfar_output.detectsGOCA, cfar_output.false_detectsGOCA, signal.length);
            probabilitiesSOCA.add(cfar_output.detectsSOCA, cfar_output.false_detectsSOCA, signal.length);
            if (counter % 1000 == 0)
                std::cout << "thread " << number_of_thread << " reached " << counter << " tests\n";
        }
        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        std::cout << "thread " << number_of_thread << " finished\n";// after " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;

    }
    ~SimulationThread()
    {

    }
    void add(SimulationThread& finished_simulation)
    {
        probabilitiesCA.add(finished_simulation.probabilitiesCA);
        probabilitiesGOCA.add(finished_simulation.probabilitiesGOCA);
        probabilitiesSOCA.add(finished_simulation.probabilitiesSOCA);
    }
};

int main()
{
    CFAR cfar(CFAR_GUARD_CELLS, CFAR_TRAINING_CELLS,
        CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE,
        CFAR_DALTA_TESTED_VALUE);
    float amplitude = (float)pow(10, (float)SNR_dB / 20) * SIGMA;
    Signal signal(SIGMA, DATA_LENGTH, SNR_dB, CFAR_GUARD_CELLS,
        { amplitude, amplitude * 1000 });
    SimulationThread* simulation[NUMBER_OF_THREADS];

    std::thread simulation_thread[NUMBER_OF_THREADS];

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int number_of_thread = 0; number_of_thread < NUMBER_OF_THREADS; number_of_thread++)
    {
        simulation[number_of_thread] = new SimulationThread(cfar, signal);
        simulation_thread[number_of_thread] = std::thread(&SimulationThread::start_simulation, simulation[number_of_thread], TESTS_PER_THREAD, number_of_thread);
    }
    for (int number_of_thread = 0; number_of_thread < NUMBER_OF_THREADS; number_of_thread++)
    {
        simulation_thread[number_of_thread].join();
    }
    for (int number_of_thread = 1; number_of_thread < NUMBER_OF_THREADS; number_of_thread++)
    {
        simulation[0]->add(*(simulation[number_of_thread]));
    }
    simulation[0]->export_to_csv();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Simulation finished after " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    std::cin.get();
}
