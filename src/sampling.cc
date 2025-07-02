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


Sine::Sine(G4double xmin, G4double xmax)
{
    if (xmax <= xmin) {
        G4cerr << "Error in initialing Sine: xmax must be greater than xmin." << G4endl;
    }
    if (xmin <  0){
        G4cerr << "Error in initialing Sine: xmin must be greater than or equal to 0." << G4endl;
    }
    if (xmax > PI){
        G4cerr << "Error in initialing Sine: xmax must be less than or equal to pi." << G4endl;
    }

    this_->xmin = xmin;
    this_->xmax = xmax;
}

Sine::~Sine()
{
}

G4double Sine::sample()
{
    /*
    Samples a value from the sine distribution.
    The probability density function is given by:
    P(x) ~ sin(x) for xmin <= x <= xmax
    */
    G4double u = G4UniformRand(); 
    G4double x = acos(cos(xmin)+ u*(cos(xmax)- cos(xmin)))
    return x;
}


Cosine::Cosine(G4double xmin, G4double xmax)
{
    if (xmax <= xmin) {
        G4cerr << "Error in initialing Cosine: xmax must be greater than xmin." << G4endl;
    }
    if (xmin < -PI/2){
        G4cerr << "Error in initialing Cosine: xmin must be greater than or equal to -pi/2." << G4endl;
    }
    if (xmax > PI/2){
        G4cerr << "Error in initialing Cosine: xmax must be less than or equal to pi/2." << G4endl;
    }

    this->xmin = xmin;
    this->xmax = xmax;
}

Cosine::~Cosine()
{
}


G4double Cosine::sample()
{
    /*
    Samples a value from the cosine distribution.
    The probability density function is given by:
    P(x) ~ cos(x) for xmin <= x <= xmax
    */
    G4double u = G4UniformRand(); 
    G4double x = asin(sin(xmin) + u * (sin(xmax) - sin(xmin)));
    return x;
}