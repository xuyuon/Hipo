#ifndef TIMER_HH
#define TIMER_HH

#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>

class Timer
{
public:
    Timer();
    ~Timer();

    void start(std::string session_name);
    void stop();
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> start_timepoint;
    std::string session_name;
};

#endif