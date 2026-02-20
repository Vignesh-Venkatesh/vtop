#ifndef READER_H
#define READER_H

#include <string>
#include <vector>

struct CPUStat{
    std::string cpu;
    unsigned long long busy;
    unsigned long long idle;
    double cpu_usage_percent;

    CPUStat(const std::string& c, unsigned long long b, unsigned long long i, double u) : cpu(c), busy(b), idle(i), cpu_usage_percent(u){}
};

std::string getOSTime();
std::string getOSName();
std::vector<CPUStat> getIdleAndBusyTime();
std::vector<CPUStat> calculateDeltaTime(const std::vector<CPUStat> &prevResults, const std::vector<CPUStat> &currResults);

#endif
