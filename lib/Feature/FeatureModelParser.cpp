#include "vara/Feature/FeatureModelParser.h"
#include "vara/Feature/ConstraintParser.h"

#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include "SxfmConstants.h"
#include "XmlConstants.h"

#include <iostream>
#include <regex>

using std::make_unique;

namespace vara::feature {

std::string trim(llvm::StringRef S) { return llvm::StringRef(S).trim().str(); }

Result<FTErrorCode>
FeatureModelXmlParser::parseConfigurationOption(xmlNode *Node,
                                                bool Num = false) {
  std::string Name{"root"};
  bool Opt = false;
  int MinValue = 0;
  int MaxValue = 0;
  std::vector<int> Values;
  std::vector<FeatureSourceRange> SourceRanges;
  for (xmlNode *Head = Node->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      std::string Cnt{trim(reinterpret_cast<char *>(
          UniqueXmlChar(xmlNodeGetContent(Head), xmlFree).get()))};
      if (Cnt.empty()) {
        continue;
      }
      // The DTD enforces name to be the first element of an
      // configurationOption. This method is never called without validating
      // the input beforehand.
      if (!xmlStrcmp(Head->name, XmlConstants::NAME)) {
        Name = Cnt;
      } else if (!xmlStrcmp(Head->name, XmlConstants::OPTIONAL)) {
        Opt = Cnt == "True";
      } else if (!xmlStrcmp(Head->name, XmlConstants::PARENT)) {
        FMB.addEdge(Cnt, Name);
      } else if (!xmlStrcmp(Head->name, XmlConstants::CHILDREN)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              FMB.addEdge(Name, std::string(reinterpret_cast<char *>(
                                    std::unique_ptr<xmlChar, void (*)(void *)>(
                                        xmlNodeGetContent(Child), xmlFree)
                                        .get())));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::EXCLUDEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              UniqueXmlChar CCnt(xmlNodeGetContent(Child), xmlFree);
              FMB.addConstraint(make_unique<ExcludesConstraint>(
                  make_unique<PrimaryFeatureConstraint>(
                      make_unique<Feature>(Name)),
                  make_unique<PrimaryFeatureConstraint>(make_unique<Feature>(
                      trim(reinterpret_cast<char *>(CCnt.get()))))));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::IMPLIEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              UniqueXmlChar CCnt(xmlNodeGetContent(Child), xmlFree);
              FMB.addConstraint(make_unique<ImpliesConstraint>(
                  make_unique<PrimaryFeatureConstraint>(
                      make_unique<Feature>(Name)),
                  make_unique<PrimaryFeatureConstraint>(make_unique<Feature>(
                      trim(reinterpret_cast<char *>(CCnt.get()))))));
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
          for (std::string Suffix = Cnt; regex_search(Suffix, Matches, Regex);
               Suffix = Matches.suffix()) {
            Values.emplace_back(std::stoi(Matches.str()));
          }
        }
      }
    }
  }

  // XML has those names specified as root nodes
  if (Name == "root" || Name == "base") {
    FMB.makeRoot(Name);
  } else if (Num) {
    if (Values.empty()) {
      FMB.makeFeature<NumericFeature>(Name, std::make_pair(MinValue, MaxValue),
                                      Opt, std::move(SourceRanges));
    } else {
      FMB.makeFeature<NumericFeature>(Name, Values, Opt,
                                      std::move(SourceRanges));
    }
  } else {
    FMB.makeFeature<BinaryFeature>(Name, Opt, std::move(SourceRanges));
  }
  return Ok();
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
        Path = fs::path(trim(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(Child), xmlFree)
                                         .get())));

      } else if (!xmlStrcmp(Child->name, XmlConstants::START)) {
        Start = createFeatureSourceLocation(Child);
      } else if (!xmlStrcmp(Child->name, XmlConstants::END)) {
        End = createFeatureSourceLocation(Child);
      }
    }
  }
  return FeatureSourceRange(Path, Start, End, Category);
}

Result<FTErrorCode> FeatureModelXmlParser::parseOptions(xmlNode *Node,
                                                        bool Num = false) {
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONFIGURATIONOPTION)) {
        if (!parseConfigurationOption(H, Num)) {
          return Error(ERROR);
        }
      }
    }
  }
  return Ok();
}

