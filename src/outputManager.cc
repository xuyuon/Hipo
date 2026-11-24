#include "outputManager.hh"

OutputManager::OutputManager()
{
    // private constructor
}

void OutputManager::initialize(std::string project_name,
                         std::string output_dir,
                         std::string detecting_particle_name,
                         int nThreads,
                         int nSensitiveLayer)
{
    this->root_dir = output_dir;
    this->project_name = project_name;
    this->detecting_particle_name = detecting_particle_name;
    this->nThreads = nThreads;
    this->nSensitiveLayer = nSensitiveLayer;

    // create root output directories
    fs::create_directories(root_dir);

    // create subdirectories for different types of output
    primaries_dir = root_dir + "/raw/primaries";
    runtime_dir = root_dir + "/runtime";
    tracks_dir = root_dir + "/tracks";
    summary_dir = root_dir + "/summary";

    fs::create_directories(primaries_dir);
    fs::create_directories(runtime_dir);
    // TODO: tracking of particles
    fs::create_directories(tracks_dir);
    fs::create_directories(summary_dir);

    // create subdirectories for secondaries
    for (int i = 1; i <= nSensitiveLayer; i++)
    {
        secondaries_dir.push_back(root_dir + "/raw/" + detecting_particle_name + "_" + std::to_string(i));
        fs::create_directories(root_dir + "/raw/" + detecting_particle_name + "_" + std::to_string(i));
    }

    // create output files
    createPrimariesFile();
    createSecondariesFile();
}

OutputManager::~OutputManager()
{
}

void OutputManager::createSecondariesFile() const
{
    for (int j = 0; j < nSensitiveLayer; j++)
    {
        for (int i = 0; i < nThreads; i++)
        {
            std::ofstream ofs;
            ofs.open(secondaries_dir[j] + "/" + project_name + "_" + std::to_string(i) + ".csv", std::ofstream::out | std::ofstream::trunc);
            ofs << "EventID,Energy,Process,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta,Production_mag,Production_phi,Production_theta,Production_nadir_phi,Production_nadir_theta\n"; // CSV header
            ofs.close();
        }
    }
}

void OutputManager::createPrimariesFile() const
{
    for (int i = 0; i < nThreads; i++)
    {
        std::ofstream ofs;
        ofs.open(primaries_dir + "/" + project_name + "_" + std::to_string(i) + ".csv", std::ofstream::out | std::ofstream::trunc);
        ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n"; // CSV header
        ofs.close();
    }
}

std::string OutputManager::getSecondariesFilesDir(int sens_layer_id, int thread_id) const
{
    return secondaries_dir[sens_layer_id] + "/" + project_name + "_" + std::to_string(thread_id) + ".csv";
}

std::string OutputManager::getPrimariesFilesDir(int thread_id) const
{
    return primaries_dir + "/" + project_name + "_" + std::to_string(thread_id) + ".csv";
}

void OutputManager::merge_output_files(std::string data_folder, std::string data_type)
{
    // set up the merged output file
    std::string result_file_name = summary_dir + "/" + project_name + "_" + data_type + ".csv";
    std::ofstream merged_file(result_file_name, std::ofstream::out | std::ofstream::trunc);

    for (const auto &file_for_merge : fs::recursive_directory_iterator(data_folder))
    {
        if (file_for_merge.path().extension() == ".csv")
        {
            std::ifstream src(file_for_merge.path().string(), std::ios::binary);
            std::ofstream dst(result_file_name, std::ios::binary | std::ios::app);
            dst.seekp(0, std::ios_base::end);

            // copy header for the first file
            if (dst.tellp() == 0)
            {
                dst << src.rdbuf();
            }
            else
            {
                // skip header for other files
                std::string line;
                std::getline(src, line); // skip header
                dst << src.rdbuf();
            }
            src.close();
            dst.close();

            // std::filesystem::remove(file_to_merge); // remove the individual file after merging
        }
    }
}

void OutputManager::merge_all_output_files()
{
    G4cout << "Merging output files..." << G4endl;

    // merge primaries files
    merge_output_files(primaries_dir, "primaries");
    // merge secondaries files
    for (int i = 0; i < nSensitiveLayer; i++)
    {
        merge_output_files(secondaries_dir[i], detecting_particle_name + "_layer_" + std::to_string(i));
    }
}
