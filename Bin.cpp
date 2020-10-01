#include "Bin.hpp"

Bin &Bin::operator++() {
    this->count++;
    return *this;
}

bool operator<(float a, const Bin &b) {
    return a < b.maxValue;
}

Bin operator+(const Bin &a, const Bin &b) {
    return {a.maxValue, a.count + b.count};
}