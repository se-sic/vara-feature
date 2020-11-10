#include "vara/Feature/FeatureModelParser.h"

#include "XmlConstants.h"
#include "SxfmConstants.h"

#include <iostream>
#include <regex>

namespace vara::feature {

bool FeatureModelXmlParser::parseConfigurationOption(xmlNode *Node,
                                                     bool Num = false) {
  string Name;
  bool Opt = false;
  int MinValue = 0;
  int MaxValue = 0;
  std::vector<int> Values;
  std::optional<FeatureSourceRange> Loc;
  for (xmlNode *Head = Node->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      string Cnt = std::string(
          reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                       xmlNodeGetContent(Head), xmlFree)
                                       .get()));
      if (!xmlStrcmp(Head->name, XmlConstants::NAME)) {
        Name = Cnt;
      } else if (!xmlStrcmp(Head->name, XmlConstants::OPTIONAL)) {
        Opt = Cnt == "True";
      } else if (!xmlStrcmp(Head->name, XmlConstants::PARENT)) {
        FMB.addParent(Name, Cnt);
      } else if (!xmlStrcmp(Head->name, XmlConstants::EXCLUDEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              FMB.addExclude(Name, reinterpret_cast<char *>(CCnt.get()));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::LOCATION)) {
        fs::path Path;
        std::optional<FeatureSourceRange::FeatureSourceLocation> Start;
        std::optional<FeatureSourceRange::FeatureSourceLocation> End;
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::PATH)) {
              Path = fs::path(reinterpret_cast<char *>(
                  std::unique_ptr<xmlChar, void (*)(void *)>(
                      xmlNodeGetContent(Child), xmlFree)
                      .get()));

            } else if (!xmlStrcmp(Child->name, XmlConstants::START)) {
              Start = createFeatureSourceLocation(Child);
            } else if (!xmlStrcmp(Child->name, XmlConstants::END)) {
              End = createFeatureSourceLocation(Child);
            }
          }
        }
        Loc = FeatureSourceRange(Path, Start, End);
      } else if (Num) {
        if (!xmlStrcmp(Head->name, XmlConstants::MINVALUE)) {
          MinValue = std::stoi(Cnt);
        } else if (!xmlStrcmp(Head->name, XmlConstants::MAXVALUE)) {
          MaxValue = std::stoi(Cnt);
        } else if (!xmlStrcmp(Head->name, XmlConstants::VALUES)) {
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
      return FMB.makeFeature<NumericFeature>(
          Name, std::make_pair(MinValue, MaxValue), Opt, std::move(Loc));
    }
    return FMB.makeFeature<NumericFeature>(Name, Values, Opt, std::move(Loc));
  }
  return FMB.makeFeature<BinaryFeature>(Name, Opt, std::move(Loc));
}

bool FeatureModelXmlParser::parseOptions(xmlNode *Node, bool Num = false) {
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONFIGURATIONOPTION)) {
        if (!parseConfigurationOption(H, Num)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool FeatureModelXmlParser::parseConstraints(xmlNode *Node) {
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONSTRAINT)) {
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

bool FeatureModelXmlParser::parseVm(xmlNode *Node) {
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(Node, XmlConstants::NAME),
                                                   xmlFree);
    FMB.setVmName(std::string(reinterpret_cast<char *>(Cnt.get())));
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(Node, XmlConstants::ROOT),
                                                   xmlFree);
    FMB.setPath(Cnt ? fs::path(reinterpret_cast<char *>(Cnt.get()))
                    : fs::current_path());
  }
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::BINARYOPTIONS)) {
        if (!parseOptions(H)) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, XmlConstants::NUMERICOPTIONS)) {
        if (!parseOptions(H, true)) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, XmlConstants::BOOLEANCONSTRAINTS)) {
        if (!parseConstraints(H)) {
          return false;
        }
      }
    }
  }
  return true;
}

