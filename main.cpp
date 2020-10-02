#include <iostream>
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>

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
void count(Range r, std::vector<int> &global_bins, std::mutex &mutex, float min, float max, int bin_count);

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
    std::vector<int> global_bins(bin_count, 0);
    std::mutex mutex;
    std::vector<std::thread> threads;

    for (const auto &chunk: chunks) {
        threads.emplace_back([&](){count(chunk, global_bins, mutex, min_meas, max_meas, bin_count);});
    }

    for (auto &thread: threads) {
        thread.join();
    }

    std::cout << "Bin counts: ";
    for (const auto x: global_bins) {
        std::cout << x << ' ';
    }
    std::cout << std::endl;
}

void count(Range r, std::vector<int> &global_bins, std::mutex &mutex, float min, float max, int bin_count) {
    std::vector<int> local_bins(bin_count, 0);
    std::for_each(r.first, r.second, [&](float x){
        local_bins[findBin(x, min, max, bin_count)]++;
    });

    std::lock_guard<std::mutex> guard(mutex);
    std::transform(local_bins.begin(), local_bins.end(), global_bins.begin(), global_bins.begin(), std::plus<float>{});
}

size_t findBin(float x, float min, float max, int bins) {
    const auto binSize = (max - min) / static_cast<float>(bins);
    return std::min(static_cast<size_t>((x - min) / binSize), static_cast<size_t>(bins - 1));
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
