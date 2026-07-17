#ifndef PARSER_HH
#define PARSER_HH

#include "G4Transportation.hh"
#include "G4Types.hh"
#include "G4UIcommand.hh"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

class Parser
{
public:
    Parser();
    ~Parser();

    std::string GetSValue(const std::string &key) const;
    G4double GetDValue(const std::string &key) const;
    G4double GetDimensionedDValue(const std::string &key) const;
    G4int GetIValue(const std::string &key) const;
    G4bool GetBValue(const std::string &key) const;

    bool HasKey(const std::string &key) const;
    std::vector<std::string> Keys() const;

    bool ReadConfig(const std::string &filename);

private:
    std::unordered_map<std::string, std::string> dict;

    static std::string trim(const std::string &value);
    const std::string &require_key(const std::string &key) const;
};

#endif
