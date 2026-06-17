#include "generator.hh"

PrimaryGenerator::PrimaryGenerator(SamplingBase *energySpectrum,
                                   CoordinatesSamplingBase *pos_dist,
                                   CoordinatesSamplingBase *dir_dist)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager

    // Constructor implementation
    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->dir_dist = dir_dist;
}

PrimaryGenerator::~PrimaryGenerator()
{
    delete fParticleGun; // Clean up the particle gun
    delete EnergySpectrum;
    delete pos_dist;
    delete dir_dist;
}

void PrimaryGenerator::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    G4int event_id = anEvent->GetEventID();
    analysisManager->FillNtupleIColumn(1, 0, event_id);

    // Sample the particle energy
    G4double p_energy = EnergySpectrum->sample();
    analysisManager->FillNtupleDColumn(1, 1, p_energy / eV);
    analysisManager->AddNtupleRow(1);
    fParticleGun->SetParticleEnergy(p_energy); // Set the energy of the particle

    // Sample the initial position
    G4ThreeVector p_pos = pos_dist->sample();
    fParticleGun->SetParticlePosition(p_pos);

    // Sample the momentum direction
    G4ThreeVector p_dir = dir_dist->sample();
    fParticleGun->SetParticleMomentumDirection(p_dir);

    fParticleGun->GeneratePrimaryVertex(anEvent); // Generate the primary vertex for the event
}

SphericalSurfaceSampling::SphericalSurfaceSampling(
    G4String particleName,
    SamplingBase *energySpectrum,
    CoordinatesSamplingBase *pos_dist,
    SamplingBase *theta_dist,
    SamplingBase *phi_dist,
    std::string OutputDir)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager

    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->theta_dist = theta_dist;
    this->phi_dist = phi_dist;

    this->OutputDir = OutputDir;

    // Create the output file
    std::ofstream ofs;
    ofs.open(OutputDir + "_" + std::to_string(G4Threading::G4GetThreadId()) + ".csv", std::ofstream::out | std::ofstream::trunc);
    ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n"; // CSV header
    ofs.close();
}

SphericalSurfaceSampling::~SphericalSurfaceSampling()
{
    delete fParticleGun; // Clean up the particle gun
    delete EnergySpectrum;
    delete pos_dist;
    delete theta_dist;
    delete phi_dist;
}

void SphericalSurfaceSampling::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    // Save the event id
    G4int event_id = anEvent->GetEventID();

    // Sample the particle energy
    G4double p_energy = EnergySpectrum->sample();
    fParticleGun->SetParticleEnergy(p_energy); // Set the energy of the particle

    // Sample the initial position
    G4ThreeVector p_pos = pos_dist->sample();
    fParticleGun->SetParticlePosition(p_pos);

    // Sample the momentum direction
    G4double p_mom_theta = theta_dist->sample();
    G4double p_mom_phi = phi_dist->sample();

    // Transform the local coordinate into global coordinate
    G4ThreeVector p_dir = nadir_angles_to_p_dir(
        p_pos.theta(),
        p_pos.phi(),
        p_mom_theta,
        p_mom_phi);
    fParticleGun->SetParticleMomentumDirection(p_dir);

    // Write the data to data file
    std::ofstream output_file;
    output_file.open(OutputDir + "_" + std::to_string(G4Threading::G4GetThreadId()) + ".csv", std::ios::app);
    if (output_file.is_open())
    {
        // set precision
        output_file << std::fixed << std::setprecision(10);

        output_file << event_id << ","
                    << p_energy / eV << ","
                    << p_pos.mag() / km << ","
                    << p_pos.phi() / radian << ","
                    << p_pos.theta() / radian << ","
                    << p_mom_phi / radian << ","
                    << p_mom_theta / radian << "\n";
        output_file.close();
    }
    else
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }

    // Generate the primary vertex for the event
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

    G4ThreeVector p_dir(p_dir_x, p_dir_y, p_dir_z);
    return p_dir;
}

