#include <iostream>
#include <string>
#include <cMath>
#include <queue>
#include <fstream>
#include <chrono>
#include <thread>

#include <random>

#define LEFT 0
#define RIGHT 1
#define CENTER 2
#define NUMBER_OF_CFAR_TYPES 3

#define CFAR_GUARD_CELLS 8
#define CFAR_TRAINING_CELLS 32
#define CFAR_MIN_TESTED_VALUE 0
#define CFAR_MAX_TESTED_VALUE 30
#define CFAR_DALTA_TESTED_VALUE 0.2

#define SIGMA 1
#define SNR_dB 12
#define DATA_LENGTH 2048
#define TESTS_PER_THREAD 10000
#define NUMBER_OF_THREADS 1

//using namespace std;

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
    
    int number_of_guard_cells = 0;
    int number_of_training_cells = 0;

protected:
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

    TwoDimensionalTableOfFloats* calculate_means(float signal[], int signal_length)
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

            count_left = std::max(0, first_left_guard_cell_number) - std::max(0, first_left_training_cell_number);
            count_right = std::min(last_right_training_cell_number + 1, signal_length) - std::min(last_right_guard_cell_number + 1, signal_length);

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
public:
    unsigned int tested_parameters_table_size = 0;
    float* tested_parameters_table = NULL;

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

    CFAR(CFAR &cfar)
    {
        this->number_of_guard_cells = cfar.number_of_guard_cells;
        this->number_of_training_cells = cfar.number_of_training_cells;
        this->tested_parameters_table_size = cfar.tested_parameters_table_size;
        this->tested_parameters_table = new float[tested_parameters_table_size];
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
            return std::max(average_left, average_right);
        case SOCA:
            return std::min(average_left, average_right);
        default:
            return nanf("");
        }
    }

    TwoDimensionalTableOfFloats calculate_all_thresholds_single(float signal[], unsigned int signal_length, float threshold_factor)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            signal[cell_number] = signal[cell_number] * signal[cell_number];
            //if (signal[cell_number] < 0)
            //    signal[cell_number] = -signal[cell_number];
        }
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

    TwoDimensionalTableOfUInts find_objects_for_multiple_threshold_factors(float signal[], unsigned int signal_length, std::queue<int> object_indexes)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            signal[cell_number] = signal[cell_number] * signal[cell_number];
            //if (signal[cell_number] < 0)
            //    signal[cell_number] = -signal[cell_number];
        }
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

    TwoDimensionalTableOfUInts find_objects_for_multiple_offsets(float signal[], unsigned int signal_length, std::queue<int> object_indexes, float threshold_factor = 1)
    {
        for (int cell_number = 0; cell_number < signal_length; cell_number++)
        {
            signal[cell_number] = signal[cell_number] * signal[cell_number];
            //if (signal[cell_number] < 0)
            //    signal[cell_number] = -signal[cell_number];
        }
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
                    float threshold = /*threshold_factor * */calculate_threshold(means->table_pointer[LEFT][cell_under_test_number], means->table_pointer[RIGHT][cell_under_test_number], means->table_pointer[CENTER][cell_under_test_number], algorithm_type) + tested_parameters_table[threshold_offset_index];

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
        delete means;
        return detects_and_false_detects_count;
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
        total_cells_number +=data_len - number_of_objects;
        total_objects_number +=number_of_objects;
    }
    void add(Probabilities &probabilities)
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
    int uiThresholdFactorTableSize = 0;
    float* tested_parameter_table_pointer = NULL;
    Probabilities* probabilities = NULL;
public:

    std::string header = "tf, Pd, Pfa\n";

    ProbabilitiesForMultipleThresholdFactors(float threshold_factor_min, float threshold_factor_max, float threshold_factor_delta)
    {
        uiThresholdFactorTableSize = (threshold_factor_max - threshold_factor_min) / threshold_factor_delta;
        if ((threshold_factor_max - threshold_factor_min) / threshold_factor_delta - ((float)uiThresholdFactorTableSize) >= 0.5)
            uiThresholdFactorTableSize++;
        tested_parameter_table_pointer = new float[uiThresholdFactorTableSize];
        for (int iIndex = 0; iIndex < uiThresholdFactorTableSize; iIndex++)
        {
            tested_parameter_table_pointer[iIndex] = threshold_factor_min + threshold_factor_delta * iIndex; 
        }
        probabilities = new Probabilities[uiThresholdFactorTableSize];
    }

    ~ProbabilitiesForMultipleThresholdFactors()
    {
        delete[] probabilities;
        delete[] tested_parameter_table_pointer;
    }

    void export_to_csv(std::string filepath)
    {
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].calculate_probabilities();
        }
        std::ofstream output(filepath);
        output << header;
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            output << tested_parameter_table_pointer[index] << ", " << probabilities[index].probability_of_detection << ", " << probabilities[index].probability_of_false_detection << "\n";
        }
    }

    void add(unsigned int detects_count[], unsigned int false_detects_count[], unsigned int data_len, unsigned int number_of_objects = 1)
    {
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].add(detects_count[index], false_detects_count[index], data_len, number_of_objects);
        }
    }
    void add(ProbabilitiesForMultipleThresholdFactors &results_to_add)
    {
        for (int index = 0; index < uiThresholdFactorTableSize; index++)
        {
            probabilities[index].add(results_to_add.probabilities[index]);
        }
    }
};

