#include "vara/Feature/FeatureModelParser.h"

#include <iostream>
#include <regex>

namespace vara::feature {

bool FeatureModelXmlParser::parseConfigurationOption(xmlNode *N,
                                                     bool Num = false) {
  string Name;
  bool Opt = false;
  int MinValue = 0;
  int MaxValue = 0;
  std::vector<int> Values;
  std::optional<FeatureSourceRange> Loc;
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
        FMB.addChild(Cnt, Name);
      } else if (!xmlStrcmp(Head->name, EXCLUDEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              FMB.addExclude(Name, reinterpret_cast<char *>(CCnt.get()));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, LOCATION)) {
        fs::path Path;
        std::optional<FeatureSourceRange::FeatureSourceLocation> Start;
        std::optional<FeatureSourceRange::FeatureSourceLocation> End;
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, PATH)) {
              Path = fs::path(reinterpret_cast<char *>(
                  std::unique_ptr<xmlChar, void (*)(void *)>(
                      xmlNodeGetContent(Child), xmlFree)
                      .get()));

            } else if (!xmlStrcmp(Child->name, START)) {
              Start = createFeatureSourceLocation(Child);
            } else if (!xmlStrcmp(Child->name, END)) {
              End = createFeatureSourceLocation(Child);
            }
          }
        }
        Loc = FeatureSourceRange(Path, Start, End);
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
            Values.emplace_back(std::stoi(Matches.str()));
          }
        }
      }
    }
  }
  if (Num) {
    if (Values.empty()) {
      return FMB.addFeature(Name, std::make_pair(MinValue, MaxValue), Opt,
                            std::move(Loc));
    }
    return FMB.addFeature(Name, Values, Opt, std::move(Loc));
  }
  return FMB.addFeature(Name, Opt, std::move(Loc));
}

bool FeatureModelXmlParser::parseOptions(xmlNode *N, bool Num = false) {
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, CONFIGURATIONOPTION)) {
        if (!parseConfigurationOption(H, Num)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool FeatureModelXmlParser::parseConstraints(xmlNode *N) {
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, CONSTRAINT)) {
        string Cnt = std::string(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(H), xmlFree)
                                         .get()));
        const std::regex Regex(R"((!?\w+))");
        std::smatch Matches;
        FeatureModel::ConstraintTy Constraint;
        for (string Suffix = Cnt; regex_search(Suffix, Matches, Regex);
             Suffix = Matches.suffix()) {
          string B = Matches.str(0);
          if (B.length() > 1 && B[0] == '!') {
            Constraint.emplace_back(B.substr(1, B.length()), false);
          } else {
            Constraint.emplace_back(B, true);
          }
        }
        FMB.addConstraint(Constraint);
      }
    }
  }
  return true;
}

bool FeatureModelXmlParser::parseVm(xmlNode *N) {
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(N, NAME),
                                                   xmlFree);
    FMB.setVmName(std::string(reinterpret_cast<char *>(Cnt.get())));
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(N, ROOT),
                                                   xmlFree);
    FMB.setPath(Cnt ? fs::path(reinterpret_cast<char *>(Cnt.get()))
                    : fs::current_path());
  }
  for (xmlNode *H = N->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, BINARYOPTIONS)) {
        if (!parseOptions(H)) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, NUMERICOPTIONS)) {
        if (!parseOptions(H, true)) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, BOOLEANCONSTRAINTS)) {
        if (!parseConstraints(H)) {
          return false;
        }
      }
    }
  }
  return true;
}

FeatureSourceRange::FeatureSourceLocation
FeatureModelXmlParser::createFeatureSourceLocation(xmlNode *N) {
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
  return FeatureSourceRange::FeatureSourceLocation(Line, Column);
}

std::unique_ptr<FeatureModel> FeatureModelXmlParser::buildFeatureModel() {
  auto Doc = parseDoc();
  if (!Doc) {
    return nullptr;
  }
  FMB.init();
  return parseVm(xmlDocGetRootElement(Doc.get())) ? FMB.buildFeatureModel()
                                                  : nullptr;
}

std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)>
FeatureModelXmlParser::createDtd() {
  std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> Dtd(
      xmlIOParseDTD(nullptr,
                    xmlParserInputBufferCreateMem(DtdRaw.c_str(),
                                                  DtdRaw.length(),
                                                  XML_CHAR_ENCODING_UTF8),
                    XML_CHAR_ENCODING_UTF8),
      xmlFreeDtd);
  xmlCleanupParser();
  assert(Dtd && "Failed to parse DTD.");
  return Dtd;
}

std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> FeatureModelXmlParser::parseDoc() {
  std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> Ctxt(
      xmlNewParserCtxt(), xmlFreeParserCtxt);
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> Doc(
      xmlCtxtReadMemory(Ctxt.get(), Xml.c_str(), Xml.length(), nullptr, nullptr,
                        XML_PARSE_NOBLANKS),
      xmlFreeDoc);
  xmlCleanupParser();
  if (Doc && Ctxt->valid) {
    xmlValidateDtd(&Ctxt->vctxt, Doc.get(), createDtd().get());
    if (Ctxt->vctxt.valid) {
      return Doc;
    } else {
      std::cerr << "Failed to validate DTD." << std::endl;
    }
  } else {
    std::cerr << "Failed to parse / validate XML." << std::endl;
  }
  return std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>(nullptr, nullptr);
}

// TODO(s9latimm): replace with builder err
bool FeatureModelXmlParser::verifyFeatureModel() { return parseDoc().get(); }

} // namespace vara::feature
