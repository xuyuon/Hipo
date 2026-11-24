#ifndef FILEMANAGER_HH
#define FILEMANAGER_HH

#include "G4ios.hh"

#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <filesystem>
namespace fs = std::filesystem;

// #include <experimental/filesystem>
// namespace fs = std::experimental::filesystem;


class OutputManager
{
/*
A utility class to manage output files and directories for the simulation.
*/
public:
    OutputManager();
    ~OutputManager();

    // set values
    void initialize(std::string project_name,
        std::string output_dir,
        std::string detecting_particle_name,
        int nThreads,
        int nSensitiveLayer);


    std::string getSecondariesFilesDir(int sens_layer_id, int thread_id) const;
    std::string getPrimariesFilesDir(int thread_id) const;

    void merge_all_output_files();
    void merge_output_files(std::string data_folder, std::string data_type);
private:
    // Private constructor
    OutputManager(std::string project_name,
        std::string output_dir,
        std::string detecting_particle_name,
        int nThreads,
        int nSensitiveLayer);



    //private functions
    void createSecondariesFile() const;
    void createPrimariesFile() const;

    int nThreads;
    int nSensitiveLayer;

    // detecting particle name
    std::string detecting_particle_name;

    // project name
    std::string project_name;

    // root output directory
    std::string root_dir;

    // output subdirectories
    std::string primaries_dir;
    std::vector<std::string> secondaries_dir;
    std::string backtraced_dir;
    std::string runtime_dir;
    std::string tracks_dir;
    std::string summary_dir;

    // Static pointer to the Singleton instance
    static OutputManager* instancePtr;
};

#endif
