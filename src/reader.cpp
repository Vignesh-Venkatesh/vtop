#include <cctype>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <ctime>
#include <filesystem>
#include <algorithm>

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
    double cpu_percent; // cpu %
    unsigned long memb_kb; // memb (rss in KB)
};

// ─────────────────────────────────────────────
// System related functions
// ─────────────────────────────────────────────

// get system time
std::string getOSTime(){
    std::time_t now = time(NULL);
    std::tm* tm_info = localtime(&now);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%a %b %d %Y | %H:%M:%S" , tm_info);
    return std::string(buffer);
}


// getting OS release name
std::string getOSName(){
    std::ifstream file("/etc/os-release");

    if (!file){
        return "Unknown OS";
    }

    std::string os_release;
    std::string line;
    const std::string key = "PRETTY_NAME=";

    while (std::getline(file, line)){
        if (line.find(key) == 0){
            os_release = line.substr(key.length());

            if (!os_release.empty() && os_release.front() == '"' && os_release.back() == '"'){
                os_release = os_release.substr(1, os_release.length()-2);
            }

            break;
        }
    }

    if (os_release.empty()){
        return "Unknown OS";
    }

    return os_release;
}


// ─────────────────────────────────────────────
// CPU related functions
// ─────────────────────────────────────────────

// getting idle and busy times
std::vector<CPUStat> getIdleAndBusyTime(){
    std::ifstream file("/proc/stat");

    if (!file){
        return {};
    }

    std::string line;
    const std::string key = "cpu";

    std::vector<CPUStat> results;

    while(std::getline(file, line)){
        if (line.find(key) == 0){
            std::istringstream line_iss(line);

            std::string cpu;
            unsigned long long user_time; // user time
            unsigned long long nice_time; // nice time
            unsigned long long system_time; // system time
            unsigned long long idle_time; // idle time
            unsigned long long iowait_time; // I/O wait time
            unsigned long long irq_time; // interrupt servicing time
            unsigned long long softirq_time; // softirqs servicing time
            unsigned long long steal_time; // stolen time

            if (!(line_iss >> cpu >> user_time >> nice_time >> system_time >> idle_time >> iowait_time >> irq_time >> softirq_time >> steal_time)){
                continue;
            }

            unsigned long long busy = user_time + nice_time + system_time + irq_time + softirq_time + steal_time;
            unsigned long long  idle = idle_time + iowait_time;

            double usage = ((double)busy/(busy+idle))*100;

            results.emplace_back(cpu, busy, idle, usage);

        }
    }

    return results;
}

// calculating delta time
std::vector<CPUStat> calculateDeltaTime(const std::vector<CPUStat> &prevResults, const std::vector<CPUStat> &currResults){
    std::vector<CPUStat> deltaTimeResults;

    for (size_t i=0; i<prevResults.size() && i<currResults.size(); ++i){
        std::string cpu = prevResults[i].cpu;
        unsigned long long deltaBusy = currResults[i].busy - prevResults[i].busy;
        unsigned long long deltaIdle = currResults[i].idle - prevResults[i].idle;

        double deltaUsage = ((double)deltaBusy/(deltaBusy+deltaIdle))*100;

        deltaTimeResults.emplace_back(cpu, deltaBusy, deltaIdle, deltaUsage);
    }

    return deltaTimeResults;
}


// displaying CPU stat
void displayCPUStat(const std::vector<CPUStat> &results){
    for (size_t i=0; i<results.size(); i++){
        std::cout << "CPU: " << results[i].cpu << "\t\tBusy: " << results[i].busy << "\t\tIdle: " << results[i].idle << "\n";
    }
}

// ─────────────────────────────────────────────
// Meminfo related functions
// ─────────────────────────────────────────────

