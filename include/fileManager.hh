#ifndef FILEMANAGER_HH
#define FILEMANAGER_HH

#include "G4ios.hh"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace fs = std::filesystem;

class FileManager
{
public:
    FileManager(std::string project_name,
                std::string output_dir,
                std::string detecting_particle_name,
                int nThreads,
                int nSensitiveLayer);
    ~FileManager();

    std::string getSecondariesFilesDir(int sens_layer_id, int thread_id) const;
    void ensureSecondariesFile(int sens_layer_id, int thread_id);

    void merge_all_output_files();
    void merge_output_files(std::string data_folder, std::string data_type);

private:
    int nThreads;
    int nSensitiveLayer;

    std::string detecting_particle_name;
    std::string project_name;
    std::string root_dir;

    std::string primaries_dir;
    std::vector<std::string> secondaries_dir;
    std::string backtraced_dir;
    std::string runtime_dir;
    std::string tracks_dir;
    std::string summary_dir;

    mutable std::mutex secondaries_init_mutex;
    std::unordered_set<std::string> initialized_secondaries_files;
};

#endif
