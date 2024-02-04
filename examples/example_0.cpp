#include <CLIParser.hpp>
#include <filesystem>

int main (int argc, char** argv) {
    cli::Opt<std::filesystem::path, CLI_OPTION_NULL> inputFile("Input File", "File to be run on", "f", "input", "file");
    cli::Opt<bool, CLI_OPTION_NULL> useExperimentalFeatures("Experimental Features", "Enable beta features", "e", "experimental", "beta");
    cli::parse(argc, argv, inputFile, useExperimentalFeatures);
}
