#include <gtest/gtest.h>

#define UNIT_TESTING
#include "Signal.cpp"
#undef UNIT_TESTING

#define SIGMA 1.0f
#define SNR_dB 12.0f
#define DATA_LENGTH 2048
#define TESTS_PER_THREAD 1000
#define NUMBER_OF_THREADS 5
#define SIGNAL_COUNT 2
#define CFAR_GUARD_CELLS 8

// Demonstrate some basic assertions.
//TEST(HelloTest, BasicAssertions) {
//	// Expect two strings not to be equal.
//	EXPECT_STRNE("hello", "world");
//	// Expect equality.
//	EXPECT_EQ(7 * 6, 42);
//}

TEST(CompleteTest, DetectingObjects) {
	Signal signal(SIGMA, DATA_LENGTH, SNR_dB, CFAR_GUARD_CELLS, std::vector<float>{1,2});
	
	//signal.signal_and_noise_generation(SIGNAL_COUNT);
	EXPECT_EQ(signal.generate_int_in_range(0, 6), 0);
	EXPECT_EQ(signal.generate_int_in_range(0, 6), 6);
	EXPECT_EQ(signal.generate_int_in_range(0, 6), 3);

}