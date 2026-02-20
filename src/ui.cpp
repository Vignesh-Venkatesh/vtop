#include <cstddef>
#include <ncurses.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>
#include "../include/reader.hpp"

// ─────────────────────────────────────────────
// Panel — base class for all panels
// ─────────────────────────────────────────────
class Panel{
protected:
    WINDOW *win; // window
    std::string m_title; // title
    int m_title_color_pair; // color pair
    int m_height, m_width; // height and width of the window
    int m_pos_y, m_pos_x; // position of the window

public:
    // constructor
    Panel
    (
        const std::string& title,
        int title_color_pair,
        int height,
        int width,
        int pos_y,
        int pos_x
    )
    :
        win(newwin(height, width, pos_y, pos_x)),
        m_title(title),
        m_title_color_pair(title_color_pair),
        m_height(height),
        m_width(width),
        m_pos_y(pos_y),
        m_pos_x(pos_x)
    {}

    // destructor
    virtual ~Panel() {
        if (win){
            delwin(win);
        }
    }

    // to prevent accidental copying
    Panel(const Panel&) = delete;
    Panel& operator=(const Panel&) = delete;

    // drawing panel
    void drawPanel(){
        box(win,0,0); // drawing outer border

        // title of the panel
        wattron(win, COLOR_PAIR(m_title_color_pair) | A_BOLD);
        mvwprintw(win, 0, 1, " %s ", m_title.c_str());
        wattroff(win, COLOR_PAIR(m_title_color_pair) | A_BOLD);

        wnoutrefresh(win); // marking window for update
    }
};

// ─────────────────────────────────────────────
// MemPanel — displays memory utilisation
// extends Panel class
// ─────────────────────────────────────────────
class MemPanel : public Panel{
private:
    MemStat m_mem_info;
    unsigned long long m_active_memory; // truly used memory (green)
    unsigned long long m_buffer; // buffers (blue)
    unsigned long long m_cached; // cache (yellow)
    unsigned long long m_free; // free (space)
    unsigned long long m_total; // total space

    // function to get memory stats
    void getMemStats(){
        m_mem_info = getMemInfo();
        m_active_memory = m_mem_info.used_kb - m_mem_info.buffers_kb - m_mem_info.cached_kb;
        m_buffer = m_mem_info.buffers_kb;
        m_cached = m_mem_info.cached_kb;
        m_free = m_mem_info.free_kb;
        m_total = m_mem_info.total_kb;
    }

    // function to calculate number of bars
    std::vector<int> calculateBars(int container_width, const unsigned long long& total_memory, const unsigned long long& active_memory, const unsigned long long& buffer, const unsigned long long& cached){

        if (total_memory == 0) {
            return {0, 0, 0, 0};
        }

        int active_memory_bars = static_cast<int>(container_width * (static_cast<double>(active_memory) / total_memory));
        int buffer_memory_bars = static_cast<int>(container_width * (static_cast<double>(buffer) / total_memory));
        int cached_memory_bars = static_cast<int>(container_width * (static_cast<double>(cached) / total_memory));

        return {active_memory_bars, buffer_memory_bars, cached_memory_bars};
    }



