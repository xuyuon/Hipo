#include "parser.hh"

#include <algorithm>
#include <stdexcept>

Parser::Parser()
{
}

Parser::~Parser()
{
}

std::string Parser::trim(const std::string &value)
{
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
    {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

const std::string &Parser::require_key(const std::string &key) const
{
    auto it = dict.find(key);
    if (it == dict.end())
    {
        throw std::runtime_error("Missing required config key: " + key);
    }
    return it->second;
}

std::string Parser::GetSValue(const std::string &key) const
{
    return require_key(key);
}

G4double Parser::GetDValue(const std::string &key) const
{
    return G4UIcommand::ConvertToDouble(require_key(key).c_str());
}

G4double Parser::GetDimensionedDValue(const std::string &key) const
{
    return G4UIcommand::ConvertToDimensionedDouble(require_key(key).c_str());
}

G4int Parser::GetIValue(const std::string &key) const
{
    return G4UIcommand::ConvertToInt(require_key(key).c_str());
}

G4bool Parser::GetBValue(const std::string &key) const
{
    return G4UIcommand::ConvertToBool(require_key(key).c_str());
}

bool Parser::HasKey(const std::string &key) const
{
    return dict.find(key) != dict.end();
}

std::vector<std::string> Parser::Keys() const
{
    std::vector<std::string> keys;
    keys.reserve(dict.size());
    for (const auto &kv : dict)
    {
        keys.push_back(kv.first);
    }
    return keys;
}

bool Parser::ReadConfig(const std::string &filename)
{
    dict.clear();

    std::ifstream file(filename);
    if (!file.is_open())
    {
        G4cerr << "Error: Could not open config file " << filename << G4endl;
        return false;
    }

    std::string line;
    G4int line_number = 0;
    while (std::getline(file, line))
    {
        line_number++;

        const auto comment_pos = line.find('#');
        if (comment_pos != std::string::npos)
        {
            line = line.substr(0, comment_pos);
        }

        line = trim(line);
        if (line.empty())
        {
            continue;
        }

        const auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos)
        {
            G4cerr << "Error: Invalid config line " << line_number << " (expected key=value): " << line << G4endl;
            return false;
        }

        std::string key = trim(line.substr(0, eq_pos));
        std::string value = trim(line.substr(eq_pos + 1));

        if (key.empty() || value.empty())
        {
            G4cerr << "Error: Invalid config line " << line_number << " (empty key or value): " << line << G4endl;
            return false;
        }

        dict[key] = value;
    }

    G4cout << "Config file read successfully." << G4endl;
    return true;
}
