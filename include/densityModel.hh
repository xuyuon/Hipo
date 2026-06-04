#ifndef DENSITYMODEL_HH
#define DENSITYMODEL_HH

#include "G4SystemOfUnits.hh"
#include "G4Types.hh"
#include "G4Exp.hh"
#include "G4Transportation.hh"

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

class DensityModelBase
{
    /*
    DensityModelBase is a base class for air density models.
    It provides a list of boundaries along with the densities of
    the atmosphere between those boundaries.
    */
public:
    DensityModelBase() {}
    ~DensityModelBase() {}

    virtual G4double Get_r(G4int idx) const = 0;
    virtual G4double Get_theta(G4int idx) const = 0;
    virtual G4double Get_phi(G4int idx) const = 0;
    virtual int Get_r_boundaries_size() const = 0;
    virtual int Get_theta_boundaries_size() const = 0;
    virtual int Get_phi_boundaries_size() const = 0;
    virtual G4double GetDensities(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
    virtual G4double GetHPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
    virtual G4double GetHePercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
    virtual G4double GetNPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
    virtual G4double GetOPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
    virtual G4double GetArPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const = 0;
};

class TabulatedDensity : public DensityModelBase
{
public:
    TabulatedDensity(G4double r_min, G4double r_pause, G4double r_max,
                     G4double delta_r1, G4double delta_r2,
                     G4double delta_theta, G4double delta_phi, G4String data_file);
    ~TabulatedDensity();

    virtual G4double Get_r(G4int idx) const { return r_boundaries[idx]; }
    virtual G4double Get_theta(G4int idx) const { return theta_boundaries[idx]; }
    virtual G4double Get_phi(G4int idx) const { return phi_boundaries[idx]; }
    virtual int Get_r_boundaries_size() const { return r_boundaries.size(); }
    virtual int Get_theta_boundaries_size() const { return theta_boundaries.size(); }
    virtual int Get_phi_boundaries_size() const { return phi_boundaries.size(); }
    virtual G4double GetDensities(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return densities[r_idx][theta_idx][phi_idx];
    }
    virtual G4double GetHPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return h_percentage_mass[r_idx][theta_idx][phi_idx];
    }
    virtual G4double GetHePercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return he_percentage_mass[r_idx][theta_idx][phi_idx];
    }
    virtual G4double GetNPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return n_percentage_mass[r_idx][theta_idx][phi_idx];
    }
    virtual G4double GetOPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return o_percentage_mass[r_idx][theta_idx][phi_idx];
    }
    virtual G4double GetArPercentageMass(G4int r_idx, G4int theta_idx, G4int phi_idx) const
    {
        return ar_percentage_mass[r_idx][theta_idx][phi_idx];
    }

private:
    // The atmosphere is divided into two regions with different altitude steps:
    // from h_min to h_pause with delta_h1, and from h_pause to h_max with delta_h2
    // h_pause should be defined such that most of the particles is produced below it.
    G4double r_min, r_pause, r_max;
    G4double delta_r1, delta_r2;
    G4double delta_theta, delta_phi;
    G4double R_earth;

    // file directory
    std::string data_file;

    std::vector<G4double> r_boundaries;
    std::vector<G4double> theta_boundaries;
    std::vector<G4double> phi_boundaries;
    std::vector<std::vector<std::vector<G4double>>> densities; // [r][theta][phi]
    std::vector<std::vector<std::vector<G4double>>> h_percentage_mass;
    std::vector<std::vector<std::vector<G4double>>> he_percentage_mass;
    std::vector<std::vector<std::vector<G4double>>> n_percentage_mass;
    std::vector<std::vector<std::vector<G4double>>> o_percentage_mass;
    std::vector<std::vector<std::vector<G4double>>> ar_percentage_mass;

    // Helper functions
    void InitBoundaries();
    void LoadData();
};

#endif