    void drawVisuals(){
        // calculating width of the bar container
        int bar_container_width = getmaxx(win) - 6;

        // getting memory stats
        getMemStats();

        // calculating bars
        std::vector<int> barsCount = calculateBars(bar_container_width, m_total, m_active_memory, m_buffer, m_cached);

        // setting bars
        std::string active_bars(barsCount[0], '|');
        std::string buffer_bars(barsCount[1], '|');
        std::string cached_bars(barsCount[2], '|');
        // free 'spaces'
        int used = barsCount[0] + barsCount[1] + barsCount[2];
        int free_bar_count = bar_container_width - used;
        std::string free_bars(std::max(0, free_bar_count), ' ');


        // drawing the bars
        mvwprintw(win, 1, 2, "[");

        wattron(win, COLOR_PAIR(1) | A_BOLD);
        wprintw(win, "%s", active_bars.c_str());
        wattroff(win, COLOR_PAIR(1) | A_BOLD);

        wattron(win, COLOR_PAIR(4) | A_BOLD);
        wprintw(win, "%s", buffer_bars.c_str());
        wattroff(win, COLOR_PAIR(4) | A_BOLD);

        wattron(win, COLOR_PAIR(2) | A_BOLD);
        wprintw(win, "%s", cached_bars.c_str());
        wattroff(win, COLOR_PAIR(2) | A_BOLD);

        wprintw(win, "%s", free_bars.c_str());

        wprintw(win, "]");


        // drawing usage stats
        double used_gb  = static_cast<double>(m_mem_info.used_kb)  / (1024 * 1024);
        double total_gb = static_cast<double>(m_mem_info.total_kb) / (1024 * 1024);
        mvwprintw(win, 3, 2, "Usage: %.2fG/%.1fG", used_gb, total_gb);

        // active memory
        double active_gb = static_cast<double>(m_active_memory) / (1024 * 1024);
        wattron(win, COLOR_PAIR(1) | A_BOLD);
        mvwprintw(win, 5, 2, "Active:\t");
        wattroff(win, COLOR_PAIR(1) | A_BOLD);
        wprintw(win," %.2fG", active_gb);

        // buffer
        double buffer_gb = static_cast<double>(m_buffer) / (1024 * 1024);
        wattron(win, COLOR_PAIR(4) | A_BOLD);
        mvwprintw(win, 6, 2, "Buffer: \t");
        wattroff(win, COLOR_PAIR(4) | A_BOLD);
        wprintw(win," %.2fG", buffer_gb);

        // cached memory
        double cache_gb = static_cast<double>(m_cached) / (1024 * 1024);
        wattron(win, COLOR_PAIR(2) | A_BOLD);
        mvwprintw(win, 7, 2, "Cache:\t");
        wattroff(win, COLOR_PAIR(2) | A_BOLD);
        wprintw(win," %.2fG", cache_gb);

        // free memory
        double free_gb = static_cast<double>(m_free) / (1024 * 1024);
        wattron(win, COLOR_PAIR(8) | A_BOLD);
        mvwprintw(win, 8, 2, "Free: \t");
        wattroff(win, COLOR_PAIR(8) | A_BOLD);
        wprintw(win," %.2fG", free_gb);
    }


public:
    MemPanel(
        int height, // height of the panel
        int width, // width of the panel
        int y, // y coordinate of the panel
        int x // x coordinate of the panel
    )
    :
    Panel(
        "mem", // title
        5, // color pair
        height,
        width,
        y,
        x) {}


    // function to draw memory stats
    void drawMemStats(){
        // drawing the cpu panel first
        drawPanel();

        // drawing visuals
        drawVisuals();

        wnoutrefresh(win); // refreshing window (mem panel contents)

    }
};

// ─────────────────────────────────────────────
// CPUPanel — displays per-core CPU utilisation
// extends Panel class
// ─────────────────────────────────────────────
class CPUPanel : public Panel{
private:
    std::vector<CPUStat> prev_results; // storing previous results
    std::vector<CPUStat> curr_results; // storing current results
    std::vector<CPUStat> delta_results; // storing the results of the difference b/w previous and current


    // function to generate bars
    std::string generateBars(int container_width, double utilization_percent) {
        if (container_width <= 0){
            return "";
        }

        // normalizing to 0..1
        double utilization = utilization_percent / 100.0;

        // containing
        if (utilization < 0.0){
            utilization = 0.0;
        }
        if (utilization > 1.0){
            utilization = 1.0;
        }

        int total_bars = (container_width * utilization);
        int total_space = container_width - total_bars;

        return std::string(total_bars, '|') + std::string(total_space, ' ');
    }

    // function to get color
    int getColor(double utilization){
        if (utilization <= 50){
            return 1;
        } else if (utilization <= 80){
            return 2;
        } else {
            return 3;
        }
    }

