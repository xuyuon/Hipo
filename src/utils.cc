#include "utils.hh"

G4double trilinear_interp(G4double x_d, G4double y_d, G4double z_d,
                          G4double c000, G4double c100,
                          G4double c010, G4double c110,
                          G4double c001, G4double c101,
                          G4double c011, G4double c111)
{
    return c000 * (1 - x_d) * (1 - y_d) * (1 - z_d) +
           c100 * x_d * (1 - y_d) * (1 - z_d) +
           c010 * (1 - x_d) * y_d * (1 - z_d) +
           c110 * x_d * y_d * (1 - z_d) +
           c001 * (1 - x_d) * (1 - y_d) * z_d +
           c101 * x_d * (1 - y_d) * z_d +
           c011 * (1 - x_d) * y_d * z_d +
           c111 * x_d * y_d * z_d;
}

void associated_legendre(G4int n_max, G4int m_max, G4double theta,
                         std::vector<std::vector<double>> &P,
                         std::vector<std::vector<double>> &dP)
{
    G4double sinth = sin(theta);
    G4double costh = cos(theta);

    std::vector<std::vector<double>> S(n_max + 1, std::vector<double>(m_max + 1, 0));

    P[0][0] = 1.0;
    S[0][0] = 1.0;

    for (int n = 1; n <= n_max; n++)
    {
        for (int m = 0; m <= std::min(m_max, n); m++)
        {
            if (n == m)
            {
                P[n][m] = sinth * P[n - 1][m - 1];
                dP[n][m] = sinth * dP[n - 1][m - 1] + costh * P[n - 1][m - 1];
            }
            else if (n == 1)
            {
                P[n][m] = costh * P[n - 1][m];
                dP[n][m] = costh * dP[n - 1][m] - sinth * P[n - 1][m];
            }
            else if (n > 1)
            {
                double Knm = (((double)n - 1.) * (n - 1.) - m * m) / ((2. * n - 1.) * (2. * n - 3.));
                P[n][m] = costh * P[n - 1][m] - Knm * P[n - 2][m];
                dP[n][m] = costh * dP[n - 1][m] - sinth * P[n - 1][m] - Knm * dP[n - 2][m];
            }
        }
    }

    for (int n = 1; n <= n_max; n++)
    {
        for (int m = 0; m <= std::min(m_max, n); m++)
        {
            // Compute Schmidt normalization
            if (m == 0)
            {
                S[n][0] = S[n - 1][0] * (2. * n - 1.) / n;
            }
            else
            {
                S[n][m] = S[n][m - 1] * sqrt((n - m + 1.) * (m == 1 ? 2.0 : 1.0) / (n + m));
            }

            // apply Schmidt normalization
            P[n][m] = P[n][m] * S[n][m];
            dP[n][m] = dP[n][m] * S[n][m];
        }
    }
}

bool is_leap_year(G4int year)
{
    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;
    if (year % 4 == 0)
        return true;
    return false;
}

double datetime_to_utc(int year, int month, double day)
{
    static const int epoch_year = 1970;
    static const int epoch_month = 1;
    static const double epoch_day = 1;

    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (year < epoch_year)
    {
        G4int days = 0;
        // Count days until next year
        for (int m = month; m <= 12; m++)
        {
            days += days_in_month[m-1];
            if (m == 2 && is_leap_year(year))
            {
                days += 1;
            }
        }
        days -= (day-1); // Subtract days already passed in the current month
        // Count full years in between
        for (G4int y = year + 1; y < epoch_year; y++)
        {
            days += is_leap_year(y) ? 366 : 365;
        }
        return - days * 24.0 * 3600.0; // Convert days to seconds
    }
    else{
        G4int days = 0;
        for (G4int y = epoch_year; y < year; y++)
        {
            days += is_leap_year(y) ? 366 : 365;
        }
        for (G4int m = epoch_month; m < month; m++)
        {
            days += days_in_month[m - 1];
            if (m == 2 && is_leap_year(year))
            {
                days += 1;
            }
        }
        days += day - epoch_day;
        return days * 24.0 * 3600.0; // Convert days to seconds
    }
}

G4double datetime_to_yearfrac(G4int year, G4int month, G4int day)
{
    G4int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    G4int days_in_year = 365;

    if (is_leap_year(year))
    {
        days_in_month[1] = 29;
        days_in_year = 366;
    }

    G4int n_days = day;
    for (int m = 0; m < month - 1; m++)
    {
        n_days += days_in_month[m];
    }

    return year + static_cast<G4double>(n_days - 1) / days_in_year;
}