Result<FTErrorCode> FeatureModelXmlParser::parseConstraints(xmlNode *Node) {
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONSTRAINT)) {
        UniqueXmlChar Cnt(xmlNodeGetContent(H), xmlFree);
        if (auto Constraint =
                ConstraintParser(
                    std::string(reinterpret_cast<char *>(Cnt.get())))
                    .buildConstraint()) {
          FMB.addConstraint(std::move(Constraint));
        } else {
          return Error(ERROR);
        }
      }
    }
  }
  return Ok();
}

Result<FTErrorCode> FeatureModelXmlParser::parseVm(xmlNode *Node) {
  {
    UniqueXmlChar Cnt(xmlGetProp(Node, XmlConstants::NAME), xmlFree);
    FMB.setVmName(trim(reinterpret_cast<char *>(Cnt.get())));
  }
  {
    UniqueXmlChar Cnt(xmlGetProp(Node, XmlConstants::ROOT), xmlFree);
    FMB.setPath(Cnt ? fs::path(trim(reinterpret_cast<char *>(Cnt.get())))
                    : fs::current_path());
  }
  {
    std::unique_ptr<xmlChar, void (*)(void *)> Cnt(
        xmlGetProp(Node, XmlConstants::COMMIT), xmlFree);
    FMB.setCommit(Cnt ? trim(reinterpret_cast<char *>(Cnt.get())) : "");
  }
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::BINARYOPTIONS)) {
        if (!parseOptions(H)) {
          return Error(ERROR);
        }
      } else if (!xmlStrcmp(H->name, XmlConstants::NUMERICOPTIONS)) {
        if (!parseOptions(H, true)) {
          return Error(ERROR);
        }
      } else if (!xmlStrcmp(H->name, XmlConstants::BOOLEANCONSTRAINTS)) {
        if (!parseConstraints(H)) {
          return Error(ERROR);
        }
      }
    }
  }
  return Ok();
}

FeatureSourceRange::FeatureSourceLocation
FeatureModelXmlParser::createFeatureSourceLocation(xmlNode *Node) {
  int Line = 0;
  int Column = 0;
  for (xmlNode *Head = Node->children; Head; Head = Head->next) {
    if (Head->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(Head->name, XmlConstants::LINE)) {
        Line = atoi(reinterpret_cast<char *>(
            UniqueXmlChar(xmlNodeGetContent(Head), xmlFree).get()));
      } else if (!xmlStrcmp(Head->name, XmlConstants::COLUMN)) {
        Column = atoi(reinterpret_cast<char *>(
            UniqueXmlChar(xmlNodeGetContent(Head), xmlFree).get()));
      }
    }
  }
  return FeatureSourceRange::FeatureSourceLocation(Line, Column);
}

/// Decide whether feature A excludes B. Beware that this method only detects
/// very simple trees with binary excludes.
bool detectExclude(const Feature *A, const Feature *B) {
  return std::any_of(
      A->excludes().begin(), A->excludes().end(), [A, B](const auto *E) {
        if (const auto *LHS =
                llvm::dyn_cast<PrimaryFeatureConstraint>(E->getLeftOperand())) {
          if (const auto *RHS = llvm::dyn_cast<PrimaryFeatureConstraint>(
                  E->getRightOperand())) {
            return (LHS->getFeature() &&
                    LHS->getFeature()->getName() == A->getName() &&
                    RHS->getFeature() &&
                    RHS->getFeature()->getName() == B->getName());
          }
        }
        return false;
      });
}

Result<FTErrorCode>
FeatureModelXmlParser::detectXMLAlternatives(FeatureModel &FM) {
  auto Transactions = FeatureModelModifyTransaction::openTransaction(FM);
  for (auto *F : FM) {
    auto Children = F->getChildren<Feature>();
    if (Children.size() > 1 &&
        std::all_of(Children.begin(), Children.end(), [Children](auto *F) {
          return !F->isOptional() &&
                 std::all_of(Children.begin(), Children.end(), [F](auto *C) {
                   return F == C ||
                          (detectExclude(F, C) && detectExclude(C, F));
                 });
        })) {
      Transactions.addRelationship(
          Relationship::RelationshipKind::RK_ALTERNATIVE, F);
    }
  }
  return Transactions.commit();
}

