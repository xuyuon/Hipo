#ifndef SAMPLING_HH
#define SAMPLING_HH

#include "Randomize.hh"
#include "G4Types.hh"
#include "G4ios.hh"

static const PI = 3.14159265358979323846;

class SamplingBase
{
public:
    SamplingBase() {}
    ~SamplingBase() {}

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
    G4double alpha; // power law index
    G4double xmin;  // lower bound
    G4double xmax;  // upper bound
};

class Sine : public SamplingBase
{
    /*
    The probability density function is given by:
    P(x) ~ sin(x) for 0 <= x <= pi

    The lower and upper bounds can be set arbitrarily to values
    within [0, pi].
    */
public:
    Sine(G4double xmin, G4double xmax);
    ~Sine();

    virtual G4double sample() override;

private:
    G4double xmin;
    G4double xmax;
};


class Cosine : public SamplingBase
{
    /*
    The probability density function is given by:
    P(x) ~ cos(x) for -pi/2 <= x <= pi/2

    The lower and upper bounds can be set arbitrarily to values
    within [-pi/2, pi/2].
    */
public:
    Cosine(G4double xmin, G4double xmax);
    ~Cosine();

    virtual G4double sample() override;
    
private:
    G4double xmin;
    G4double xmax;
};  


class UniformSphere : public SamplingBase
{
    /*
    Samples uniformly from the surface of a sphere with radius r.
    The sampling is done in spherical coordinates.
    */
public:
};

#endif