GenerateFromTable::GenerateFromTable(
    G4String particleName,
    std::string data_file,
    std::string output_file)
{
    n_threads = G4RunManager::GetRunManager()->GetNumberOfThreads();
    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle(particleName);
    fParticleGun->SetParticleDefinition(particle);

    this->data_file = data_file;
    this->output_file = output_file;

    init_table();

    // Create the output file
    /*
    TODO: Potential thread safety issue in constructor. The constructor uses G4Threading::G4GetThreadId() to create thread-specific output files,
    but constructors may be called from different contexts in multi-threaded Geant4 applications.
    This could lead to incorrect file creation if the constructor is called before threads are properly initialized.
    Consider creating the output files in the GeneratePrimaries method on first use instead.
    */
    std::ofstream ofs;
    ofs.open(output_file + "_" + std::to_string(G4Threading::G4GetThreadId()) + ".csv", std::ofstream::out | std::ofstream::trunc);
    ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n"; // CSV header
    ofs.close();
}

GenerateFromTable::~GenerateFromTable()
{
    // Destructor implementation
    delete fParticleGun;
}

void GenerateFromTable::init_table()
{
    // Read the data file
    std::ifstream file(data_file);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << data_file << G4endl;
        return;
    }
    std::string line;
    // Read the first line to get the header information
    std::getline(file, line);
    // TODO: check header format

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        std::string numstr;

        // eventid
        std::getline(ss, numstr, ',');

        // energies
        std::getline(ss, numstr, ',');
        energies.push_back(std::stod(numstr) * eV);

        // Position_mag
        std::getline(ss, numstr, ',');
        pos_mag.push_back(std::stod(numstr) * km);

        // Position_phi
        std::getline(ss, numstr, ',');
        pos_phi.push_back(std::stod(numstr) * radian);

        // Position_theta
        std::getline(ss, numstr, ',');
        pos_theta.push_back(std::stod(numstr) * radian);

        // Nadir_phi
        std::getline(ss, numstr, ',');
        nadir_phi.push_back(std::stod(numstr) * radian);

        // Nadir_theta
        std::getline(ss, numstr, ',');
        nadir_theta.push_back(std::stod(numstr) * radian);
    }
    file.close();
}

int GenerateFromTable::get_n_events() const
{
    return energies.size();
}

void GenerateFromTable::GeneratePrimaries(G4Event *anEvent)
{
    G4ParticleDefinition *particle = fParticleGun->GetParticleDefinition(); // Find the particle definition

    // Save the event id
    G4int event_id = anEvent->GetEventID();

    // Set the particle energy
    G4double p_energy = energies[event_id];
    fParticleGun->SetParticleEnergy(p_energy); // Set the energy of the particle

    // Set the initial position
    G4double p_pos_mag = pos_mag[event_id];
    G4double p_pos_phi = pos_phi[event_id];
    G4double p_pos_theta = pos_theta[event_id];
    G4ThreeVector p_pos = G4ThreeVector(
        p_pos_mag * sin(p_pos_theta) * cos(p_pos_phi),
        p_pos_mag * sin(p_pos_theta) * sin(p_pos_phi),
        p_pos_mag * cos(p_pos_theta));
    fParticleGun->SetParticlePosition(p_pos);

    // Set the momentum direction
    G4double p_nadir_theta = nadir_theta[event_id];
    G4double p_nadir_phi = nadir_phi[event_id];

    // Transform the local coordinate into global coordinate
    G4ThreeVector p_dir = nadir_angles_to_p_dir(
        p_pos_theta,
        p_pos_phi,
        p_nadir_theta,
        p_nadir_phi);
    fParticleGun->SetParticleMomentumDirection(p_dir);

    // Write the data to data file
    std::ofstream ofile;
    ofile.open(output_file + "_" + std::to_string(G4Threading::G4GetThreadId()) + ".csv", std::ios::app);
    if (ofile.is_open())
    {
        // set precision
        ofile << std::fixed << std::setprecision(10);

        ofile << event_id << ","
              << p_energy / eV << ","
              << p_pos.mag() / km << ","
              << p_pos.phi() / radian << ","
              << p_pos.theta() / radian << ","
              << p_nadir_phi / radian << ","
              << p_nadir_theta / radian << "\n";
        ofile.close();
    }
    else
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }

    // Generate the primary vertex for the event
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

    G4ThreeVector p_dir(p_dir_x, p_dir_y, p_dir_z);
    return p_dir;
}