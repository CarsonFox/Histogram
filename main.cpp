#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>

#include <cstdlib>

#include "Bin.hpp"

/*
 * Use a pair of iterators to represent a subset of the data
 */
using VecIter = std::vector<float>::iterator;
using Range = std::pair<VecIter, VecIter>;

float randomFloat(float min, float max);
std::vector<float> generateData(int count, float min, float max);
std::vector<Range> chunkData(std::vector<float> &data, int n);
std::vector<Bin> getBins(float min, float max, int n);
void report(const std::vector<Bin> &bins);
void count(Range range, std::vector<Bin> &global_bins, std::mutex &mutex, float min, float max);

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
    auto bins = getBins(min_meas, max_meas, bin_count);
    std::mutex mutex;
    std::vector<std::thread> threads;

    /*
     * Divide the work between threads
     */
    for (const auto &chunk: chunks) {
        threads.emplace_back([&]() { count(chunk, bins, mutex, min_meas, max_meas); });
    }

    /*
     * Wait for the results
     */
    for (auto &t: threads) {
        t.join();
    }

    report(bins);
    return 0;
}

void count(const Range range, std::vector<Bin> &global_bins, std::mutex &mutex, const float min, const float max) {
    const auto begin = range.first, end = range.second;

    //Using a thread-local copy of the bins and counts
    auto local_bins = getBins(min, max, global_bins.size());

    //For each data point, find the bin it belongs in, and increment that bin's value
    std::for_each(begin, end, [&](float f){
        auto bin = std::upper_bound(local_bins.begin(), local_bins.end(), f);
        ++(*bin);
    });

    //Once the data has been processed, add the local bin sums to the global bins
    std::lock_guard<std::mutex> lock_guard(mutex);
    std::transform(local_bins.begin(), local_bins.end(), global_bins.begin(), global_bins.begin(), std::plus<Bin>{});
}

void report(const std::vector<Bin> &bins) {
    std::cout << "bin_maxes: ";
    for (const auto &e: bins) {
        std::cout << e.maxValue << " ";
    }
    std::cout << "\n";

    std::cout << "bin_counts: ";
    for (const auto &e: bins) {
        std::cout << e.count << " ";
    }
    std::cout << "\n";
}

//Get a vector of empty bins
std::vector<Bin> getBins(float min, float max, int n) {
    std::vector<Bin> bins;
    const float bin_size = (max - min) / static_cast<float>(n);

    for (int i = 1; i <= n; i++) {
        bins.emplace_back(static_cast<float>(i) * bin_size);
    }

    return bins;
}

//Split the data into chunks, to divide the work between threads
std::vector<Range> chunkData(std::vector<float> &data, int n) {
    std::vector<Range> chunks;
    const auto chunk_size = data.size() / n;

    for (int i = 0; i < n - 1; i++) {
        auto chunk_begin = data.begin() + chunk_size * i;
        chunks.emplace_back(std::make_pair(chunk_begin, chunk_begin + chunk_size));
    }

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