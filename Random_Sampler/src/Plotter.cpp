#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "Plotter.h"
extern "C" {
#include <oxidd/capi.h>
}

using Var = oxidd::capi::oxidd_var_no_t;
using Sample = std::unordered_map<Var, bool>;

namespace oxidd::capi {

    // Update counts from one sample
    void update_counts(const Sample& s,
                                    std::unordered_map<Var, Freq>& acc) {
        for (const auto& [v, val] : s) {
            auto& f = acc[v];
            f.total++;
            if (val) f.true_count++;
        }
    }

    // Convert counts to rows and (optionally) sort by label
    std::vector<Row>
    to_rows(const std::unordered_map<Var, Freq>& acc,
            const std::unordered_map<Var, oxidd::capi::BDDFactory::BDDFeat>* names) {
        std::vector<Row> rows;
        rows.reserve(acc.size());
        for (const auto& [v, f] : acc) {
            double p = f.total ? double(f.true_count) / double(f.total) : 0.0;
            std::string label = names ? names->at(v).name : "NAN";
            rows.push_back({label, v, f.true_count, f.total, p});
        }
        std::sort(rows.begin(), rows.end(),
                [](const Row& a, const Row& b){ return a.label < b.label; });
        return rows;
    }

    // Write CSV for plotting (optional)
    void write_csv(const std::vector<Row>& rows,
                                const std::string& path) {
        std::ofstream out(path);
        out << "label,var,true,total,p_true\n";
        for (const auto& r : rows)
            out << r.label << "," << r.v << "," << r.t << "," << r.n << "," << r.p << "\n";
    }
}

