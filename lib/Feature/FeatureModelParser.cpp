#include "vara/Feature/FeatureModelParser.h"

#include "SxfmConstants.h"
#include "XmlConstants.h"

#include <iostream>
#include <regex>
#include <utility>

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
              FMB.addConstraint(std::make_unique<ImpliesConstraint>(
                  std::make_unique<PrimaryFeatureConstraint>(
                      std::make_unique<Feature>(Name)),
                  std::make_unique<NotConstraint>(
                      std::make_unique<PrimaryFeatureConstraint>(
                          std::make_unique<Feature>(
                              reinterpret_cast<char *>(CCnt.get()))))));
            }
          }
        }
      } else if (!xmlStrcmp(Head->name, XmlConstants::IMPLIEDOPTIONS)) {
        for (xmlNode *Child = Head->children; Child; Child = Child->next) {
          if (Child->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(Child->name, XmlConstants::OPTIONS)) {
              std::unique_ptr<xmlChar, void (*)(void *)> CCnt(
                  xmlNodeGetContent(Child), xmlFree);
              FMB.addConstraint(std::make_unique<ImpliesConstraint>(
                  std::make_unique<PrimaryFeatureConstraint>(
                      std::make_unique<Feature>(Name)),
                  std::make_unique<PrimaryFeatureConstraint>(
                      std::make_unique<Feature>(
                          reinterpret_cast<char *>(CCnt.get())))));
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

// TODO(s9latimm): Refactor with new Constraints representation
// TODO(s9latimm): remove NOLINT
bool FeatureModelXmlParser::parseConstraints(xmlNode *Node) { // NOLINT
  for (xmlNode *H = Node->children; H; H = H->next) {
    if (H->type == XML_ELEMENT_NODE) {
      if (!xmlStrcmp(H->name, XmlConstants::CONSTRAINT)) {
        string Cnt = std::string(
            reinterpret_cast<char *>(std::unique_ptr<xmlChar, void (*)(void *)>(
                                         xmlNodeGetContent(H), xmlFree)
                                         .get()));
        const std::regex Regex(R"((!?\w+))");
        std::smatch Matches;
        //        FeatureModel::ConstraintTy Constraint;
        //        for (string Suffix = Cnt; regex_search(Suffix, Matches,
        //        Regex);
        //             Suffix = Matches.suffix()) {
        //          string B = Matches.str(0);
        //          if (B.length() > 1 && B[0] == '!') {
        //            Constraint.emplace_back(B.substr(1, B.length()), false);
        //          } else {
        //            Constraint.emplace_back(B, true);
        //          }
        //        }
        //        FMB.addConstraint(std::move(Constraint));
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
//                        FeatureModelSxfmParser Class
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

bool FeatureModelSxfmParser::parseFeatureTree(xmlChar *FeatureTree) {
  // Split the lines of the feature tree by new lines
  {
    std::stringstream Ss(reinterpret_cast<const char *>(FeatureTree));
    std::string To;
    string Name;
    bool Opt;
    int LastIndentationLevel = -1;
    int OrGroupCounter = 0;
    std::map<int, string> IndentationToParentMapping;

    if (FeatureTree == nullptr) {
      std::cerr << "Failed to read in feature tree. Is it empty?" << std::endl;
      return false;
    }

    while (std::getline(Ss, To)) {
      if (!std::getline(Ss, To)) {
        return true;
      }
      Opt = false;

      if (To.empty() || std::all_of(To.begin(),To.end(),isspace)) {
        continue;
      }

      // For every line, count the indentation
      // not more than 1 additional indentations are allowed to the original one
      // However, we may have arbitrarily less indentations
      int CurrentIndentationLevel = countOccurrences(To, Indentation);
      int Diff = CurrentIndentationLevel - LastIndentationLevel;
      if ((LastIndentationLevel != -1) && Diff > 1) {
        std::cerr << "Indentation error in feature tree in line " << To << std::endl;
        return false;
      }

      // Move pointer to first character after indentation
      // The first character has to be a colon followed by the type of
      // the feature (m for mandatory, o for optional, a for alternative)
      std::string::size_type Pos = CurrentIndentationLevel * Indentation.length() + 2;
      std::optional<std::tuple<int, int>> Cardinalities;
      if (To.at(Pos) != ':') {
        std::cerr << "Colon is missing in line" << To << std::endl;
      }

      switch (To.at(Pos - 1)) {
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
      }
      // Extract the name
      Name = readUntil(To, ' ', Pos + 1);

      // Note that we ignore the ID and use the name of the feature
      // as unique identifier.
      if (Name.find_first_of('(') != std::string::npos) {
        Name = readUntil(Name, '(', 0);
      }

      // Remove the cardinality
      if (Name.find_first_of('[') != std::string::npos) {
        Name = readUntil(Name, '[', 0);
      }

      // If there is no name, provide an artificial one
      if (Name.empty()) {
        OrGroupCounter++;
        Name = "group_" + std::to_string(OrGroupCounter);
      }

      // Create the feature
      FMB.makeFeature<BinaryFeature>(Name, Opt);
      IndentationToParentMapping[CurrentIndentationLevel] = Name;

      // Add parent from the upper indentation level if there is one
      if (LastIndentationLevel != -1) {
        auto Parent = IndentationToParentMapping.find(CurrentIndentationLevel - 1);
        assert(Parent != IndentationToParentMapping.end());
        FMB.addParent(Name, Parent->second);
      }
      LastIndentationLevel = CurrentIndentationLevel;
    }
  }

  return true;
}

/// This method extracts the cardinality from the given line.
/// The cardinality is wrapped in square brackets (e.g., [1,1])
/// \param StringToExtractFrom the string to extract the cardinality from
/// \return
std::optional<std::tuple<int, int>> FeatureModelSxfmParser::extractCardinality(const string& StringToExtractFrom) {
  std::optional<int> MinCardinality;
  std::optional<int> MaxCardinality;

  // Search for the first occurrence of '['; then read in the min cardinality
  // until the comma. Afterwards, read in the max cardinality until ']'
  std::string::size_type Pos = StringToExtractFrom.find_first_of('[');
  if (Pos == std::string::npos) {
    std::cerr << "No cardinality given in or group!" << std::endl;
    return std::optional<std::tuple<int, int>>();
  }
  MinCardinality = parseCardinality(readUntil(StringToExtractFrom, ',', Pos + 1));
  Pos = StringToExtractFrom.find_first_of(',', Pos);
  MaxCardinality = parseCardinality(readUntil(StringToExtractFrom, ']', Pos + 1));

  if (!MinCardinality.has_value() || !MaxCardinality.has_value()) {
    std::cerr << "No parsable cardinality!" << std::endl;
    return std::optional<std::tuple<int, int>>();
  }

  if (MinCardinality.value() != 1 || MaxCardinality.value() != 1 || MaxCardinality.value() != UINT_MAX) {
    std::cerr << "Cardinality unsupported. We support cardinalities [1,1] (alternative) or [1, *] (or group)." << std::endl;
    return std::optional<std::tuple<int, int>>();
  }

  return std::optional<std::tuple<int, int>>(std::tuple<int, int>{MinCardinality.value(), MaxCardinality.value()});
}


/// This method parses the given cardinality and returns an optional.
/// If the optional is empty, the process failed; otherwise the result contains
/// either UINT_MAX for the wildcard or the cardinality number as integer.
/// \param CardinalityString the cardinality to parse
/// \return an optional that contains no integer in case of failure or UINT_MAX
/// for wildcard, or the number itself.
std::optional<int> FeatureModelSxfmParser::parseCardinality(const string& CardinalityString) {
  std::optional<int> Result = std::optional<int>();
  if (CardinalityString == "*") {
    // We use UINT_MAX as our magic integer (which is -1 as int) to indicate
    // that the cardinality is a wildcard.
    Result = UINT_MAX;
  } else {
    // Convert the string into an integer in a safe way
    char *End;
    long LongNumber;
    errno = 0;
    LongNumber = strtol(CardinalityString.c_str(), &End, 0);
    if (errno == ERANGE || LongNumber < INT_MIN || LongNumber > INT_MAX
        || *CardinalityString.c_str() == '\0' || *End != '\0') {
      std::cerr << "The following cardinality is not integer: " << CardinalityString << std::endl;
    } else {
      Result = LongNumber;
    }
  }

  return Result;
}

/// This method reads beginning from a certain starting position and reads in
/// every character until CharToSearch is found.
/// \param StringToReadFrom string to read from
/// \param CharToSearch character to search for
/// \param Start the starting position to start reading
/// \return the string read until the given character is found
string FeatureModelSxfmParser::readUntil(const string& StringToReadFrom, const char& CharToSearch, std::string::size_type Start) {
  string Result;
  std::string::size_type Pos = Start;
  if (Pos > StringToReadFrom.length()) {
    std::cerr << "Position out of bounds!" << std::endl;
    return "";
  }

  while ((Pos < StringToReadFrom.length()) && (StringToReadFrom.at(Pos) != CharToSearch)) {
    Result += StringToReadFrom.at(Pos);
    Pos++;
  }
  return Result;
}

/// This method counts the occurrences of the second argument in the first argument.
/// The intended use of this method is to count the indentation level of the feature
/// tree in the sxfm format.
/// \param StringToSearch the string to search in
/// \param StringToFind the string to count the occurrences of
/// \return the number of occurrences
int FeatureModelSxfmParser::countOccurrences(const string& StringToSearch, const string& StringToFind) {
  int Occurrences = 0;
  std::string::size_type Pos = 0;
  while ((Pos = StringToSearch.find(StringToFind, Pos )) != std::string::npos) {
  ++Occurrences;
  Pos += StringToFind.length();
  }
  return Occurrences;
}

bool FeatureModelSxfmParser::parseConstraints(xmlChar *Constraints) {
  // TODO (CK): This has to wait until the constraint part is implemented in
  // the FeatureModelBuilder (issue #24)
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
          std::unique_ptr<xmlChar, void (*)(void *)> Cnt(xmlGetProp(H, SxfmConstants::INDENTATION),
                                                         xmlFree);
          if (Cnt) {
            Indentation = reinterpret_cast<char*>(Cnt.get());
          }

        }
        if (!parseFeatureTree(xmlNodeGetContent(H))) {
          return false;
        }
      } else if (!xmlStrcmp(H->name, SxfmConstants::CONSTRAINTS)) {
        if (!parseConstraints(xmlNodeGetContent(H))) {
          return false;
        }
      }
    }
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

/// This method checks if the given feature model is valid
/// \return true iff the feature model is valid
bool FeatureModelSxfmParser::verifyFeatureModel() { return parseDoc().get(); }

} // namespace vara::feature