FeatureSourceRange::FeatureSourceLocation
FeatureModelXmlParser::createFeatureSourceLocation(xmlNode *Node) {
  int Line = 0;
  int Column = 0;
  for (xmlNode *Head = Node->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(Head->name, XmlConstants::LINE)) {
        Line = atoi(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(Head), xmlFree)
                                         .get()));
      } else if (!xmlStrcmp(Head->name, XmlConstants::COLUMN)) {
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
                    xmlParserInputBufferCreateMem(XmlConstants::DtdRaw.c_str(),
                                                  XmlConstants::DtdRaw.length(),
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

//===----------------------------------------------------------------------===//
//                               FeatureModelSxfmParser Class
//===----------------------------------------------------------------------===//

std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)>
FeatureModelSxfmParser::createDtd() {
  std::unique_ptr<xmlDtd, void (*)(xmlDtdPtr)> Dtd(
      xmlIOParseDTD(nullptr,
                    xmlParserInputBufferCreateMem(SxfmConstants::DtdRaw.c_str(),
                                                  SxfmConstants::DtdRaw.length(),
                                                  XML_CHAR_ENCODING_UTF8),
                    XML_CHAR_ENCODING_UTF8),
      xmlFreeDtd);
  xmlCleanupParser();
  assert(Dtd && "Failed to parse DTD.");
  return Dtd;
}

bool FeatureModelSxfmParser::parseFeatureTree(xmlChar *feature_tree) {
  return true;
}

bool FeatureModelSxfmParser::parseConstraints(xmlChar *constraints) {
  return true;
}

bool FeatureModelSxfmParser::parseVm(xmlNode *Node) {
  // Parse the name first. This procedure is the same as for parsing the
  // XML file.
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(Node, XmlConstants::NAME),
                                                   xmlFree);
    FMB.setVmName(std::string(reinterpret_cast<char *>(Cnt.get())));
  }
  // After the feature model tag, some metadata is provided.
  // Currently, we skip the metadata and continue with parsing the feature tree

  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, SxfmConstants::FEATURE_TREE)) {
        // Check whether a custom indentation is defined in the feature tree and
        // set it accordingly
        {
          std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(Node, SxfmConstants::INDENTATION),
                                                         xmlFree);
          if (Cnt) {
            Indentation = reinterpret_cast<char*>(Cnt.get());
          }

        }
        // TODO: Implement parsing the feature tree
        if (!parseFeatureTree(xmlNodeGetContent(H))) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, SxfmConstants::CONSTRAINTS)) {
        // TODO: Implement parsing the constraints
        if (!parseConstraints(xmlNodeGetContent(H))) {
          return false;
        }
      }
    }
    return true;
  }

  // Parse the feature tree with all its features and relations among them.

  // Finally, parse the cross-tree-constraints in the constraints tag.
  return true;
}

std::unique_ptr<FeatureModel> FeatureModelSxfmParser::buildFeatureModel() {
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> Doc = parseDoc();
  if (!Doc) {
    return nullptr;
  }

  FMB.init();
  return parseVm(xmlDocGetRootElement(Doc.get())) ? FMB.buildFeatureModel()
                                                  : nullptr;
}

std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> FeatureModelSxfmParser::parseDoc() {
  // Initialize the XML parser
  std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> Ctxt(
      xmlNewParserCtxt(), xmlFreeParserCtxt);
  // Parse the given model by libxml2
  std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)> Doc(
      xmlCtxtReadMemory(Ctxt.get(), Sxfm.c_str(), Sxfm.length(), nullptr, nullptr,
                        XML_PARSE_NOBLANKS),
      xmlFreeDoc);
  xmlCleanupParser();

  // In the following, the document is validated.
  // Therefore, (1) check whether it could be parsed
  if (Doc && Ctxt->valid) {
    // (2) validate the sxfm format by using the dtd (document type definition) file
    xmlValidateDtd(&Ctxt->vctxt, Doc.get(), createDtd().get());
    if (Ctxt->vctxt.valid) {
      // TODO (CK):
      // and (3) check the tree-like structure of the embedded feature model
      // as well as constraints
      return Doc;
    } else {
      std::cerr << "Failed to validate DTD." << std::endl;
    }
  } else {
    std::cerr << "Failed to parse / validate XML." << std::endl;
  }
  return std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>(nullptr, nullptr);
}

// TODO (CK): Check the feature model before reading in the whole model by
// using a syntax graph?
/// This method checks if the given feature model is valid
/// \return true iff the feature model is valid
bool FeatureModelSxfmParser::verifyFeatureModel() { return parseDoc().get(); }

} // namespace vara::feature
