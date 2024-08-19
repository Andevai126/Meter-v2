#include <cstdlib>	 // For rand() and srand()
#include <ctime>	 // For time()
#include <numeric>   // For iota()
#include <random>    // For default_random_engine()

void randomSetup() {
    // Seed the random number generator
	std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void setShuffledIntArray(int* array, int size) {
    std::iota(array, array + size, 0);
    std::shuffle(array, array + size, std::default_random_engine(std::time(nullptr)));
}

void shuffleIntArray(int* array, int size) {
    std::shuffle(array, array + size, std::default_random_engine(std::time(nullptr)));
}

// Can't use std::shuffle, because it can't handle VLA's
// Can't use std::swap, because it can't handle Strings
void customShuffleStringArray(String* array, int size) {
    for (int i = size - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        String temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

// int randomGen(int size) {
//     return std::rand() % size;
// }