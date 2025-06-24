#include "sampling.hh"


PowerLaw::PowerLaw(G4double alpha, G4double xmin, G4double xmax)
    : alpha(alpha), xmin(xmin), xmax(xmax)
{
    if (xmax <= xmin) {
        G4cerr << "Error in initialing PowerLaw: xmax must be greater than xmin." << G4endl;
    }
    if (alpha == 1) {
        G4cerr << "Error in initialing PowerLaw: use ParetoDist instead of PowerLaw for alpha=1" << G4endl;
    }
    this->alpha = alpha;
    this->xmin = xmin;
    this->xmax = xmax;
}

PowerLaw::~PowerLaw()
{
}

double PowerLaw::sample()
{
    /*
    Samples a value from the power law distribution.
    The probability density function is given by:
    P(x) ~ x^(-alpha) for xmin <= x <= xmax
    */
    double u = G4UniformRand(); // uniform random number in [0, 1)
    double x = pow(((pow(xmax, 1-alpha) - pow(xmin, 1-alpha))*u + pow(xmin, 1-alpha)), 1 / (1 - alpha));
    return x;
}