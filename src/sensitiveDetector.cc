#include "sensitiveDetector.hh"

SensitiveDetectorBase::SensitiveDetectorBase(G4String name) : G4VSensitiveDetector(name)
{
    analysisManager = G4AnalysisManager::Instance(); // singleton instance of analysis manager
}

SensitiveDetectorBase::~SensitiveDetectorBase()
{
}

G4bool SensitiveDetectorBase::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    G4Track *track = aStep->GetTrack();
    G4String pName = track->GetDefinition()->GetParticleName();

    // we are only interested in recording photons
    if (pName != "gamma")
        return false;

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();   // when the particle enters the sensitive detector
    G4StepPoint *postStepPoint = aStep->GetPostStepPoint(); // when the particle exits the sensitive detector

    // Record the energy of the photon at the pre-step point
    G4double energy = preStepPoint->GetKineticEnergy();
    analysisManager->FillNtupleDColumn(0, 0, energy / eV);

    // Record the physical process that generated the photon
    G4String processName = track->GetCreatorProcess()->GetProcessName();
    analysisManager->FillNtupleSColumn(0, 1, processName);

    analysisManager->AddNtupleRow(0);

    return true;
}

SphericalShellSD::SphericalShellSD(G4String name,
                                   G4String particle,
                                   std::string OutputDir) : SensitiveDetectorBase(name)
{
    this->OutputDir = OutputDir;
    this->particle = particle;

    G4int n_threads = G4RunManager::GetRunManager()->GetNumberOfThreads();
    // Create the output file for each thread
    /*
    TODO: Thread safety issue in file creation. The constructor creates
    output files for all threads upfront (lines 46-52), but this can cause
    race conditions if multiple threads instantiate this class simultaneously.
    In multi-threaded Geant4 simulations, the sensitive detector is typically
    constructed once per thread. Creating files for all threads in each
    instance's constructor will lead to files being overwritten. Consider
    creating only the file for the current thread, or move file creation
    to a master-thread-only initialization.
    */
    for (int i = 0; i < n_threads; i++)
    {
        std::ofstream ofs;
        ofs.open(OutputDir + "_" + std::to_string(i) + ".csv", std::ofstream::out | std::ofstream::trunc);
        ofs << "EventID,Energy,Process,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta,Production_mag,Production_phi,Production_theta,Production_nadir_phi,Production_nadir_theta\n"; // CSV header
        ofs.close();
    }
}

SphericalShellSD::~SphericalShellSD()
{
}

G4bool SphericalShellSD::ProcessHits(G4Step *aStep, G4TouchableHistory *ROhist)
{
    // Check if this is the first step in the volume
    // TODO: maybe kill the particle?
    if (aStep->IsFirstStepInVolume() == false)
    {
        return true;
    }

    G4Track *track = aStep->GetTrack();
    G4String pName = track->GetDefinition()->GetParticleName();

    // we are only interested in recording photons
    if (pName != particle)
        return false;

    G4StepPoint *preStepPoint = aStep->GetPreStepPoint();   // when the particle enters the sensitive detector
    G4StepPoint *postStepPoint = aStep->GetPostStepPoint(); // when the particle exits the sensitive detector

    // 1. Record the event ID
    G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();

    // 2. Record the energy of the photon at the pre-step point
    G4double energy = postStepPoint->GetKineticEnergy();

    // 3. Record the physical process that generated the photon
    G4String processName = "primary";
    if (track->GetCreatorProcess() != nullptr)
        processName = track->GetCreatorProcess()->GetProcessName();

    // 4. Record the position of the photon
    G4ThreeVector p_pos = track->GetPosition();

    // 5. Record the momentum direction of the photon
    G4ThreeVector p_mom = track->GetMomentumDirection();
    // Transform to local coordinates
    G4double nadir_angles[2];
    p_mom_to_nadir_angles(&p_pos, &p_mom, nadir_angles);

    // 6. Record the production position of the photon
    G4ThreeVector vertex_pos = track->GetVertexPosition();

    // 7. Record the momentum direction of the photon at production height
    G4ThreeVector vertex_mom = track->GetVertexMomentumDirection();
    // Transform to local coordinates
    G4double vertex_nadir_angles[2];
    p_mom_to_nadir_angles(&vertex_pos, &vertex_mom, vertex_nadir_angles);

    // Write the data to data file
    std::ofstream output_file;
    output_file.open(OutputDir + "_" + std::to_string(G4Threading::G4GetThreadId()) + ".csv", std::ios::app);
    if (output_file.is_open())
    {
        output_file << std::fixed << std::setprecision(10);
        output_file << eventID << ","
                    << energy / eV << ","
                    << processName << ","
                    << p_pos.mag() / km << ","
                    << p_pos.phi() / radian << ","
                    << p_pos.theta() / radian << ","
                    << nadir_angles[1] / radian << ","
                    << nadir_angles[0] / radian << ","
                    << vertex_pos.mag() / km << ","
                    << vertex_pos.phi() / radian << ","
                    << vertex_pos.theta() / radian << ","
                    << vertex_nadir_angles[1] / radian << ","
                    << vertex_nadir_angles[0] / radian << "\n";
        output_file.close();
    }
    else
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }

    return true;
}

void SphericalShellSD::p_mom_to_nadir_angles(const G4ThreeVector *p_pos,
                                             const G4ThreeVector *p_mom,
                                             G4double *nadir_angles)
{
    // nadir_angles[0]: nadir_theta
    // nadir_angles[1]: nadir_phi
    // Transform to local coordinates
    G4double p_pos_phi = p_pos->phi();
    G4double p_pos_theta = p_pos->theta();

    // arrival direction is opposite to momentum direction
    G4double arrival_x = -p_mom->x();
    G4double arrival_y = -p_mom->y();
    G4double arrival_z = -p_mom->z();

    G4double arrival_up = arrival_x * sin(p_pos_theta) * cos(p_pos_phi) +
                           arrival_y * sin(p_pos_theta) * sin(p_pos_phi) +
                           arrival_z * cos(p_pos_theta);
    G4double arrival_south = arrival_x * cos(p_pos_theta) * cos(p_pos_phi) +
                              arrival_y * cos(p_pos_theta) * sin(p_pos_phi) -
                              arrival_z * sin(p_pos_theta);
    G4double arrival_east = -arrival_x * sin(p_pos_phi) +
                            arrival_y * cos(p_pos_phi);

    // nadir angle system is defined such that nadir_phi=0 is particle coming from north pole
    // and nadir_phi=90 is particle coming from east, and nadir_theta=0 is particle coming from center of the Earth
    // and nadir_theta=90 is particle arriving from the horizon
    nadir_angles[0] = acos(-arrival_up);
    nadir_angles[1] = atan2(arrival_east, -arrival_south);
}