#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <thread>
#include <vector>
#include <algorithm>
#include <queue>


class Signal
{
private:
    float sigma = 1;
    float snr_dB = 12;
    std::vector<float> signal_amplitudes = {(float)pow(10, snr_dB / 20) * sigma};
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

    static bool isIntInVector(int value, std::vector<int> vector){
        return !(std::find(vector.begin(), vector.end(), value) == vector.end()) || vector.back() == value ;
    }

    static int generate_int_in_range(int bot, int top){
        return rand() % (top + 1 - bot) + bot;
    }

    void signal_generation(int signal_count)
    {
        int index;
        for (int i = 0; i < signal_count; ++i) {
            if (object_indexes.empty()){
                index = rand() % length;
            } else{
                while (!
                (index >= 0 && index < length &&
                !(isIntInVector(index,object_indexes)))){
                    int neighbour_index = rand() % object_indexes.size();
                    index = object_indexes[neighbour_index] + generate_int_in_range(-signal_max_distance,
                                                                             signal_max_distance);
                }
            }
            object_indexes.push_back(index);
            samples[index] += signal_amplitudes[i%signal_amplitudes.size()];
        }

    }

    void setQueue(){
        while (!object_indexes_queue.empty()) {
            object_indexes_queue.pop();
        }
        for (const auto& element : object_indexes) {
            object_indexes_queue.push(element);
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
        this->signal_amplitudes = signal_amplitudes;
    }

    void signal_and_noise_generation(int signal_count)
    {
        object_indexes = {};
        samples = {};
        srand(time(nullptr));
        noise_generation();
        signal_generation(signal_count);
        setQueue();
    }

    ~Signal() = default;
};