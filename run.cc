#include <iostream>
#include <stdexcept>

#include "runManager.hh"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <config-file>\n";
        std::cerr << "Example: " << argv[0] << " input/cs_local_2008_06_12_1e9eV-1e10eV.config.txt\n";
        return 1;
    }

    const std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help")
    {
        std::cout << "Usage: " << argv[0] << " <config-file>\n";
        std::cout << "Runs Hipo simulation in batch mode using the provided config file.\n";
        return 0;
    }

    try
    {
        G4EarthRunManager manager(arg1);
        manager.run();
        G4cout << "Run Completed" << G4endl;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 2;
    }
}
