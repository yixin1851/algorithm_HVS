#include "ApsRawTestRunner.h"

#include <iostream>
#include <string>

namespace {

void PrintUsage()
{
	std::cout << "Usage:\n";
	std::cout << "  AlgorithmDemo[d].exe\n";
	std::cout << "  AlgorithmDemo[d].exe --profile <profile.json>\n";
}

} // namespace

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i) {
		const std::string arg = argv[i];
		if (arg == "--profile" || arg == "-p") {
			if (i + 1 >= argc) {
				PrintUsage();
				return 1;
			}
			return RunApsRawTestProfile(argv[i + 1]);
		}
		if (arg == "--help" || arg == "-h" || arg == "/?") {
			PrintUsage();
			return 0;
		}
	}
	return RunApsRawTestDemo();
}
