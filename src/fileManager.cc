#include "fileManager.hh"

FileManager::FileManager(std::string project_name,
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

    fs::create_directories(root_dir);

    primaries_dir = root_dir + "/primaries";
    runtime_dir = root_dir + "/runtime";
    tracks_dir = root_dir + "/tracks";
    summary_dir = root_dir + "/summary";
    fs::create_directories(primaries_dir);
    fs::create_directories(runtime_dir);
    fs::create_directories(tracks_dir);
    fs::create_directories(summary_dir);

    for (int i = 1; i <= nSensitiveLayer; i++)
    {
        secondaries_dir.push_back(root_dir + "/" + detecting_particle_name + "_" + std::to_string(i));
        fs::create_directories(root_dir + "/" + detecting_particle_name + "_" + std::to_string(i));
    }
}

FileManager::~FileManager()
{
}

void FileManager::ensureSecondariesFile(int sens_layer_id, int thread_id)
{
    const std::string file_path = getSecondariesFilesDir(sens_layer_id, thread_id);
    std::lock_guard<std::mutex> lock(secondaries_init_mutex);
    if (initialized_secondaries_files.insert(file_path).second)
    {
        std::ofstream ofs(file_path, std::ofstream::out | std::ofstream::trunc);
        if (!ofs.is_open())
        {
            G4cerr << "Unable to open file for creating secondaries output: " << file_path << G4endl;
            return;
        }
        ofs << "EventID,Energy,Process,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta,Production_mag,Production_phi,Production_theta,Production_nadir_phi,Production_nadir_theta\n";
    }
}

std::string FileManager::getSecondariesFilesDir(int sens_layer_id, int thread_id) const
{
    return secondaries_dir[sens_layer_id] + "/" + project_name + "_" + std::to_string(thread_id) + ".csv";
}

void FileManager::merge_output_files(std::string data_folder, std::string data_type)
{
    std::string result_file_name = summary_dir + "/" + project_name + "_" + data_type + ".csv";

    for (const auto &file_for_merge : fs::recursive_directory_iterator(data_folder))
    {
        if (file_for_merge.path().extension() == ".csv")
        {
            std::ifstream src(file_for_merge.path().string(), std::ios::binary);
            if (!src.is_open())
            {
                G4cerr << "Unable to open source file for merge: " << file_for_merge.path().string() << G4endl;
                continue;
            }

            std::ofstream dst(result_file_name, std::ios::binary | std::ios::app);
            if (!dst.is_open())
            {
                G4cerr << "Unable to open destination merge file: " << result_file_name << G4endl;
                continue;
            }

            dst.seekp(0, std::ios_base::end);

            if (dst.tellp() == 0)
            {
                dst << src.rdbuf();
            }
            else
            {
                std::string line;
                std::getline(src, line);
                dst << src.rdbuf();
            }
        }
    }
}

void FileManager::merge_all_output_files()
{
    G4cout << "Merging output files..." << G4endl;

    merge_output_files(primaries_dir, "primaries");
    for (int i = 0; i < nSensitiveLayer; i++)
    {
        merge_output_files(secondaries_dir[i], detecting_particle_name + "_layer_" + std::to_string(i));
    }
}