std::unique_ptr<FeatureModel> FeatureModelXmlParser::buildFeatureModel() {
  auto Doc = parseDoc();
  if (!Doc || !parseVm(xmlDocGetRootElement(Doc.get()))) {
    return nullptr;
  }

  auto FM = FMB.buildFeatureModel();
  if (FM) {
    detectXMLAlternatives(*FM);
  }

  return FM;
}

FeatureModelParser::UniqueXmlDtd FeatureModelXmlParser::createDtd() {
  UniqueXmlDtd Dtd(
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

FeatureModelParser::UniqueXmlDoc FeatureModelXmlParser::parseDoc() {
  std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> Ctxt(
      xmlNewParserCtxt(), xmlFreeParserCtxt);
  UniqueXmlDoc Doc(xmlCtxtReadMemory(Ctxt.get(), Xml.c_str(), Xml.length(),
                                     nullptr, nullptr, XML_PARSE_NOBLANKS),
                   xmlFreeDoc);
  xmlCleanupParser();
  if (Doc && Ctxt->valid) {
    xmlValidateDtd(&Ctxt->vctxt, Doc.get(), createDtd().get());
    if (Ctxt->vctxt.valid) {
      return Doc;
    }
    llvm::errs() << "Failed to validate DTD.\n";
  } else {
    llvm::errs() << "Failed to parse / validate XML.\n";
  }
  return UniqueXmlDoc(nullptr, nullptr);
}

Result<FTErrorCode> FeatureModelXmlParser::verifyFeatureModel() {
  if (!parseDoc().get()) {
    return Error(ERROR);
  }
  return Ok();
}

//===----------------------------------------------------------------------===//
//                        FeatureModelSxfmParser Class
//===----------------------------------------------------------------------===//

std::unique_ptr<FeatureModel> FeatureModelSxfmParser::buildFeatureModel() {
  UniqueXmlDoc Doc = parseDoc();
  if (!Doc || !parseVm(xmlDocGetRootElement(Doc.get()))) {
    return nullptr;
  }
  return FMB.buildFeatureModel();
}

FeatureModelSxfmParser::UniqueXmlDtd FeatureModelSxfmParser::createDtd() {
  UniqueXmlDtd Dtd(
      xmlIOParseDTD(nullptr,
                    xmlParserInputBufferCreateMem(
                        SxfmConstants::DtdRaw.c_str(),
                        SxfmConstants::DtdRaw.length(), XML_CHAR_ENCODING_UTF8),
                    XML_CHAR_ENCODING_UTF8),
      xmlFreeDtd);
  xmlCleanupParser();
  assert(Dtd && "Failed to parse DTD.");
  return Dtd;
}

FeatureModelSxfmParser::UniqueXmlDoc FeatureModelSxfmParser::parseDoc() {
  // Initialize the XML parser
  std::unique_ptr<xmlParserCtxt, void (*)(xmlParserCtxtPtr)> Ctxt(
      xmlNewParserCtxt(), xmlFreeParserCtxt);
  // Parse the given model by libxml2
  UniqueXmlDoc Doc(xmlCtxtReadMemory(Ctxt.get(), Sxfm.c_str(), Sxfm.length(),
                                     nullptr, nullptr, XML_PARSE_NOBLANKS),
                   xmlFreeDoc);
  xmlCleanupParser();

  // In the following, the document is validated.
  // Therefore, (1) check whether it could be parsed
  if (Doc && Ctxt->valid) {
    // (2) validate the sxfm format by using the dtd (document type definition)
    // file
    if (xmlValidateDtd(&Ctxt->vctxt, Doc.get(), createDtd().get())) {
      // and (3) check the tree-like structure of the embedded feature model
      // as well as constraints
      return Doc;
    }
    llvm::errs() << "Failed to validate DTD.\n";
  }
  llvm::errs() << "Failed to parse / validate XML.\n";

  return UniqueXmlDoc(nullptr, nullptr);
}

bool FeatureModelSxfmParser::parseVm(xmlNode *Node) {
  // Parse the name first.
  {
    UniqueXmlChar Cnt(xmlGetProp(Node, XmlConstants::NAME), xmlFree);
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
          UniqueXmlChar Cnt(xmlGetProp(H, SxfmConstants::INDENTATION), xmlFree);
          if (Cnt) {
            Indentation = reinterpret_cast<char *>(Cnt.get());
          }
        }

        // Parse the feature tree with all its features and relations among
        // them.
        if (!parseFeatureTree(H)) {
          return false;
        }

        // Finally, parse the cross-tree-constraints in the constraints tag.
      } else if (!xmlStrcmp(H->name, SxfmConstants::CONSTRAINTS)) {
        if (!parseConstraints(H)) {
          return false;
        }
      }
    }
  }
  return true;
}