class Signal
{
public:
    std::queue<int> object_index;
    int length = 0;
    float sigma = 1;
    float snr_dB = 12;
    float* signal = NULL;
    std::default_random_engine generator;
    Signal()
    {
        generator = std::default_random_engine((time(NULL)));
    }
    Signal(float sigma, int length, float snr_dB)
    {
        generator = std::default_random_engine((time(NULL)));
        this->length = length;
        this->sigma = sigma;
        this->snr_dB = snr_dB;
    }
    void signal_generation()
    {
        delete[] signal;
        signal = new float[length];
        std::normal_distribution<float> dist(0, sigma);
        srand(time(NULL));
        float signal_from_object = pow(10, snr_dB / 20) * sigma;
        while (!object_index.empty())
            object_index.pop();
        object_index.push(rand() % length);
        float b = dist(generator);
        for (int index = 0; index < length; index++)
        {
            signal[index] = dist(generator);
        }
        signal[object_index.front()] += signal_from_object;
    }

    ~Signal()
    {
        delete[] signal;
    }
};

class SimulationThread
{
public:
    CFAR cfar;
    Signal signal;
    ProbabilitiesForMultipleThresholdFactors *probabilitiesCA = NULL;
    ProbabilitiesForMultipleThresholdFactors *probabilitiesGOCA = NULL;
    ProbabilitiesForMultipleThresholdFactors *probabilitiesSOCA = NULL;

    SimulationThread(CFAR &cfar, Signal &signal)
    {
        this->signal = Signal(signal);
        this->cfar = CFAR(cfar);
        probabilitiesCA = new ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
        probabilitiesGOCA = new ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
        probabilitiesSOCA = new ProbabilitiesForMultipleThresholdFactors(CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
    }
    void export_to_csv()
    {
        probabilitiesCA->export_to_csv("output/outputCA.csv");
        probabilitiesGOCA->export_to_csv("output/outputGOCA.csv");
        probabilitiesSOCA->export_to_csv("output/outputSOCA.csv");
    }

    void start_simulation(int number_of_tests, int number_of_thread = 0)
    {
        //std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        for (int counter = 0; counter < number_of_tests; counter++)
        {
            signal.signal_generation();
            TwoDimensionalTableOfUInts cfar_output = cfar.find_objects_for_multiple_threshold_factors(signal.signal, signal.length, signal.object_index);
            cfar_output.destructor_lock = false;

            probabilitiesCA->add(cfar_output.table_pointer[0], cfar_output.table_pointer[3], 2048);
            probabilitiesGOCA->add(cfar_output.table_pointer[1], cfar_output.table_pointer[4], 2048);
            probabilitiesSOCA->add(cfar_output.table_pointer[2], cfar_output.table_pointer[5], 2048);
        }

        //std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        
        //std::cout << "thread " << number_of_thread << " finished after " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;

    }
    ~SimulationThread()
    {
        delete probabilitiesCA;
        delete probabilitiesGOCA;
        delete probabilitiesSOCA;
    }
    void add(SimulationThread& finished_simulation)
    {
        probabilitiesCA->add(*(finished_simulation.probabilitiesCA));
        probabilitiesGOCA->add(*(finished_simulation.probabilitiesGOCA));
        probabilitiesSOCA->add(*(finished_simulation.probabilitiesSOCA));
    }
};

int main()
{
    CFAR cfar(CFAR_GUARD_CELLS, CFAR_TRAINING_CELLS, CFAR_MIN_TESTED_VALUE, CFAR_MAX_TESTED_VALUE, CFAR_DALTA_TESTED_VALUE);
    Signal signal(SIGMA, DATA_LENGTH, SNR_dB);
    SimulationThread *simulation[NUMBER_OF_THREADS];// = SimulationThread(cfar, signal);
    //std::thread simulation_thread(&SimulationThread::start_simulation, &simulation, 100000, 0);
    //simulation_thread.join();
    
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
        if (number_of_thread > 0)
            simulation[0]->add(*(simulation[number_of_thread]));
    }
    simulation[0][0].export_to_csv();

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

    std::cout << "Simulation finished after " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() << "[s]" << std::endl;
    std::cin.get();
}