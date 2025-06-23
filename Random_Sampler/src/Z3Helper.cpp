#include "vara/Solver/Z3Solver.h"
#include <z3++.h>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <tinyxml2.h>
#include "Z3Helper.hpp"

using vara::solver::Z3Solver;
using tinyxml2::XMLDocument;
using tinyxml2::XMLElement;
using tinyxml2::XML_SUCCESS;

std::vector<z3::expr> parseConstraints(z3::context &ctx, const std::vector<std::string> &constraints) {
    std::vector<z3::expr> parsed;
    for (const auto &str : constraints) {
        z3::expr_vector exp = ctx.parse_string(str.c_str());

        for (int i=0; i < exp.size(); i++) {
            parsed.push_back(exp[i]);
        }
    }
    return parsed;
}

void addXmlConstraintsToSolver(Z3Solver &solver, const std::string &xmlPath) {
    z3::context &ctx = solver.getContext();  
    z3::solver &z3s = solver.getRawSolver(); 

    XMLDocument doc;
    if (doc.LoadFile(xmlPath.c_str()) != XML_SUCCESS) {
        std::cerr << "Fehler beim Laden der XML-Datei: " << xmlPath << std::endl;
        return;
    }

    std::map<std::string, z3::expr> vars;

    XMLElement *root = doc.FirstChildElement("vm")->FirstChildElement("binaryOptions");
    for (XMLElement *opt = root->FirstChildElement("configurationOption"); opt; opt = opt->NextSiblingElement("configurationOption")) {
        const char *name = opt->FirstChildElement("name")->GetText();
        vars[name] = ctx.bool_const(name);
    }

    for (XMLElement *opt = root->FirstChildElement("configurationOption"); opt; opt = opt->NextSiblingElement("configurationOption")) {
        const char *src = opt->FirstChildElement("name")->GetText();
        XMLElement *excl = opt->FirstChildElement("excludedOptions");
        if (excl) {
            for (XMLElement *entry = excl->FirstChildElement("options"); entry; entry = entry->NextSiblingElement("options")) {
                const char *target = entry->GetText();
                z3s.add(z3::implies(vars[src], !vars[target]));
            }
        }

        XMLElement* impl = opt->FirstChildElement("impliedOptions");
        if (impl) {
        for (XMLElement* entry = impl->FirstChildElement("options"); entry; entry = entry->NextSiblingElement("options")) {
            const char* target = entry->GetText();
            if (vars.count(src) && vars.count(target)) {
                z3s.add(z3::implies(vars[src], vars[target]));
            }
        }
    }
        
    }

    XMLElement *bcon = doc.FirstChildElement("vm")->FirstChildElement("booleanConstraints");
    if (bcon) {
        for (XMLElement *c = bcon->FirstChildElement("constraint"); c; c = c->NextSiblingElement("constraint")) {
            std::string exprStr = c->GetText();
            z3::expr e = ctx.parse_string(exprStr.c_str());
            z3s.add(e);
        }
    }

    XMLElement *mcon = doc.FirstChildElement("vm")->FirstChildElement("mixedConstraints");
    if (mcon) {
        for (XMLElement *c = mcon->FirstChildElement("constraint"); c; c = c->NextSiblingElement("constraint")) {
            std::string exprStr = c->GetText();
            size_t a = exprStr.find("(");
            size_t b = exprStr.find("*");
            size_t cpos = exprStr.find(")");
            std::string left = exprStr.substr(a + 1, b - a - 1);
            std::string right = exprStr.substr(b + 2, cpos - b - 2);
            z3s.add(!(vars[left] && vars[right]));
        }
    }

    XMLElement* numRoot = doc.FirstChildElement("vm")->FirstChildElement("numericOptions");
    if (numRoot) {
        for (XMLElement* opt = numRoot->FirstChildElement("configurationOption"); opt; opt = opt->NextSiblingElement("configurationOption")) {
            const char* name = opt->FirstChildElement("name")->GetText();
            const char* valuesStr = opt->FirstChildElement("values")->GetText(); 

            z3::expr var = ctx.int_const(name);
            intVars[name] = var;

            std::vector<int> values;
            std::stringstream ss(valuesStr);
            std::string item;
            while (std::getline(ss, item, ';')) {
                values.push_back(std::stoi(item));
            }

            z3::expr allowed = (var == values[0]);
            for (size_t i = 1; i < values.size(); ++i) {
                allowed = allowed || (var == values[i]);
            }
            z3s.add(allowed);
        }
    }

    XMLElement* nbRoot = doc.FirstChildElement("vm")->FirstChildElement("nonBooleanConstraints");
    if (nbRoot) {
        for (XMLElement* c = nbRoot->FirstChildElement("constraint"); c; c = c->NextSiblingElement("constraint")) {
            std::string exprStr = c->GetText();
            try {
                z3::expr e = ctx.parse_string(exprStr.c_str());
                z3s.add(e);
            } catch (...) {
                std::cerr << "Warnung: konnte nonBooleanConstraint nicht parsen: " << exprStr << std::endl;
            }
        }
    }

}
