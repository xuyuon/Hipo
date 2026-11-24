#include "parser.hh"

Parser::Parser()
{
    // Constructor implementation
}

Parser::~Parser()
{
    // Destructor implementation
}

std::string Parser::GetSValue(std::string key)
{
    return dict[key];
}

G4double Parser::GetDValue(std::string key)
{
    return G4UIcommand::ConvertToDouble(dict[key].c_str());
}

G4double Parser::GetDimensionedDValue(std::string key)
{
    return G4UIcommand::ConvertToDimensionedDouble(dict[key].c_str());
}

G4int Parser::GetIValue(std::string key)
{
    return G4UIcommand::ConvertToInt(dict[key].c_str());
}

G4bool Parser::GetBValue(std::string key)
{
    return G4UIcommand::ConvertToBool(dict[key].c_str());
}

void Parser::ReadConfig(const std::string filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open data file " << filename << G4endl;
        return;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '='))
        {
            std::string value;
            if (std::getline(is_line, value))
                dict[key] = value;
        }
    }

    G4cout << "Config file read successfully." << G4endl;
    file.close();
}
