#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>

#include <mpi.h>

/*
 * Use a pair of iterators to represent a subset of the data
 */
using VecIter = std::vector<float>::iterator;
using Range = std::pair<VecIter, VecIter>;

float randomFloat(float min, float max);
std::vector<float> generateData(int count, float min, float max);
std::vector<Range> chunkData(std::vector<float> &data, int n);
size_t findBin(float x, float min, float max, int bins);
void reportBinMaxes(float min, float max, int bin_count);

int main(int argc, char **argv) {
    if (argc < 5) {
        std::cerr << "Usage: ./Histogram bins min max data_count" << std::endl;
        std::exit(1);
    }

    //Get user input
    int bin_count = std::atoi(argv[1]);
    float min_meas = std::atof(argv[2]), max_meas = std::atof(argv[3]);
    int data_count = std::atoi(argv[4]);

    //Set up shared variables
    auto data = generateData(data_count, min_meas, max_meas);
    const auto chunks = chunkData(data, 1);
    std::vector<int> global_counts(bin_count, 0);

    //MPI variables
    int comm_size, rank;
    const auto comm = MPI_COMM_WORLD;

    //Begin distributed section
    MPI_Init(nullptr, nullptr); {
        MPI_Comm_size(comm, &comm_size);
        MPI_Comm_rank(comm, &rank);

        std::cout << "Hello from process " << rank << '!' << std::endl;
        std::cout << "Comm size: " << comm_size << std::endl;

        MPI_Finalize();
    }

    return 0;
}

//Print the max value for each bin
void reportBinMaxes(float min, float max, int bin_count) {
    const auto binSize = (max - min) / static_cast<float>(bin_count);

    std::cout << "Bin maxes: ";
    for (int i = 1; i <= bin_count; i++) {
        std::cout << binSize * i << ' ';
    }
    std::cout << '\n';
}

//Compute the bin for a given data point
size_t findBin(float x, float min, float max, int bins) {
    const auto binSize = (max - min) / static_cast<float>(bins);
    return std::min(static_cast<size_t>((x - min) / binSize), static_cast<size_t>(bins - 1));
}

//Split the data into chunks
std::vector<Range> chunkData(std::vector<float> &data, int n) {
    std::vector<Range> chunks;
    const auto chunk_size = data.size() / n;

    //Chunks are represented by pairs of iterators
    for (int i = 0; i < n - 1; i++) {
        auto chunk_begin = data.begin() + chunk_size * i;
        chunks.emplace_back(std::make_pair(chunk_begin, chunk_begin + chunk_size));
    }

    //The last chunk may not be exactly the same size
    chunks.emplace_back(std::make_pair(data.begin() + (n - 1) * chunk_size, data.end()));

    return chunks;
}

//Get a random float in the range [min, max]
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max - min);
}

//Return a vector of <count> random floats
std::vector<float> generateData(int count, float min, float max) {
    std::vector<float> data(count);
    std::generate_n(data.begin(), count, [=](){ return randomFloat(min, max); });
    return data;
}

//    reportBinMaxes(min_meas, max_meas, bin_count);
//
//    std::cout << "Bin counts: ";
//    for (int x: global_counts) {
//        std::cout << x << ' ';
//    }
//    std::cout << '\n';
