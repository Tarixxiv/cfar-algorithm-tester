#include <gtest/gtest.h>

#define UNIT_TESTING
#include "Signal.cpp"
//#include "CFAR.cpp"
#undef UNIT_TESTING

#define SIGMA 1.0f
#define SNR_dB 12.0f
#define DATA_LENGTH 2048
#define TESTS_PER_THREAD 10
#define NUMBER_OF_THREADS 1
#define SIGNAL_COUNT 2
#define CFAR_GUARD_CELLS 8


TEST(SignalTest, SignalDistance) {
	Signal signal(SIGMA, DATA_LENGTH, SNR_dB, CFAR_GUARD_CELLS, std::vector<float>{1,2});
	int test_count = 3;
	signal.signal_and_noise_generation(SIGNAL_COUNT);

	for (int i = 0; i < SIGNAL_COUNT; i++) {
        for (int i = 0; i < SIGNAL_COUNT; i++) {
            for (int j = 0; j < SIGNAL_COUNT; j++) {
                if (i != j) {
					EXPECT_NE(signal.object_indexes[i], signal.object_indexes[j]);
					EXPECT_LE(abs(signal.object_indexes[i] - signal.object_indexes[j]), signal.signal_max_distance);
                }
            }
        }
	}
}
//
//TEST(CfarTest, Complete) {
//	main();
//	findobjects (), srand(451)
//}
