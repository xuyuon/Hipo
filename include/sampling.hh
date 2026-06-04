#ifndef SAMPLING_HH
#define SAMPLING_HH

#include "Randomize.hh"
#include "G4Types.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"

#include <vector>

static const G4double PI = 3.14159265358979323846;

class SamplingBase
{
public:
    SamplingBase() {}
    ~SamplingBase() {}

    virtual G4double sample() = 0;
};

class Scaled : public SamplingBase
{
    /*
    Given a distribution f(x), it returns a scaled version of it:
    p(x) ~ f(alpha * x)
    */
public:
    Scaled(SamplingBase *dist, G4double alpha);
    ~Scaled();

    virtual G4double sample() override;

private:
    SamplingBase *dist;
    G4double alpha; // scaling factor
};

class Offset : public SamplingBase
{
    /*
    Given a distribution f(x), it returns a offset version of it:
    p(x) ~ f(x + offset)
    */
public:
    Offset(SamplingBase *dist, G4double offset);
    ~Offset();

    virtual G4double sample() override;

private:
    SamplingBase *dist; // distribution to be shifted
    G4double offset;    // offset value
};

class Uniform : public SamplingBase
{
    /*
    Uniform distribution in the range [xmin, xmax].
    */
public:
    Uniform(G4double xmin, G4double xmax);
    ~Uniform();

    virtual G4double sample() override;

private:
    G4double xmin; // lower bound
    G4double xmax; // upper bound
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
    Sine(G4double xmin = 0., G4double xmax = PI);
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
    Cosine(G4double xmin = -PI / 2., G4double xmax = PI / 2.);
    ~Cosine();

    virtual G4double sample() override;

private:
    G4double xmin;
    G4double xmax;
};

class LogUniform : public SamplingBase
{
    /*
    Uniform distribution in the range [log(xmin), log(xmax)].
    */

public:
    LogUniform(G4double xmin, G4double xmax);
    ~LogUniform();

    virtual G4double sample() override;

private:
    G4double xmin;
    G4double xmax;
};

class Lambertian : public SamplingBase
{
    /*
    The probability density function is given by:
    P(x) ~ sin(2x) for 0 <= x <= pi/2
    */
public:
    Lambertian();
    ~Lambertian();

    virtual G4double sample() override;
};

class UpwardLambertian : public SamplingBase
{
    /*
    The probability density function is given by:
    P(x) ~ sin(2x) for pi/2 <= x <= pi
    */
public:
    UpwardLambertian();
    ~UpwardLambertian();

    virtual G4double sample() override;
};

class CoordinatesSamplingBase
{
public:
    /*
        Base class for sampling coordinates in a 3D space.
        sample() method returns G4ThreeVector.
    */
    CoordinatesSamplingBase() {}
    ~CoordinatesSamplingBase() {}

    virtual G4ThreeVector sample() { return G4ThreeVector(0., 0., 0.); }
};

class ConstCoordinates : public CoordinatesSamplingBase
{
    /*
    Returns a constant 3D vector.
    */
public:
    ConstCoordinates(G4double x, G4double y, G4double z);
    ~ConstCoordinates();

    virtual G4ThreeVector sample() override;

private:
    G4double x;
    G4double y;
    G4double z;
};

class UniformSphericalSurface : public CoordinatesSamplingBase
{
    /*
    Samples points uniformly on the surface of a sphere.
    The radius of the sphere is given by the parameter r.
    */
public:
    UniformSphericalSurface(G4double r,
                            G4double theta_min = 0.,
                            G4double theta_max = PI,
                            G4double phi_min = 0.,
                            G4double phi_max = 2. * PI);
    ~UniformSphericalSurface();

    virtual G4ThreeVector sample() override;

private:
    G4double r;
    G4double theta_min;
    G4double theta_max;
    G4double phi_min;
    G4double phi_max;

    SamplingBase *thetaSampling;
};

#endif