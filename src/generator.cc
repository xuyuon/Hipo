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
    SamplingBase *energySpectrum,
    CoordinatesSamplingBase *pos_dist,
    SamplingBase *theta_dist,
    SamplingBase *phi_dist,
    std::string GenOutputDir)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager

    fParticleGun = new G4ParticleGun(1); // Create a particle gun with 1 particle

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition *particle = particleTable->FindParticle("proton");
    fParticleGun->SetParticleDefinition(particle);

    this->EnergySpectrum = energySpectrum;
    this->pos_dist = pos_dist;
    this->theta_dist = theta_dist;
    this->phi_dist = phi_dist;

    this->GenOutputDir = GenOutputDir;
    this->GenOutputFile = GenOutputDir + "/primaries.csv";

    // Create the output directory
    std::filesystem::create_directories(GenOutputDir);

    // Create the output file
    std::ofstream ofs;
    ofs.open(GenOutputFile, std::ofstream::out | std::ofstream::trunc);
    ofs << "EventID,Energy,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n"; // CSV header
    ofs.close();
}

SphericalSurfaceSampling::~SphericalSurfaceSampling()
{
    delete fParticleGun; // Clean up the particle gun
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
    G4double p_theta = theta_dist->sample();
    G4double p_phi = phi_dist->sample();

    // Transform the local coordinate into global coordinate
    G4double p_pos_theta = p_pos.theta();
    G4double p_pos_phi = p_pos.phi();

    G4double local_x = sin(p_theta) * cos(p_phi);
    G4double local_y = sin(p_theta) * sin(p_phi);
    G4double local_z = -cos(p_theta); // reflected about x-y plane

    G4double p_dir_x = local_x * cos(p_pos_theta) * cos(p_pos_phi) -
                       local_y * sin(p_pos_phi) +
                       local_z * sin(p_pos_theta) * cos(p_pos_phi);
    G4double p_dir_y = local_x * cos(p_pos_theta) * sin(p_pos_phi) +
                       local_y * cos(p_pos_phi) +
                       local_z * sin(p_pos_theta) * sin(p_pos_phi);
    G4double p_dir_z = -local_x * sin(p_pos_theta) +
                       local_z * cos(p_pos_theta);

    G4ThreeVector p_dir(p_dir_x, p_dir_y, p_dir_z);
    fParticleGun->SetParticleMomentumDirection(p_dir);

    // Write the data to data file
    std::ofstream output_file;
    output_file.open(GenOutputFile, std::ios::app);
    if (output_file.is_open())
    {
        output_file << event_id << ","
                    << p_energy / eV << ","
                    << p_pos.mag() / km << ","
                    << p_pos.phi() / radian << ","
                    << p_pos.theta() / radian << ","
                    << p_phi / radian << ","
                    << p_theta / radian << "\n";
        output_file.close();
    }
    else
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }

    // Generate the primary vertex for the event
    fParticleGun->GeneratePrimaryVertex(anEvent); 
}