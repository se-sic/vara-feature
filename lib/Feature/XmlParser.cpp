#include "vara/Feature/XmlParser.h"

#include <iostream>
#include <regex>

namespace vara::feature {

void XmlParser::parseConfigurationOption(xmlNode *N, bool Num = false) {
  string Name;
  bool Opt = false;
  int MinValue = 0;
  int MaxValue = 0;
  std::vector<int> Vals;
  std::optional<Location> Loc;
  for (xmlNode *Head = N->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      string Cnt = std::string(
          reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                       xmlNodeGetContent(Head), xmlFree)
                                       .get()));
      if (!xmlStrcmp(Head->name, NAME)) {
        Name = Cnt;
      } else if (!xmlStrcmp(Head->name, OPTIONAL)) {
        Opt = Cnt == "True";
      } else if (!xmlStrcmp(Head->name, PARENT)) {
        RawEdges.emplace_back(Cnt, Name);
      } else if (!xmlStrcmp(Head->name, EXCLUDEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              RawExcludes.emplace_back(Name,
                                       reinterpret_cast<char *>(CCnt.get()));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, LOCATION)) {
        std::string Path;
        std::optional<Location::TableEntry> Start;
        std::optional<Location::TableEntry> End;
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, PATH)) {
              Path = reinterpret_cast<char *>(
                  std::unique_ptr<xmlChar, void (*)(void *)>(
                      xmlNodeGetContent(Child), xmlFree)
                      .get());

            } else if (!xmlStrcmp(Child->name, START)) {
              Start = createTableEntry(Child);
            } else if (!xmlStrcmp(Child->name, END)) {
              End = createTableEntry(Child);
            }
          }
        }
        Loc = Location(Path, Start, End);
      } else if (Num) {
        if (!xmlStrcmp(Head->name, MINVALUE)) {
          MinValue = std::stoi(Cnt);
        } else if (!xmlStrcmp(Head->name, MAXVALUE)) {
          MaxValue = std::stoi(Cnt);
        } else if (!xmlStrcmp(Head->name, VALUES)) {
          const std::regex Regex(R"(\d+)");
          std::smatch Matches;
          for (string Suffix = Cnt; regex_search(Suffix, Matches, Regex);
               Suffix = Matches.suffix()) {
            Vals.emplace_back(std::stoi(Matches.str()));
          }
        }
      }
    }
  }
  if (Num) {
    if (Vals.empty()) {
      Features.try_emplace(
          Name, std::make_unique<NumericFeature>(
                    Name, Opt,
                    std::variant<std::pair<int, int>, std::vector<int>>(
                        std::make_pair(MinValue, MaxValue)),
                    std::move(Loc)));

    } else {
      Features.try_emplace(
          Name, std::make_unique<NumericFeature>(
                    Name, Opt,
                    std::variant<std::pair<int, int>, std::vector<int>>(Vals),
                    std::move(Loc)));
    }
  } else {
    Features.try_emplace(
        Name, std::make_unique<BinaryFeature>(Name, Opt, std::move(Loc)));
  }
}

void XmlParser::parseOptions(xmlNode *N, bool Num = false) {
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, CONFIGURATIONOPTION)) {
        parseConfigurationOption(H, Num);
      }
    }
  }
}

void XmlParser::parseConstraints(xmlNode *N) {
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, CONSTRAINT)) {
        string Cnt = std::string(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(H), xmlFree)
                                         .get()));
        const std::regex Regex(R"((!?\w+))");
        std::smatch Matches;
        RawConstraints.emplace_back();
        for (string Suffix = Cnt; regex_search(Suffix, Matches, Regex);
             Suffix = Matches.suffix()) {
          string B = Matches.str(0);
          if (B.length() > 1 && B[0] == '!') {
            RawConstraints.back().emplace_back(B.substr(1, B.length()), false);
          } else {
            RawConstraints.back().emplace_back(B, true);
          }
        }
      }
    }
  }
}

void XmlParser::parseVm(xmlNode *N) {
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(N, NAME),
                                                   xmlFree);
    VM = std::string(reinterpret_cast<char *>(Cnt.get()));
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(N, ROOT),
                                                   xmlFree);
    RootPath = Cnt ? std::filesystem::path(reinterpret_cast<char *>(Cnt.get()))
                   : std::filesystem::current_path();
  }
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, BINARYOPTIONS)) {
        parseOptions(H);
      } else if (!xmlStrcmp(H->name, NUMERICOPTIONS)) {
        parseOptions(H, true);
      } else if (!xmlStrcmp(H->name, BOOLEANCONSTRAINTS)) {
        parseConstraints(H);
      }
    }
  }
}

Location::TableEntry XmlParser::createTableEntry(xmlNode *N) {
  int Line = 0;
  int Column = 0;
  for (xmlNode *Head = N->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(Head->name, LINE)) {
        Line = atoi(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(Head), xmlFree)
                                         .get()));
      } else if (!xmlStrcmp(Head->name, COLUMN)) {
        Column = atoi(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(Head), xmlFree)
                                         .get()));
      }
    }
  }
  return Location::TableEntry(Line, Column);
}

