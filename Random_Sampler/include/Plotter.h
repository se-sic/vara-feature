#ifndef PLOTTER_H
#define PLOTTER_H

#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "../../BDD/include/BDDFactory.h"
extern "C" {
#include <oxidd/capi.h>
}

namespace oxidd::capi {
    using Var = oxidd::capi::oxidd_var_no_t;
    using Sample = std::unordered_map<Var, bool>;
    struct Freq { size_t true_count = 0; size_t total = 0; };
    struct Row { std::string label; Var v; size_t t; size_t n; double p; };


    void update_counts(const Sample& s, std::unordered_map<Var, Freq>& acc);
    std::vector<Row>to_rows(const std::unordered_map<Var, Freq>& acc, const std::unordered_map<Var, oxidd::capi::BDDFactory::BDDFeat>* names);
    void write_csv(const std::vector<Row>& rows, const std::string& path = "freq.csv");
}


#endif // PLOTTER_H

                                

