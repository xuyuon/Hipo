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
    for (int i = 0; i < n_threads; i++)
    {
        std::ofstream ofs;
        ofs.open(OutputDir + "_" + std::to_string(i) + ".csv", std::ofstream::out | std::ofstream::trunc);
        ofs << "EventID,Energy,Process,Position_mag,Position_phi,Position_theta,Nadir_phi,Nadir_theta\n"; // CSV header
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
    G4double p_pos_phi = p_pos.phi();
    G4double p_pos_theta = p_pos.theta();
    // transformation defined such that phi=0 is pointing towards north pole
    // and phi=90 is pointing towards east

    G4double p_mom_up = p_mom.x() * sin(p_pos_theta) * cos(p_pos_phi) +
                        p_mom.y() * sin(p_pos_theta) * sin(p_pos_phi) +
                        p_mom.z() * cos(p_pos_theta);
    G4double p_mom_south = p_mom.x() * cos(p_pos_theta) * cos(p_pos_phi) +
                         p_mom.y() * cos(p_pos_theta) * sin(p_pos_phi) -
                         p_mom.z() * sin(p_pos_theta);
    G4double p_mom_east = - p_mom.x() * sin(p_pos_phi) +
                          p_mom.y() * cos(p_pos_phi);
    G4double p_mom_north = -p_mom_south;
    G4double p_mom_down = -p_mom_up;

    G4double p_mom_theta_nadir = M_PI - acos(p_mom_down);
    G4double p_mom_phi_nadir = atan2(p_mom_east, p_mom_north);

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
                    << p_mom_phi_nadir / radian << ","
                    << p_mom_theta_nadir / radian << "\n";
        output_file.close();
    }
    else
    {
        G4cerr << "Unable to open file for writing output data." << G4endl;
    }

    return true;
}
