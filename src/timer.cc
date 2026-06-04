#include "timer.hh"

Timer::Timer(std::string output_filename)
{
    // Create the output file
    std::ofstream ofs;
    this->output_filename = output_filename;
    ofs.open(output_filename, std::ofstream::out | std::ofstream::trunc);
    ofs.close();
}

Timer::~Timer()
{
}

void Timer::start(std::string session_name)
{
    start_timepoint = std::chrono::high_resolution_clock::now();
    this->session_name = session_name;
}

void Timer::stop()
{
    auto end_timepoint = std::chrono::high_resolution_clock::now();

    auto start_time = std::chrono::time_point_cast<std::chrono::microseconds>(start_timepoint).time_since_epoch().count();
    auto end_time = std::chrono::time_point_cast<std::chrono::microseconds>(end_timepoint).time_since_epoch().count();

    auto duration = end_time - start_time;
    double ms = duration * 0.001;
    double s = duration * 0.000001;

    // output data
    std::ofstream output_file;
    output_file.open(output_filename, std::ios::app);
    output_file << session_name << ": " << s << "sec (" << ms << " ms)\n";
    output_file.close();
}