#include "Plotter.h"
#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
extern "C" {
#include <oxidd/bdd.hpp>
}

using Var = oxidd::capi::oxidd_var_no_t;
using Sample = std::unordered_map<Var, bool>;

namespace bdd::sample {
    
    void updateCounts(const Sample& S, std::unordered_map<Var, bdd::sample::Freq>& Acc) { 
        for (const auto& [v, val] : S) {
            auto& F = Acc[v];
            F.Total++;
            if (val) { F.TrueCount++; }
        }
    }

    // Convert counts to rows and (optionally) sort by label
    std::vector<bdd::sample::Row>
    toRows(const std::unordered_map<Var, bdd::sample::Freq>& Acc,
           const std::unordered_map<Var, bdd::sample::BDDFactory::BDDFeat>* Names) {
        std::vector<bdd::sample::Row> Rows;
        Rows.reserve(Acc.size());
        for (const auto& [v, f] : Acc) {
            double P = f.Total ? double(f.TrueCount) / double(f.Total) : 0.0;
            std::string Label = Names ? Names->at(v).Name : "NAN";
            Rows.push_back({Label, v, f.TrueCount, f.Total, P});
        }
        std::ranges::sort(Rows,[](const Row& A, const Row& B){ return A.Label < B.Label; });
        return Rows;
    }

    // Write CSV for plotting (optional)
    void writeCsv(const std::vector<Row>& Rows, const std::string& Path) {
        std::ofstream Out(Path);
        Out << "label,var,true,total,p_true\n";
        for (const auto& R : Rows) {
            Out << R.Label << "," << R.V << "," << R.T << "," << R.N << "," << R.P << "\n";
        }
    }
} // namespace bdd::sample