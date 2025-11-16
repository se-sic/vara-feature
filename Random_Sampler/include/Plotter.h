#ifndef PLOTTER_H
#define PLOTTER_H

#include "../../BDD/include/BDDFactory.h"
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
extern "C" {
#include <oxidd/capi.h>
}

namespace bdd::sample {
    using Var = oxidd::capi::oxidd_var_no_t;
    using Sample = std::unordered_map<Var, bool>;
    struct Freq { size_t TrueCount = 0; size_t Total = 0; };
    struct Row { std::string Label; Var Level; size_t Count; size_t N; double Prob; double TheoreticalProb; double Error;};


    void updateCounts(const Sample& S, std::unordered_map<Var, Freq>& Acc);
    std::vector<Row>toRows(const std::unordered_map<Var, Freq>& Acc, const std::unordered_map<Var, bdd::sample::BDDFactory::BDDFeat>* Names);
    void writeCsv(const std::vector<Row>& Rows, const std::string& Path = "freq.csv");
} // namespace bdd::sample


#endif // PLOTTER_H

                                

