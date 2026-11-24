#include "sampling.hh"

Scaled::Scaled(SamplingBase *dist, G4double alpha)
{
    if (alpha <= 0)
    {
        G4cerr << "Error in initialing Scaled: alpha must be greater than 0." << G4endl;
    }
    this->dist = dist;
    this->alpha = alpha;
}

Scaled::~Scaled()
{
    delete dist;
}

G4double Scaled::sample()
{
    G4double x = dist->sample();
    return x / alpha;
}

Offset::Offset(SamplingBase *dist, G4double offset)
{
    this->dist = dist;
    this->offset = offset;
}

Offset::~Offset()
{
    delete dist;
}

G4double Offset::sample()
{
    G4double x = dist->sample();
    return x - offset;
}

Uniform::Uniform(G4double xmin, G4double xmax)
{
    if (xmax <= xmin)
    {
        G4cerr << "Error in initialing Uniform: xmax must be greater than xmin." << G4endl;
    }
    this->xmin = xmin;
    this->xmax = xmax;
}

Uniform::~Uniform()
{
}

G4double Uniform::sample()
{
    /*
    Samples a value from the uniform distribution.
    The probability density function is given by:
    P(x) = 1 / (xmax - xmin) for xmin <= x <= xmax
    */
    G4double u = G4UniformRand();          // uniform random number in [0, 1)
    G4double x = u * (xmax - xmin) + xmin; // scale to [xmin, xmax]
    return x;
}

