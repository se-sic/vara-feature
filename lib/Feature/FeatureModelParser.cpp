#include "vara/Feature/FeatureModelParser.h"

#include "llvm/Support/raw_ostream.h"

#include "XmlConstants.h"

#include <iostream>
#include <regex>

using std::make_unique;

namespace vara::feature {

bool FeatureModelXmlParser::parseConfigurationOption(xmlNode *Node,
                                                     bool Num = false) {
  string Name = "root";
  bool Opt = false;
  int MinValue = 0;
  int MaxValue = 0;
  std::vector<int> Values;
  std::vector<FeatureSourceRange> SourceRanges;
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
        // TODO(s9latimm) fix me -- breaks if Name not set / parsed yet
        assert(!Name.empty());
        FMB.addEdge(Cnt, Name);
      } else if (!xmlStrcmp(Head->name, XmlConstants::EXCLUDEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              FMB.addConstraint(make_unique<ExcludesConstraint>(
                  make_unique<PrimaryFeatureConstraint>(
                      make_unique<Feature>(Name)),
                  make_unique<PrimaryFeatureConstraint>(make_unique<Feature>(
                      reinterpret_cast<char *>(CCnt.get())))));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::IMPLIEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              FMB.addConstraint(make_unique<ImpliesConstraint>(
                  make_unique<PrimaryFeatureConstraint>(
                      make_unique<Feature>(Name)),
                  make_unique<PrimaryFeatureConstraint>(make_unique<Feature>(
                      reinterpret_cast<char *>(CCnt.get())))));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::LOCATIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::SOURCERANGE)) {
              SourceRanges.push_back(createFeatureSourceRange(Child));
            }
          }
        }
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
      return FMB.makeFeature<NumericFeature>(Name,
                                             std::make_pair(MinValue, MaxValue),
                                             Opt, std::move(SourceRanges));
    }
    return FMB.makeFeature<NumericFeature>(Name, Values, Opt,
                                           std::move(SourceRanges));
  }
  return FMB.makeFeature<BinaryFeature>(Name, Opt, std::move(SourceRanges));
}

FeatureSourceRange
FeatureModelXmlParser::createFeatureSourceRange(xmlNode *Head) {
  fs::path Path;
  std::optional<FeatureSourceRange::FeatureSourceLocation> Start;
  std::optional<FeatureSourceRange::FeatureSourceLocation> End;
  enum FeatureSourceRange::Category Category;

  std::unique_ptr<xmlChar, void (*)(void *)> Tmp(
      xmlGetProp(Head, XmlConstants::CATEGORY), xmlFree);
  if (Tmp) {
    if (xmlStrcmp(Tmp.get(), XmlConstants::NECESSARY) == 0) {
      Category = FeatureSourceRange::Category::necessary;
    } else if (xmlStrcmp(Tmp.get(), XmlConstants::INESSENTIAL) == 0) {
      Category = FeatureSourceRange::Category::inessential;
    } else {
      llvm_unreachable("DTD check should have rejected this file or this code "
                       "was not updated to handle new values.");
    }
  } else {
    Category = FeatureSourceRange::Category::necessary;
  }
  for (xmlNode *Child = Head->children; Child; Child = Child->next) {
    if (Child->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(Child->name, XmlConstants::PATH)) {
        Path = fs::path(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(Child), xmlFree)
                                         .get()));

      } else if (!xmlStrcmp(Child->name, XmlConstants::START)) {
        Start = createFeatureSourceLocation(Child);
      } else if (!xmlStrcmp(Child->name, XmlConstants::END)) {
        End = createFeatureSourceLocation(Child);
      }
    }
  }
  return FeatureSourceRange(Path, Start, End, Category);
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

bool FeatureModelXmlParser::parseConstraints(xmlNode *Node) { // NOLINT
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONSTRAINT)) {
        string Cnt = std::string(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(H), xmlFree)
                                         .get()));
        // TODO(se-passau/VaRA#664): Implement advanced parsing into constraint
        //  tree
      }
    }
  }
  return true;
}

bool FeatureModelXmlParser::parseVm(xmlNode *Node) {
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(
        xmlGetProp(Node, XmlConstants::NAME), xmlFree);
    FMB.setVmName(std::string(reinterpret_cast<char *>(Cnt.get())));
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(
        xmlGetProp(Node, XmlConstants::ROOT), xmlFree);
    FMB.setPath(Cnt ? fs::path(reinterpret_cast<char *>(Cnt.get()))
                    : fs::current_path());
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(
        xmlGetProp(Node, XmlConstants::COMMIT), xmlFree);
    FMB.setCommit(Cnt ? std::string(reinterpret_cast<char *>(Cnt.get())) : "");
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
      if (xmlValidateDtdFinal(&Ctxt->vctxt, Doc.get()) == 1) {
        return Doc;
      }
      llvm::errs() << "Failed to validate DTD in final step\n";
    }
    llvm::errs() << "Failed to validate DTD.\n";
  } else {
    llvm::errs() << "Failed to parse / validate XML.\n";
  }
  return std::unique_ptr<xmlDoc, void (*)(xmlDocPtr)>(nullptr, nullptr);
}

// TODO(s9latimm): replace with builder err
bool FeatureModelXmlParser::verifyFeatureModel() { return parseDoc().get(); }

} // namespace vara::feature
