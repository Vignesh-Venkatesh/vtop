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

struct MemStat{
    unsigned long long total_kb;
    unsigned long long free_kb;
    unsigned long long available_kb;
    unsigned long long buffers_kb;
    unsigned long long cached_kb;
    unsigned long long used_kb;

    double used_percent;
};

std::string getOSTime();
std::string getOSName();
std::vector<CPUStat> getIdleAndBusyTime();
std::vector<CPUStat> calculateDeltaTime(const std::vector<CPUStat> &prevResults, const std::vector<CPUStat> &currResults);
MemStat getMemInfo();

#endif
