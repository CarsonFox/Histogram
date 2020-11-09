#include <iostream>
#include <vector>
#include <algorithm>

#include <cstdlib>
#include <cassert>

#include <mpi.h>

float randomFloat(float min, float max);
std::vector<float> generateData(int count, float min, float max);
std::vector<int> countBins(std::vector<float> &data, float min, float max, int bin_count);
size_t findBin(float x, float min, float max, int bins);
void reportBinMaxes(float min, float max, int bin_count);
void reportBinCounts(const std::vector<int> &bins);

int main(int argc, char **argv) {
    if (argc < 5) {
        std::cerr << "Usage: ./Histogram bins min max data_count" << std::endl;
        std::exit(1);
    }

    //Get user input
    int bin_count = std::atoi(argv[1]);
    float min_meas = std::atof(argv[2]), max_meas = std::atof(argv[3]);
    int data_count = std::atoi(argv[4]);

    //MPI variables
    int comm_size, rank;
    const auto comm = MPI_COMM_WORLD;

    //Begin distributed section
    MPI_Init(nullptr, nullptr); {
        MPI_Comm_size(comm, &comm_size);
        MPI_Comm_rank(comm, &rank);
        double start = MPI_Wtime();

        //Some math to support data sizes that don't divide evenly
        const int remainder = (data_count % comm_size) ? comm_size - (data_count % comm_size) : 0;
        const int dataChunkSize = data_count / comm_size + !!remainder;

        //Process 0 initializes the data
        std::vector<float> generatedData;
        if (rank == 0) {
            generatedData = generateData(data_count, min_meas, max_meas);

            //Make sure the data is evenly divisible by number of processes
            for (int i = 0; i < remainder; i++) {
                generatedData.push_back(0);
            }
            assert(generatedData.size() == dataChunkSize * comm_size);
        }

        //Local buffer to receive into
        std::vector<float> localData(dataChunkSize);

        //Distribute generated data
        MPI_Scatter(generatedData.data(), dataChunkSize, MPI_FLOAT, localData.data(), dataChunkSize, MPI_FLOAT, 0, comm);

        //Special case: remove dummy data
        if (rank + 1 == comm_size) {
            for (int i = 0; i < remainder; i++) {
                localData.pop_back();
            }
        }

        //Count the bins for our local data
        auto bins = countBins(localData, min_meas, max_meas, bin_count);

        //Sum the results, and have process 0 print them
        if (rank > 0) {
            MPI_Reduce(bins.data(), nullptr, bin_count, MPI_INT, MPI_SUM, 0, comm);
        } else {
            std::vector<int> globalBins(bin_count, 0);
            MPI_Reduce(bins.data(), globalBins.data(), bin_count, MPI_INT, MPI_SUM, 0, comm);

//            reportBinMaxes(min_meas, max_meas, bin_count);
//            reportBinCounts(globalBins);

            std::cout << comm_size << ',' << data_count << ',' << MPI_Wtime() - start << ',' << std::endl;
        }

        MPI_Finalize();
    }

    return 0;
}

std::vector<int> countBins(std::vector<float> &data, float min, float max, int bin_count) {
    std::vector<int> bins(bin_count, 0);
    std::for_each(data.begin(), data.end(), [&](float x) {
        bins[findBin(x, min, max, bin_count)]++;
    });
    return bins;
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

void reportBinCounts(const std::vector<int> &bins) {
    std::cout << "Bin counts: ";
    for (int x : bins) {
        std::cout << x << ' ';
    }
    std::cout << std::endl;

}

//Compute the bin for a given data point
size_t findBin(float x, float min, float max, int bins) {
    const auto binSize = (max - min) / static_cast<float>(bins);
    return std::min(static_cast<size_t>((x - min) / binSize), static_cast<size_t>(bins - 1));
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
