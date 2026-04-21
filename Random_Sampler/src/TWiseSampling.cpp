#include "TWiseSamplingLib.h"

#include <exception>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    try {
        if (argc < 4) {
            std::cerr << "Usage: ./greedy_twise_sampling <feature_model.xml> <t> <output.csv>\n";
            return 1;
        }

        const std::string FilePath = argv[1];
        const size_t SourceT = std::stoul(argv[2]);
        const std::string OutputCsv = argv[3];
        const std::string SystemName = twise::baseNameWithoutExtension(FilePath);

        const size_t SampleSize = twise::computeGreedySampleSize(FilePath, SourceT);
        twise::appendResultsToCsv(OutputCsv, SystemName, SourceT, SampleSize);

        return 0;
    } catch (const std::exception& E) {
        std::cerr << "ERROR: " << E.what() << "\n";
        return 1;
    }
}