#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <thread>
#include <utility>
#include <vector>
#include <algorithm>
#include <queue>

class Signal
{
private:
    float sigma = 1;
    float snr_dB = 12;
    int shadow_length = 30;
    std::vector<float> signal_amplitudes = { (float)pow(10, snr_dB / 20) * sigma };
    std::default_random_engine noise_generator = std::default_random_engine((time(nullptr)));


    void noise_generation()
    {
        samples.reserve(length);
        std::normal_distribution<float> dist(0, sigma);
        for (int index = 0; index < length; index++)
        {
            samples.push_back(dist(noise_generator));
        }
    }

    static bool isIntInVector(int value, std::vector<int> vector) {
        return !(std::find(vector.begin(), vector.end(), value) == vector.end()) || vector.back() == value;
    }

#ifdef UNIT_TESTING
    static int generate_int_in_range(int min, int max) {
        static size_t test_count = 0;
        if (test_count == 0) {
            test_count++;
            return min;
        }
        if (test_count == 1) {
            test_count++;
            return max;
        }
        if (test_count >= 2) {
            test_count++;
            return (min + max) / 2;
        }
    }
#else
    static int generate_int_in_range(int bot, int top) {
        return rand() % (top + 1 - bot) + bot;
    }
#endif

    void two_signal_generation(int distance)
    {
        int index = rand() % (length - distance - 1);
        object_indexes.push_back(index);
        add_signal_to_samples(index,signal_amplitudes[0]);
        object_indexes.push_back(index + distance);
        add_signal_to_samples(index + distance,signal_amplitudes[1 % signal_amplitudes.size()]);
    }

    void setQueue() {
        while (!object_indexes_queue.empty()) {
            object_indexes_queue.pop();
        }
        for (const auto& element : object_indexes) {
            object_indexes_queue.push(element);
        }
    }

    [[nodiscard]] std::vector<float> generate_signal_vector(float amplitude) const{
        std::vector<float> shadow = {};
        for (int i = 1; i <= shadow_length; ++i) {
            shadow.push_back((float)pow(sin(0.5*i)/(0.5*i),2) * amplitude);
        }
        std::vector<float> output = {};
        output.insert( output.end(), shadow.rbegin(), shadow.rend() );
        output.push_back(amplitude);
        output.insert( output.end(), shadow.begin(), shadow.end() );
        return output;
    }

    void add_signal_to_samples(int index, float amplitude){
        std::vector<float> signalVector = generate_signal_vector(amplitude);
        for (int i = 0; i < signalVector.size(); ++i) {
            int samples_iteration_index = index + i - shadow_length;
            if (samples_iteration_index > 0 && samples_iteration_index < samples.size()) {
                samples[index + i - shadow_length] += signalVector[i];
            }
        }
    }

public:
    std::queue<int> object_indexes_queue;
    std::vector<int> object_indexes;
    std::vector<float> samples;
    int length = 2048;
    int signal_max_distance = 10;

    Signal() = default;

    Signal(Signal& signal)
    {
        this->length = signal.length;
        this->sigma = signal.sigma;
        this->snr_dB = signal.snr_dB;
        this->signal_max_distance = signal.signal_max_distance;
        this->signal_amplitudes = signal.signal_amplitudes;
    }

    Signal(float sigma, int length, float snr_dB, int signal_max_distance, std::vector<float> signal_amplitudes)
    {
        this->length = length;
        this->sigma = sigma;
        this->snr_dB = snr_dB;
        this->signal_max_distance = signal_max_distance;
        this->signal_amplitudes = std::move(signal_amplitudes);
    }

    void signal_and_noise_generation(int signal_distance)
    {
        object_indexes = {};
        samples = {};
        srand(time(nullptr));
        noise_generation();
        two_signal_generation(signal_distance);
        //two_signal_generation(signal_count);
        setQueue();
    }

    ~Signal() = default;
};