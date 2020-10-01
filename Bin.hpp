#pragma once

struct Bin {
    float maxValue;
    int count = 0;

    explicit Bin(float max) : maxValue(max) {}

    Bin(float max, int count) : maxValue(max), count(count) {}

    Bin &operator++();
};

bool operator<(float a, const Bin &b);

Bin operator+(const Bin &a, const Bin &b);