    // function to draw visuals on the terminal
    void drawVisual(const CPUStat& result, int row) {
        // CPU name and utilization
        const std::string& cpu_title = result.cpu;
        double utilization = result.cpu_usage_percent;

        // containing utilization to [0, 100] just in case
        if (utilization < 0.0) utilization = 0.0;
        if (utilization > 100.0) utilization = 100.0;

        // calculating width for the bars container
        int window_width = getmaxx(win);

        // left margin + cpu title length + ' [' + '] ' + percentage + right margin
        int fixed_width = 2 + 6 + 2 + 2 + 7 + 2;
        int bars_container_width = window_width - fixed_width;
        if (bars_container_width < 1) bars_container_width = 1;

        // generating visual bar string
        std::string bars = generateBars(bars_container_width, utilization);

        // getting color pair
        int color_pair = getColor(utilization);

        // displaying CPU info
        mvwprintw(win, row, 2, "%-5s [", cpu_title.c_str());
        wattron(win, COLOR_PAIR(color_pair) | A_BOLD);
        wprintw(win, "%s", bars.c_str());
        wattroff(win, COLOR_PAIR(color_pair) | A_BOLD);
        wprintw(win, "] %6.2f%%", utilization); // fixed width percentage
    }

public:
    CPUPanel(
        int height, // height of the panel
        int width, // width of the panel
        int y, // y coordinate of the panel
        int x // x coordinate of the panel
    )
    :
    Panel(
        "cpu", // title
        4, // color pair
        height,
        width,
        y,
        x) {}

    // function to draw CPU stats
    void drawCPUStats(){

        // drawing the cpu panel first
        drawPanel();

        // main cpu stat
        if (!delta_results.empty()){
            drawVisual(delta_results[0],2);

            size_t row = 4;
            for (size_t i=1; i<delta_results.size(); ++i){
                drawVisual(delta_results[i], row++);
            }
        }


        wnoutrefresh(win); // refreshing window (cpu panel contents)
    }

    // function to get cpu stats
    void getCPUStats(){
        prev_results = getIdleAndBusyTime();
        sleep(1);
        curr_results = getIdleAndBusyTime();
        delta_results = calculateDeltaTime(prev_results, curr_results);
    }
};


// ─────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────

// function to quit the program
bool quitVtop(){
    int ch = getch();

    // user pressed 'q' or 'esc'
    if (ch == 'q' || ch == 27){
        return true;
    }

    return false;
}


// initializing colors
void initializeColors(){
    // if user's terminal does not support colors
    if (has_colors() == FALSE){
        endwin();
        std::cerr << "Your terminal does not support colors" << "\n";
        exit(-1);
    }

    // if user's terminal supports colors
    start_color();

    // initializing color pairs
    // (pair number, foreground color, background color)
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, COLOR_WHITE, COLOR_BLACK);
    init_pair(8, COLOR_BLACK + 8, COLOR_BLACK); // bright black (gray)
}

std::vector<int> getTerminalHeightWidth(){
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    return {max_y, max_x};
}


// ─────────────────────────────────────────────
// Main UI loop
// ─────────────────────────────────────────────
void drawUI(){

    int terminal_height = getTerminalHeightWidth()[0];
    int terminal_width = getTerminalHeightWidth()[1];

    // initializing main panel
    Panel mainPanel("vtop", 6, terminal_height, terminal_width, 0, 0);

    // initializing cpu panel
    int cpu_panel_height = static_cast<int>(getIdleAndBusyTime().size()) + 4;
    int cpu_panel_width = terminal_width/2 - 2;
    CPUPanel cpuPanel(cpu_panel_height, cpu_panel_width, 1, 2);

    // initializing mem panel
    int mem_panel_height =static_cast<int>(cpu_panel_height/2);
    int mem_panel_width = cpu_panel_width;
    MemPanel memPanel(mem_panel_height, mem_panel_width, 1, cpu_panel_width + 2);


    while (true){
        mainPanel.drawPanel();
        cpuPanel.getCPUStats();
        cpuPanel.drawCPUStats();
        memPanel.drawMemStats();

        doupdate(); // updating terminal once

        // quit vtop
        if (quitVtop()){
            break;
        }
    }
}

void draw(){
    initscr(); // initializing screen
    curs_set(0); // hiding the cursor
    nodelay(stdscr, TRUE); // non-blocking input
    initializeColors(); // initializing colors
    drawUI();
    endwin(); // closing window
}