bool FeatureModelSxfmParser::parseFeatureTree(xmlNode *FeatureTree) {
  // Split the lines of the feature tree by new lines
  {
    std::stringstream Ss(reinterpret_cast<const char *>(
        UniqueXmlChar(xmlNodeGetContent(FeatureTree), xmlFree).get()));
    std::string To;
    std::string Name;
    bool Opt;
    bool IsRoot;
    int LastIndentationLevel = -1;
    int RootIndentation = -1;
    int OrGroupCounter = 0;
    std::map<int, std::string> IndentationToParentMapping;

    // This map is used for the or group mapping
    // Each entry represents an or group as a tuple where the first value is
    // the name of the parent, the second is the relationship kind, and the
    // third a vector consisting of the name of the children
    std::map<int, std::tuple<std::string, Relationship::RelationshipKind>>
        OrGroupMapping;

    if (FeatureTree == nullptr) {
      llvm::errs() << "Failed to read in feature tree. Is it empty?\n";
      return false;
    }

    while (std::getline(Ss, To)) {
      Opt = false;
      IsRoot = false;

      if (To.empty() || std::all_of(To.begin(), To.end(), isspace)) {
        continue;
      }

      // For every line, count the indentation
      // not more than 1 additional indentations are allowed to the original one
      // However, we may have arbitrarily less indentations
      if ((To.find(':', 0)) == std::string::npos) {
        llvm::errs() << "Colon is missing in line" << To << "\n";
        return false;
      }

      llvm::StringRef ToStringRef(To);
      llvm::StringRef IndentationString =
          ToStringRef.substr(0, ToStringRef.find(':'));
      int CurrentIndentationLevel = IndentationString.count(Indentation);
      int Diff = CurrentIndentationLevel - LastIndentationLevel;

      // Remember the root indentation for later checks
      if (LastIndentationLevel == -1) {
        RootIndentation = CurrentIndentationLevel;
      }

      if ((LastIndentationLevel != -1) && Diff > 1) {
        llvm::errs() << "Indentation error in feature tree in line " << To
                     << "\n";
        return false;
      }

      // Move pointer to first character after indentation
      // The first character has to be a colon followed by the type of
      // the feature (m for mandatory, o for optional, a for alternative)
      std::string::size_type Pos =
          CurrentIndentationLevel * Indentation.length() + 2;
      std::optional<std::tuple<int, int>> Cardinalities;

      switch (To.at(Pos - 1)) {
      case 'r':
        IsRoot = true;
        break;
      case 'm':
        break;
      case 'o':
        // Code for optional
        Opt = true;
        break;
      case 'g':
        // Code for an or group with different cardinalities
        Opt = false;
        // Extract the cardinality
        Cardinalities = extractCardinality(To);
        if (!Cardinalities.has_value()) {
          return false;
        }
        break;
      case ' ':
        // Code for alternative child
        Pos--;
        break;
      default:
        llvm::errs()
            << "Wrong indentation or unsupported type of configuration option:'"
            << To << "'\n";
        return false;
      }
      // Extract the name
      Name =
          ToStringRef.substr(Pos + 1, ToStringRef.find(' ', Pos + 1) - Pos - 1)
              .str();

      // Remove the cardinality
      if (Name.find_first_of('[') != std::string::npos) {
        Name = ToStringRef
                   .substr(Pos + 1, ToStringRef.find('[', Pos + 1) - Pos - 1)
                   .str();

        if (Name.empty()) {
          // In this case, the name could also be after the cardinality.
          // According to the examples provided by S.P.L.O.T., this is a valid
          // format.
          auto Remainder =
              ToStringRef.substr(Pos + 1, ToStringRef.size() - Pos - 1);
          auto Tokens = Remainder.split(' ');
          if (!Tokens.second.empty()) {
            Name = Tokens.second.str();
          }
        }
      }

      // Note that we ignore the ID and use the name of the feature
      // as unique identifier.
      if (Name.find_first_of('(') != std::string::npos) {
        Name = Name.substr(0, Name.find('(', 0));
      }

      // Search for identifier
      std::regex Regex(R"(\([a-zA-Z\d_]*\))");
      std::string CurrentLine = ToStringRef.str();
      for (std::sregex_iterator Match = std::sregex_iterator(
               CurrentLine.begin(), CurrentLine.end(), Regex);
           Match != std::sregex_iterator(); Match++) {
        std::string Identifier = (*Match).str();
        Identifier = Identifier.substr(1, Identifier.size() - 2);
        IdentifierMap[Identifier] = Name;
        if (Identifier != Name && Name.find(Identifier) != std::string::npos) {
          llvm::errs() << "Name must not contain ID:'" << Name << "'\n";
          return false;
        }
      }

      // If there is no name, provide an artificial one
      if (Name.empty()) {
        OrGroupCounter++;
        Name = "group_" + std::to_string(OrGroupCounter);
      }

      // Create the feature
      if (IsRoot) {
        FMB.makeRoot(Name);
      } else {
        FMB.makeFeature<BinaryFeature>(Name, Opt);
      }
      IndentationToParentMapping[CurrentIndentationLevel] = Name;

      // Add parent from the upper indentation level if there is one
      if (LastIndentationLevel != -1 &&
          CurrentIndentationLevel == RootIndentation) {
        llvm::errs() << "Only one feature can be root and have the same "
                        "indentation as root.\n";
        return false;
      }

      if (LastIndentationLevel != -1) {
        auto Parent =
            IndentationToParentMapping.find(CurrentIndentationLevel - 1);
        assert(Parent != IndentationToParentMapping.end());
        FMB.addEdge(Parent->second, Name);
      }

      // Add the or group to the feature model if it is completely parsed
      auto OrGroup = OrGroupMapping.find(CurrentIndentationLevel);
      if (OrGroup != OrGroupMapping.end()) {
        FMB.emplaceRelationship(std::get<1>(OrGroup->second),
                                std::get<0>(OrGroup->second));
        OrGroupMapping.erase(CurrentIndentationLevel);
      }

      // Remember the new or group parent if there is one
      if (Cardinalities.has_value()) {
        Relationship::RelationshipKind GroupKind =
            Relationship::RelationshipKind::RK_ALTERNATIVE;
        if (std::get<1>(Cardinalities.value()) == SxfmConstants::WILDCARD) {
          GroupKind = Relationship::RelationshipKind::RK_OR;
        }
        OrGroupMapping[CurrentIndentationLevel] =
            std::tuple<std::string, Relationship::RelationshipKind>(Name,
                                                                    GroupKind);
      }

      LastIndentationLevel = CurrentIndentationLevel;
    }

    // Add the remaining or groups
    for (auto &OrGroup : OrGroupMapping) {
      FMB.emplaceRelationship(std::get<1>(OrGroup.second),
                              std::get<0>(OrGroup.second));
    }
  }

  return true;
}

