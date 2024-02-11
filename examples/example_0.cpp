#include <CLIParser.hpp>
#include <filesystem>

int main (int argc, char** argv) {
    cli::Opt<std::filesystem::path> inputFile("Input File", "File to be run on", "f", "input", "file");
    cli::Opt<bool> useExperimentalFeatures("Experimental Features", "Enable beta features", "e", "experimental", "beta");
    cli::parse(argc, argv, inputFile, useExperimentalFeatures);

    inputFile = std::filesystem::path("/usr/bin");
}