MemStat getMemInfo(){
    std::ifstream file("/proc/meminfo");
    MemStat mem = {0, 0, 0, 0, 0, 0, 0.0};

    if (!file){
        return mem;
    }

    std::string line;
    while (std::getline(file, line)){
        std::istringstream iss(line);
        std::string key;
        unsigned long long value;

        if (!(iss >> key >> value)){
            continue;
        }

        if (key == "MemTotal:"){
            mem.total_kb = value;
        } else if (key == "MemFree:") {
            mem.free_kb = value;
        } else if (key == "MemAvailable:"){
            mem.available_kb = value;
        } else if (key == "Buffers:"){
            mem.buffers_kb = value;
        } else if (key == "Cached:"){
            mem.cached_kb = value;
        }

    }

    mem.used_kb = mem.total_kb - mem.available_kb;

    if (mem.total_kb > 0){
        mem.used_percent = (static_cast<double>(mem.used_kb)/mem.total_kb) * 100.0;
    } else {
        mem.used_percent = 0.0;
    }

    return mem;

}

// ─────────────────────────────────────────────
// Proc related functions
// ─────────────────────────────────────────────

std::vector<std::string> procDirectories;

// listing directories in /proc/
void listProcDirectories(){
    procDirectories.clear();

    for (const auto& entry : std::filesystem::directory_iterator("/proc")){

        // skip if not a directory
        if (!entry.is_directory()){
            continue;
        }

        // converting directory name to string
        std::string dirname = entry.path().filename().string();

        if (!dirname.empty() && std::all_of(dirname.begin(), dirname.end(), ::isdigit)){
            procDirectories.push_back(dirname);
        }
    }
}

int listNumberOfProcDirectories(){
    listProcDirectories();
    return procDirectories.size();
}

bool readProcStat(const std::string &pid, ProcStat& ps){
    std::string path = "/proc/" + pid + "/stat";
    std::ifstream file(path);

    if (!file){
        return false;
    }

    std::string line;
    std::getline(file, line);

    // finding parenthesis
    size_t open = line.find('(');
    size_t close = line.rfind(')');

    if (open == std::string::npos || close == std::string::npos){
        return false;
    }

    // everything before '(' is pid
    ps.pid = std::stoi(line.substr(0, open));

    // between parenthesis is the process name
    ps.process_name = line.substr(open+1, close-open-1);

    // remaining fields
    std::istringstream iss(line.substr(close + 2));

    char state;
    iss >> state; // state
    iss >> ps.ppid; // parent process id

    // skipping unwanted fields
    unsigned long long skip;
    iss >> skip >> skip >> skip >> skip >> skip >> skip >> skip >> skip >> skip;

    // utime and stime
    iss >> ps.utime >> ps.stime;

    // skipping unwanted fields again
    iss >> skip >> skip >> skip >> skip;

    // thread count
    iss >> ps.threads;

    // skipping unwanted fields yet again;
    iss >> skip >> skip;

    // vsize and rss
    iss >> ps.vsize >> ps.rss;

    // convering rss pages to KB
    long page_size_kb = sysconf(_SC_PAGE_SIZE)/1024;
    ps.memb_kb = ps.rss + page_size_kb;

    return true;

}

std::string readCmdLine(const std::string &pid){
    std::ifstream file("/proc/" + pid + "/cmdline");
    if (!file){
        return "";
    }

    std::string cmdline;
    std::getline(file, cmdline);
    if (cmdline.empty()){
        return "";
    }

    // replacing arguments separated by null bytes with spaces
    std::replace(cmdline.begin(), cmdline.end(), '\0', ' ');

    return cmdline;
}


std::vector<ProcStat> getProcStats(){
    listProcDirectories();

    std::vector<ProcStat> procs;

    for (const auto& pid: procDirectories){
        ProcStat ps{}; // initializing struct
        if (!readProcStat(pid, ps)){
            continue;
        }
        ps.command_name = readCmdLine(pid);
        procs.push_back(ps);
    }

    std::sort(procs.begin(), procs.end(), [](const ProcStat& a, const ProcStat& b){
        return a.memb_kb > b.memb_kb;
    });

    return procs;
}