bool FeatureModelSxfmParser::parseConstraints(xmlNode *Constraints) {
  if (Constraints == nullptr) {
    llvm::errs()
        << "Failed to read in constraints. Is the constraint field empty?\n";
    return false;
  }

  std::stringstream Ss(reinterpret_cast<const char *>(
      UniqueXmlChar(xmlNodeGetContent(Constraints), xmlFree).get()));
  std::string To;

  // Prepare the identifiers for later replacement
  std::vector<std::string> Keys;
  Keys.reserve(IdentifierMap.size());
  std::for_each(IdentifierMap.begin(), IdentifierMap.end(),
                [&Keys](const auto &Entry) { Keys.push_back(Entry.first); });
  std::sort(Keys.begin(), Keys.end(), std::greater<std::string>{});

  while (std::getline(Ss, To)) {
    // Ignore if a line is empty
    if (To.empty() || std::all_of(To.begin(), To.end(), isspace)) {
      continue;
    }

    // We ignore the indentation in the beginning of each line since the format
    // does not force it
    llvm::StringRef ToStringRef(To);

    if (!ToStringRef.contains(':')) {
      llvm::errs() << "Failed to read in a constraint since it does not "
                      "contain a colon.";
      llvm::errs() << "This violates the SXFM format.";
      return false;
    }

    auto Pos = ToStringRef.find(':');
    ToStringRef = ToStringRef.substr(Pos + 1, ToStringRef.size() - Pos - 1);
    std::string CnfFormula = trim(ToStringRef);

    // In the following lines, we replace all identifiers by the real feature
    // name
    for (auto &Key : Keys) {
      std::string Value = IdentifierMap[Key];
      if (Value == Key) {
        continue;
      }
      auto FoundPos = CnfFormula.find(Key);
      while (FoundPos != std::string::npos) {
        CnfFormula = CnfFormula.replace(FoundPos, Key.length(), Value);
        FoundPos = CnfFormula.find(Key);
      }
    }

    // Replace 'or' by '|' and 'and' by '&'
    std::unordered_map<std::string, std::string> SpecialKeys;
    SpecialKeys[" or "] = " | ";
    SpecialKeys[" and "] = " & ";
    for (auto &Key : SpecialKeys) {
      auto FoundPos = CnfFormula.find(Key.first);
      while (FoundPos != std::string::npos) {
        CnfFormula =
            CnfFormula.replace(FoundPos, Key.first.length(), Key.second);
        FoundPos = CnfFormula.find(Key.first);
      }
    }

    if (auto Constraint = ConstraintParser(CnfFormula).buildConstraint()) {
      FMB.addConstraint(std::move(Constraint));
    }
  }

  return true;
}

