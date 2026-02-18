#include <cstddef>
#include <ncurses.h>
#include <unistd.h>
#include "../include/reader.hpp"

// drawing CPU stats
void drawCPUStats(){
    std::vector<CPUStat> prevResults = getIdleAndBusyTime(); // getting previous results
    sleep(1);
    std::vector<CPUStat> currResults = getIdleAndBusyTime(); // getting current results

    std::vector<CPUStat> deltaResults = calculateDeltaTime(prevResults, currResults); // calculating delta

    int row = 3;
    for (size_t i=0; i<prevResults.size() && i<currResults.size(); ++i){
        mvprintw(row++, 1, "%s\t%.2f%%\n", deltaResults[i].cpu.c_str(), deltaResults[i].cpu_usage_percent);
    }
}

// drawing system information
void drawSystemInfo(){
    // drawing OS name
    mvprintw(1, 1, "%s", getOSName().c_str());
}


void drawUI(){
    while(true){
        clear();
        box(stdscr, 0, 0);
        drawSystemInfo();
        drawCPUStats();
        refresh();
    }
}

void draw(){
    initscr();
    curs_set(0); // hiding the cursor
    drawUI();
    endwin();
}