std::unique_ptr<FeatureModel> XmlParser::buildFeatureModel() {
  if (!Doc) {
    return nullptr;
  }
  Features.clear();
  parseVm(xmlDocGetRootElement(Doc.get()));
  if (Features.find("root") == Features.end()) {
    Features.try_emplace(
        "root", std::make_unique<BinaryFeature>("root", false, std::nullopt));
  }

  for (const auto &P : RawEdges) {
    if (Features.find(P.first) != Features.end() &&
        Features.find(P.second) != Features.end()) {
      assert(Features[P.first] && Features[P.second]);
      Features[P.first]->addChild(Features[P.second].get());
      Features[P.second]->addParent(Features[P.first].get());
    }
  }

  for (const auto &P : RawExcludes) {
    if (Features.find(P.first) != Features.end() &&
        Features.find(P.second) != Features.end()) {
      assert(Features[P.first] && Features[P.second]);
      Features[P.first]->addExclude(Features[P.second].get());
    }
  }

  for (const auto &C : RawConstraints) {
    Constraints.emplace_back();
    for (const auto &P : C) {
      if (Features.find(P.first) != Features.end()) {
        assert(Features[P.first]);
        Constraints.back().emplace_back(Features[P.first].get(), P.second);
      }
    }
  }

  for (const auto &C : Constraints) {
    if (C.size() == 2) {
      if (C[0].second != C[1].second) {
        if (C[0].second) { // A || !B
          C[1].first->addImplication(C[0].first);
        } else { // !A || B
          C[0].first->addImplication(C[1].first);
        }
      } else if (!(C[0].second || C[1].second)) { // !A || !B
        C[0].first->addExclude(C[1].first);
        C[1].first->addExclude(C[0].first);
      } else if (C[0].second && C[1].second) { // A || B
        C[0].first->addAlternative(C[1].first);
        C[1].first->addAlternative(C[0].first);
      }
    } else if (C.size() > 2) {
      bool B = true;
      for (const auto &P : C) {
        B &= P.second;
      }
      if (B) {
        for (const auto &P : C) {
          for (const auto &PP : C) {
            if (P.first != PP.first) {
              P.first->addAlternative(PP.first);
            }
          }
        }
      }
    }
  }

  Feature *Root = Features["root"].get();
  for (const auto &P : Features) {
    if (P.second->isRoot() && P.second.get() != Root) {
      Root->addChild(P.second.get());
      P.second->addParent(Root);
    }
    for (auto &C : *P.second) {
      if (C->isOptional()) {
        P.second->addRelationship(std::make_unique<Optional<Feature>>(C));
      } else {
        P.second->addRelationship(std::make_unique<Mandatory<Feature>>(C));
      }
    }
    // TODO (se-passau/VaRA#42): relationships or and xor
  }
  return std::make_unique<FeatureModel>(VM, RootPath, std::move(Features),
                                        Constraints);
}

bool XmlParser::parseDtd(const string &Filename) {
  std::unique_ptr<FILE, int (*)(FILE *)> File(fopen(Filename.c_str(), "r"),
                                              fclose);
  if (!File) {
    return false;
  }
  xmlParserInputBufferPtr Buf =
      xmlParserInputBufferCreateFile(File.get(), XML_CHAR_ENCODING_UTF8);
  Dtd = std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)>(
      xmlIOParseDTD(nullptr, Buf, XML_CHAR_ENCODING_UTF8), xmlFreeDtd);
  if (!Dtd) {
    std::cerr << "Failed to parse \'" << Filename << "\'." << std::endl;
  }
  xmlCleanupParser();
  return Dtd.get();
}

bool XmlParser::parseDoc(const string &Filename) {
  std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> Ctxt(
      xmlNewParserCtxt(), xmlFreeParserCtxt);
  if (!Ctxt) {
    return false;
  }
  Doc = std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>(
      xmlCtxtReadFile(Ctxt.get(), Filename.c_str(), nullptr,
                      XML_PARSE_NOBLANKS),
      xmlFreeDoc);
  if (!Doc) {
    std::cerr << "Failed to parse \'" << Filename << "\'." << std::endl;
  } else if (!Ctxt->valid) {
    std::cerr << "Failed to validate \'" << Filename << "\'." << std::endl;
    Doc = nullptr;
  } else if (Dtd) {
    xmlValidateDtd(&Ctxt->vctxt, Doc.get(), Dtd.get());
    if (!Ctxt->vctxt.valid) {
      std::cerr << "Failed to validate Dtd for \'" << Filename << "\'."
                << std::endl;
      Doc = nullptr;
    }
  }
  xmlCleanupParser();
  return Doc.get();
}

} // namespace vara::feature
