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

struct ProcStat{
    int pid; // process id
    int ppid; // parent process id
    std::string process_name; // process name
    std::string command_name; // full command
    int threads; // number of threads

    // stats
    unsigned long utime; // user cpu ticks
    unsigned long stime; // system cpu ticks
    unsigned long vsize; // virtual memory (bytes)
    long rss; // resident pages

    // derived
    // double cpu_percent; // cpu %
    unsigned long memb_kb; // memb (rss in KB)
};

std::string getOSTime();
std::string getOSName();
std::vector<CPUStat> getIdleAndBusyTime();
std::vector<CPUStat> calculateDeltaTime(const std::vector<CPUStat> &prevResults, const std::vector<CPUStat> &currResults);
MemStat getMemInfo();
int listNumberOfProcDirectories();
std::vector<ProcStat> getProcStats();

#endif
