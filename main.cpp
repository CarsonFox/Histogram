#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include <cstdlib>

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
    if (argc < 6) {
        std::cerr << "Invalid number of arguments\n";
        std::exit(1);
    }

    int num_threads, bin_count, data_count;
    float min_meas, max_meas;

    /*
     * Get user input
     */
    num_threads = std::atoi(argv[1]);
    bin_count = std::atoi(argv[2]);
    min_meas = std::atof(argv[3]);
    max_meas = std::atof(argv[4]);
    data_count = std::atoi(argv[5]);

    /*
     * Set up shared variables
     */
    auto data = generateData(data_count, min_meas, max_meas);
    const auto chunks = chunkData(data, num_threads);
    std::vector<int> global_counts(bin_count, 0);
    std::vector<std::thread> threads;
    std::mutex mutex;

    for (const auto &chunk: chunks) {
        //Spawn threads for each chunk of data
        threads.emplace_back([&](){
            //Local counts to avoid synchronization until the end
            std::vector<int> local_counts(bin_count, 0);

            //Find the bin for each data point in the chunk
            std::for_each(chunk.first, chunk.second, [&](const float x) {
                local_counts[findBin(x, min_meas, max_meas, bin_count)]++;
            });

            //Lock the mutex, then add results to global sum
            std::lock_guard<std::mutex> guard(mutex);

            //Black magic to add two vectors
            std::transform(local_counts.begin(), local_counts.end(),
                global_counts.begin(), global_counts.begin(),
                std::plus<int>{});
        });
    }

    //Wait for every thread to finish
    for (auto &thread: threads) {
        thread.join();
    }

    reportBinMaxes(min_meas, max_meas, bin_count);

    std::cout << "Bin counts: ";
    for (int x: global_counts) {
        std::cout << x << ' ';
    }
    std::cout << '\n';
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
