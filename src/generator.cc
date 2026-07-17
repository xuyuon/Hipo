#include "generator.hh"

#include <exception>

PrimaryGenerator::PrimaryGenerator(SamplingBase *energySpectrum,
                                   CoordinatesSamplingBase *pos_dist,
                                   CoordinatesSamplingBase *dir_dist)
{
    analysisManager = G4AnalysisManager::Instance();
    fParticleGun = new G4ParticleGun(1);

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->dir_dist = dir_dist;
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4int event_id = anEvent->GetEventID();
    analysisManager->FillNtupleIColumn(1, 0, event_id);

    G4double p_energy = EnergySpectrum->sample();
    analysisManager->FillNtupleDColumn(1, 1, p_energy / eV);
    analysisManager->AddNtupleRow(1);
    fParticleGun->SetParticleEnergy(p_energy);

    G4ThreeVector p_pos = pos_dist->sample();
    fParticleGun->SetParticlePosition(p_pos);

    G4ThreeVector p_dir = dir_dist->sample();
    fParticleGun->SetParticleMomentumDirection(p_dir);

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

SphericalSurfaceSampling::SphericalSurfaceSampling(
    G4String particleName,
    SamplingBase *energySpectrum,
    CoordinatesSamplingBase *pos_dist,
    SamplingBase *theta_dist,
    SamplingBase *phi_dist,
    std::string OutputDir)
{
    analysisManager = G4AnalysisManager::Instance();
    fParticleGun = new G4ParticleGun(1);

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->theta_dist = theta_dist;
    this->phi_dist = phi_dist;
    this->OutputDir = OutputDir;
}

SphericalSurfaceSampling::~SphericalSurfaceSampling()
{
    delete fParticleGun;
}

void SphericalSurfaceSampling::ensure_output_file(int thread_id) const
{
    std::lock_guard<std::mutex> lock(output_init_mutex);
    if (initialized_output_threads.insert(thread_id).second)
    {
        std::ofstream ofs(OutputDir + "_" + std::to_string(thread_id) + ".csv", std::ofstream::out | std::ofstream::trunc);
        if (ofs.is_open())
        {
            ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n";
        }
    }
}

void SphericalSurfaceSampling::GeneratePrimaries(G4Event *anEvent)
{
    G4int event_id = anEvent->GetEventID();

    G4double p_energy = EnergySpectrum->sample();
    fParticleGun->SetParticleEnergy(p_energy);

    G4ThreeVector p_pos = pos_dist->sample();
    fParticleGun->SetParticlePosition(p_pos);

    G4double p_mom_theta = theta_dist->sample();
    G4double p_mom_phi = phi_dist->sample();

    G4ThreeVector p_dir = nadir_angles_to_p_dir(
        p_pos.theta(),
        p_pos.phi(),
        p_mom_theta,
        p_mom_phi);
    fParticleGun->SetParticleMomentumDirection(p_dir);

    const int thread_id = G4Threading::G4GetThreadId();
    ensure_output_file(thread_id);

    std::ofstream output_file(OutputDir + "_" + std::to_string(thread_id) + ".csv", std::ios::app);
    if (!output_file.is_open())
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }
    else
    {
        output_file << std::fixed << std::setprecision(10);
        output_file << event_id << ","
                    << p_energy / eV << ","
                    << p_pos.mag() / km << ","
                    << p_pos.phi() / radian << ","
                    << p_pos.theta() / radian << ","
                    << p_mom_phi / radian << ","
                    << p_mom_theta / radian << "\n";
    }

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

G4ThreeVector SphericalSurfaceSampling::nadir_angles_to_p_dir(
    G4double pos_theta,
    G4double pos_phi,
    G4double nadir_theta,
    G4double nadir_phi)
{
    G4double p_mom_north = sin(nadir_theta) * cos(nadir_phi);
    G4double p_mom_east = sin(nadir_theta) * sin(nadir_phi);
    G4double p_mom_down = cos(nadir_theta);

    G4double p_mom_up = -p_mom_down;
    G4double p_mom_south = -p_mom_north;

    G4double p_dir_x = sin(pos_theta) * cos(pos_phi) * p_mom_up +
                       cos(pos_theta) * cos(pos_phi) * p_mom_south -
                       sin(pos_phi) * p_mom_east;
    G4double p_dir_y = sin(pos_theta) * sin(pos_phi) * p_mom_up +
                       cos(pos_theta) * sin(pos_phi) * p_mom_south +
                       cos(pos_phi) * p_mom_east;
    G4double p_dir_z = cos(pos_theta) * p_mom_up - sin(pos_theta) * p_mom_south;

    return G4ThreeVector(p_dir_x, p_dir_y, p_dir_z);
}

GenerateFromTable::GenerateFromTable(
    G4String particleName,
    std::string data_file,
    std::string output_file)
{
    fParticleGun = new G4ParticleGun(1);

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
    fParticleGun->SetParticleDefinition(particle);

    this->data_file = data_file;
    this->output_file = output_file;

    init_table();
}

GenerateFromTable::~GenerateFromTable()
{
    delete fParticleGun;
}

void GenerateFromTable::ensure_output_file(int thread_id) const
{
    std::lock_guard<std::mutex> lock(output_init_mutex);
    if (initialized_output_threads.insert(thread_id).second)
    {
        std::ofstream ofs(output_file + "_" + std::to_string(thread_id) + ".csv", std::ofstream::out | std::ofstream::trunc);
        if (ofs.is_open())
        {
            ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n";
        }
    }
}

void GenerateFromTable::init_table()
{
    std::ifstream file(data_file);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << data_file << G4endl;
        return;
    }

    std::string line;
    if (!std::getline(file, line) || line.find("EventID") == std::string::npos)
    {
        G4cerr << "Error: Invalid primaries table header in " << data_file << G4endl;
        return;
    }

    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        std::stringstream ss(line);
        std::string numstr;
        try
        {
            std::getline(ss, numstr, ',');

            std::getline(ss, numstr, ',');
            energies.push_back(std::stod(numstr) * eV);

            std::getline(ss, numstr, ',');
            pos_mag.push_back(std::stod(numstr) * km);

            std::getline(ss, numstr, ',');
            pos_phi.push_back(std::stod(numstr) * radian);

            std::getline(ss, numstr, ',');
            pos_theta.push_back(std::stod(numstr) * radian);

            std::getline(ss, numstr, ',');
            nadir_phi.push_back(std::stod(numstr) * radian);

            std::getline(ss, numstr, ',');
            nadir_theta.push_back(std::stod(numstr) * radian);
        }
        catch (const std::exception &e)
        {
            G4cerr << "Error: Invalid data row in " << data_file << ": " << line << G4endl;
            G4cerr << "Reason: " << e.what() << G4endl;
            energies.clear();
            pos_mag.clear();
            pos_phi.clear();
            pos_theta.clear();
            nadir_phi.clear();
            nadir_theta.clear();
            return;
        }
    }
}

