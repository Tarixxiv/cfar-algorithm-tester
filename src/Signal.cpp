#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <queue>
#include <random>
#include <string>
#include <thread>
#include <vector>

class Signal
{
public:
    std::queue<int> object_index;
    int length = 1024;
    float sigma = 1;
    float snr_dB = 12;
    float* samples = nullptr;
    std::default_random_engine generator = std::default_random_engine((time(nullptr)));
    Signal() = default;
    Signal(Signal& signal)
    {
        this->length = signal.length;
        this->sigma = signal.sigma;
        this->snr_dB = signal.snr_dB;
    }
    Signal(float sigma, int length, float snr_dB)
    {
        this->length = length;
        this->sigma = sigma;
        this->snr_dB = snr_dB;
    }
    void cleanup()
    {
        delete[] samples;
        while (!object_index.empty())
            object_index.pop();
    }
    void noise_generation()
    {
        samples = new float[length];
        std::normal_distribution<float> dist(0, sigma);
        for (int index = 0; index < length; index++)
        {
            samples[index] = dist(generator);
        }
    }
    void signal_generation()
    {
        float signal_from_object = (float)pow(10, snr_dB / 20) * sigma;
        object_index.push(rand() % length);
        samples[object_index.front()] += signal_from_object;
    }

    void signal_and_noise_generation()
    {
        srand(time(nullptr));
        cleanup();
        noise_generation();
        signal_generation();
    }

    ~Signal()
    {
        delete[] samples;
    }
};