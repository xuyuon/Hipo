#ifndef SAMPLING_HH
#define SAMPLING_HH

#include "Randomize.hh"
#include "G4Types.hh"
#include "G4ios.hh"


class SamplingBase
{
public:
    SamplingBase(){}
    ~SamplingBase(){}

    virtual G4double sample() = 0;
};


class PowerLaw : public SamplingBase
{
/*
The probability density function is given by:
P(x) ~ x^(-alpha) for xmin <= x <= xmax

*/
public:
    PowerLaw(G4double alpha, G4double xmin, G4double xmax);
    ~PowerLaw();

    virtual G4double sample() override;

private:
    G4double alpha;  // power law index
    G4double xmin;   // lower bound
    G4double xmax;   // upper bound
};

#endif