int GenerateFromTable::get_n_events() const
{
    return energies.size();
}

void GenerateFromTable::GeneratePrimaries(G4Event *anEvent)
{
    G4int event_id = anEvent->GetEventID();
    if (event_id < 0 || static_cast<size_t>(event_id) >= energies.size())
    {
        G4cerr << "Error: event index " << event_id << " is out of range for primaries table size " << energies.size() << G4endl;
        return;
    }

    G4double p_energy = energies[event_id];
    fParticleGun->SetParticleEnergy(p_energy);

    G4double p_pos_mag = pos_mag[event_id];
    G4double p_pos_phi = pos_phi[event_id];
    G4double p_pos_theta = pos_theta[event_id];
    G4ThreeVector p_pos = G4ThreeVector(
        p_pos_mag * sin(p_pos_theta) * cos(p_pos_phi),
        p_pos_mag * sin(p_pos_theta) * sin(p_pos_phi),
        p_pos_mag * cos(p_pos_theta));
    fParticleGun->SetParticlePosition(p_pos);

    G4double p_nadir_theta = nadir_theta[event_id];
    G4double p_nadir_phi = nadir_phi[event_id];

    G4ThreeVector p_dir = nadir_angles_to_p_dir(
        p_pos_theta,
        p_pos_phi,
        p_nadir_theta,
        p_nadir_phi);
    fParticleGun->SetParticleMomentumDirection(p_dir);

    const int thread_id = G4Threading::G4GetThreadId();
    ensure_output_file(thread_id);

    std::ofstream ofile(output_file + "_" + std::to_string(thread_id) + ".csv", std::ios::app);
    if (!ofile.is_open())
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }
    else
    {
        ofile << std::fixed << std::setprecision(10);
        ofile << event_id << ","
              << p_energy / eV << ","
              << p_pos.mag() / km << ","
              << p_pos.phi() / radian << ","
              << p_pos.theta() / radian << ","
              << p_nadir_phi / radian << ","
              << p_nadir_theta / radian << "\n";
    }

    fParticleGun->GeneratePrimaryVertex(anEvent);
}

G4ThreeVector GenerateFromTable::nadir_angles_to_p_dir(
    G4double pos_theta,
    G4double pos_phi,
    G4double nadir_theta,
    G4double nadir_phi)
{
    G4double p_mom_north = sin(nadir_theta) * cos(nadir_phi);
    G4double p_mom_east = sin(nadir_theta) * sin(nadir_phi);
    G4double p_mom_down = cos(nadir_theta);

    G4double p_mom_up = -p_mom_down;
    G4double p_mom_south = -p_mom_north;

    G4double p_dir_x = sin(pos_theta) * cos(pos_phi) * p_mom_up +
                       cos(pos_theta) * cos(pos_phi) * p_mom_south -
                       sin(pos_phi) * p_mom_east;
    G4double p_dir_y = sin(pos_theta) * sin(pos_phi) * p_mom_up +
                       cos(pos_theta) * sin(pos_phi) * p_mom_south +
                       cos(pos_phi) * p_mom_east;
    G4double p_dir_z = cos(pos_theta) * p_mom_up - sin(pos_theta) * p_mom_south;

    return G4ThreeVector(p_dir_x, p_dir_y, p_dir_z);
}