std::optional<std::tuple<int, int>> FeatureModelSxfmParser::extractCardinality(
    llvm::StringRef StringToExtractFrom) {
  std::optional<int> MinCardinality;
  std::optional<int> MaxCardinality;

  // Search for the first occurrence of '['; then read in the min cardinality
  // until the comma. Afterwards, read in the max cardinality until ']'
  std::string::size_type Pos = StringToExtractFrom.find_first_of('[');
  if (Pos == std::string::npos) {
    llvm::errs() << "No cardinality given in or group!\n";
    return std::optional<std::tuple<int, int>>();
  }
  llvm::StringRef CardinalityString(StringToExtractFrom);
  size_t CommaPos = CardinalityString.find(',', Pos + 1);
  MinCardinality = parseCardinality(
      CardinalityString.substr(Pos + 1, CommaPos - Pos - 1).str());
  Pos = CommaPos;
  MaxCardinality = parseCardinality(
      CardinalityString
          .substr(Pos + 1, CardinalityString.find(']', Pos + 1) - Pos - 1)
          .str());

  if (!MinCardinality.has_value() || !MaxCardinality.has_value()) {
    llvm::errs() << "No parsable cardinality!\n";
    return std::optional<std::tuple<int, int>>();
  }

  if (MinCardinality.value() != 1 ||
      (MaxCardinality.value() != 1 &&
       MaxCardinality.value() != SxfmConstants::WILDCARD)) {
    llvm::errs() << "Cardinality unsupported. We support cardinalities [1,1] "
                    "(alternative) or [1, *] (or group).\n";
    return std::optional<std::tuple<int, int>>();
  }

  return std::optional<std::tuple<int, int>>(
      std::tuple<int, int>{MinCardinality.value(), MaxCardinality.value()});
}

std::optional<int>
FeatureModelSxfmParser::parseCardinality(llvm::StringRef CardinalityString) {
  std::optional<int> Result = std::optional<int>();
  if (CardinalityString == "*") {
    // We use -1 as our magic integer to indicate that the cardinality is a
    // wildcard.
    Result = SxfmConstants::WILDCARD;
  } else {
    // Convert the string into an integer in a safe way
    long LongNumber;
    if (!llvm::to_integer(CardinalityString, LongNumber, 10)) {
      llvm::errs() << llvm::formatv(
          "The cardinality: '{0}' was not an integer.\n", CardinalityString);
    } else {
      Result = LongNumber;
    }
  }

  return Result;
}

} // namespace vara::feature
