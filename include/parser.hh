#ifndef PARSER_HH
#define PARSER_HH

#include "G4Transportation.hh"
#include "G4Types.hh"
#include "G4UIcommand.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <unordered_map>


class Parser {
public:
    Parser();
    ~Parser();

    std::string GetSValue(std::string key);
    G4double GetDValue(std::string key);
    G4double GetDimensionedDValue(std::string key);
    G4int GetIValue(std::string key);
    G4bool GetBValue(std::string key);

    void ReadConfig(const std::string filename);
private:
    std::unordered_map<std::string, std::string> dict;
};



#endif
