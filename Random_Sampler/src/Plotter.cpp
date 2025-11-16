#include "Plotter.h"
#include <algorithm>
#include <cmath>
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
            double P = f.Total > 0 ? double(f.TrueCount) / double(f.Total) : 0.0;
            std::string Label = Names ? Names->at(v).Name : "NAN";
            double Theoretic = Names->at(v).Probability.value_or(0.0);
            double Error = std::fabs(P - Theoretic);
            Rows.push_back({Label, v, f.TrueCount, f.Total, P, Theoretic, Error});
        }
        std::ranges::sort(Rows,[](const Row& A, const Row& B){ return A.Label < B.Label; });
        return Rows;
    }

    // Write CSV for plotting (optional)
    void writeCsv(const std::vector<Row>& Rows, const std::string& Path) {
        std::ofstream Out(Path);
        Out << "Label,Level,Count,Total,Prob,TheoProb,Error\n";
        for (const auto& R : Rows) {
            Out << R.Label << "," << R.Level << "," << R.Count << "," << R.N << "," << R.Prob << "," << R.TheoreticalProb << "," << R.Error << "\n";
        }
    }
} // namespace bdd::sample