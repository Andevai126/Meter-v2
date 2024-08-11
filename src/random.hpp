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

void shuffleCharPointerArray(char** array, int size) {
    std::shuffle(array, array + size, std::default_random_engine(std::time(nullptr)));
}

// int randomGen(int size) {
//     return std::rand() % size;
// }