PowerLaw::PowerLaw(G4double alpha, G4double xmin, G4double xmax)
    : alpha(alpha), xmin(xmin), xmax(xmax)
{
    if (xmax <= xmin)
    {
        G4cerr << "Error in initialing PowerLaw: xmax must be greater than xmin." << G4endl;
    }
    if (alpha == 1)
    {
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
    double x = pow(((pow(xmax, 1 - alpha) - pow(xmin, 1 - alpha)) * u + pow(xmin, 1 - alpha)), 1 / (1 - alpha));
    return x;
}

Sine::Sine(G4double xmin, G4double xmax)
{
    if (xmax <= xmin)
    {
        G4cerr << "Error in initialing Sine: xmax must be greater than xmin." << G4endl;
    }
    if (xmin < 0)
    {
        G4cerr << "Error in initialing Sine: xmin must be greater than or equal to 0." << G4endl;
    }
    if (xmax > PI)
    {
        G4cerr << "Error in initialing Sine: xmax must be less than or equal to pi." << G4endl;
    }

    this->xmin = xmin;
    this->xmax = xmax;
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
    G4double x = acos(cos(xmin) + u * (cos(xmax) - cos(xmin)));
    return x;
}

Cosine::Cosine(G4double xmin, G4double xmax)
{
    if (xmax <= xmin)
    {
        G4cerr << "Error in initialing Cosine: xmax must be greater than xmin." << G4endl;
    }
    if (xmin < -PI / 2)
    {
        G4cerr << "Error in initialing Cosine: xmin must be greater than or equal to -pi/2." << G4endl;
    }
    if (xmax > PI / 2)
    {
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

LogUniform::LogUniform(G4double xmin, G4double xmax)
{
    if (xmax <= xmin)
    {
        G4cerr << "Error in initialing LogUniform: xmax must be greater than xmin." << G4endl;
    }
    if (xmin <= 0)
    {
        G4cerr << "Error in initialing LogUniform: xmin must be greater than 0." << G4endl;
    }
    this->xmin = xmin;
    this->xmax = xmax;
}

LogUniform::~LogUniform()
{
}

G4double LogUniform::sample()
{
    G4double u = G4UniformRand();
    return xmin * pow(xmax / xmin, u);
}

Lambertian::Lambertian()
{
}

Lambertian::~Lambertian()
{
}

G4double Lambertian::sample()
{
    /*
    Samples a value from the Lambertian distribution.
    The probability density function is given by:
    P(x) ~ sin(2x) for 0 <= x <= pi/2
    */
    G4double u = G4UniformRand();
    G4double x = acos(1.0 - 2.0 * u);
    return x / 2.0;
}

UpwardLambertian::UpwardLambertian()
{
}

UpwardLambertian::~UpwardLambertian()
{
}

G4double UpwardLambertian::sample()
{
    /*
    Samples a value from the Upward Lambertian distribution.
    The probability density function is given by:
    P(x) ~ sin(2x) for pi/2 <= x <= pi
    */
    G4double u = G4UniformRand();
    return 0.5 * (PI + acos(2.0 * u - 1.0));
}

Beta::Beta(G4double alpha, G4double beta, G4double x_min, G4double x_max)
{
    if (alpha <= 1 || beta <= 1)
    {
        G4cerr << "Error in initialing Beta: alpha and beta must be greater than 1." << G4endl;
    }
    if (x_max <= x_min)
    {
        G4cerr << "Error in initialing Beta: x_max must be greater than x_min." << G4endl;
    }
    this->alpha = alpha;
    this->beta = beta;
    this->x_min = x_min;
    this->x_max = x_max;
    this->mode = (alpha - 1) / (alpha + beta - 2) * (x_max-x_min) + x_min; // mode of the distribution

    this->max_pdf = UnnormalizedBeta(mode);
}

Beta::~Beta()
{
}

G4double Beta::UnnormalizedBeta(G4double x)
{
    if (x < x_min || x > x_max)
    {
        return 0.0;
    }
    return pow(x - x_min, alpha - 1) * pow(x_max - x, beta - 1);
}

G4double Beta::sample()
{
    /*
    Samples a value from the Beta distribution by rejection sampling.
    The probability density function is given by:
    P(x) ~ (x-x_min)^(alpha-1) * (x_max-x)^(beta-1) for x_min <= x <= x_max
    */
    while (true)
    {
        G4double x = G4UniformRand() * (x_max-x_min)+x_min; // uniform random number in [0, x_max)
        G4double y = G4UniformRand() * max_pdf; // uniform random number in [0, max of the PDF)
        if (y <= UnnormalizedBeta(x))
        {
            return x;
        }
    }
}

ConstCoordinates::ConstCoordinates(G4double x,
                                   G4double y,
                                   G4double z)
{
    this->x = x;
    this->y = y;
    this->z = z;
}

ConstCoordinates::~ConstCoordinates()
{
}

G4ThreeVector ConstCoordinates::sample()
{
    /*
    Returns a constant 3D vector.
    */
    return G4ThreeVector(x, y, z);
}

UniformSphericalSurface::UniformSphericalSurface(G4double r,
                                                 G4double theta_min,
                                                 G4double theta_max,
                                                 G4double phi_min,
                                                 G4double phi_max)
{
    if (theta_max <= theta_min)
    {
        G4cerr << "Error in initialing UniformSphericalSurface: theta_max must be greater than theta_min." << G4endl;
    }
    if (phi_max <= phi_min)
    {
        G4cerr << "Error in initialing UniformSphericalSurface: phi_max must be greater than phi_min." << G4endl;
    }
    if (theta_min < 0 || theta_max > PI)
    {
        G4cerr << "Error in initialing UniformSphericalSurface: theta must be in [0, pi]." << G4endl;
    }
    if (phi_min < 0 || phi_max > 2 * PI)
    {
        G4cerr << "Error in initialing UniformSphericalSurface: phi must be in [0, 2*pi]." << G4endl;
    }
    this->r = r;
    this->theta_min = theta_min;
    this->theta_max = theta_max;
    this->phi_min = phi_min;
    this->phi_max = phi_max;

    thetaSampling = new Sine(theta_min, theta_max);
}

UniformSphericalSurface::~UniformSphericalSurface()
{
    delete thetaSampling;
}

G4ThreeVector UniformSphericalSurface::sample()
{
    G4double theta = thetaSampling->sample();

    // phi follows a uniform distribution
    G4double phi = G4UniformRand() * (phi_max - phi_min) + phi_min;

    // transform to Cartesian coordinates
    G4double x = r * sin(theta) * cos(phi);
    G4double y = r * sin(theta) * sin(phi);
    G4double z = r * cos(theta);

    return G4ThreeVector(x, y, z);
}
