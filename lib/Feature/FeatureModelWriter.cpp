#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/OrderedFeatureVector.h"

#include <llvm/Support/Casting.h>

#include "XmlConstants.h"

#include "libxml/xmlwriter.h"

#define CHECK_RC                                                               \
  if (RC < 0) {                                                                \
    return RC;                                                                 \
  }

namespace vara::feature {

const unsigned char *charToUChar(const char *CP) {
  return (const unsigned char *)CP;
}

static constexpr xmlChar INDENTATIONSTRING[] = "  ";
static constexpr char ENCODING[] = "UTF-8";

int FeatureModelXmlWriter::writeFeatureModel(std::string Path) {
  int RC;
  std::unique_ptr<xmlTextWriter, void (*)(xmlTextWriterPtr)> Writer(
      xmlNewTextWriterFilename(Path.data(), 0), &xmlFreeTextWriter);
  if (!Writer) {
    return false;
  }

  RC = writeFeatureModel(Writer.get());
  return RC;
}

std::optional<std::string> FeatureModelXmlWriter::writeFeatureModel() {
  int RC;

  std::unique_ptr<xmlDocPtr, void (*)(xmlDocPtr *)> DocPtrPtr(
      new xmlDocPtr{nullptr}, [](xmlDocPtr *Ptr) {
        if (*Ptr) {
          xmlFreeDoc(*Ptr);
        }
        delete (Ptr);
      });

  std::unique_ptr<xmlTextWriter, void (*)(xmlTextWriterPtr)> Writer(
      xmlNewTextWriterDoc(DocPtrPtr.get(), 0), &xmlFreeTextWriter);
  if (!Writer) {
    return std::nullopt;
  }
  RC = writeFeatureModel(Writer.get());
  if (RC < 0) {
    return std::nullopt;
  }

  std::unique_ptr<xmlChar *, void (*)(xmlChar **)> XmlBuffPtr(
      new xmlChar *{nullptr}, [](xmlChar **Ptr) {
        if (*Ptr) {
          xmlFree(*Ptr);
        }
        delete (Ptr);
      });
  int Buffersize;
  xmlDocDumpMemoryEnc(*DocPtrPtr, XmlBuffPtr.get(), &Buffersize, ENCODING);
  std::string Str(reinterpret_cast<char *>(*XmlBuffPtr), Buffersize);

  return Str;
}

int FeatureModelXmlWriter::writeFeatureModel(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterSetIndent(Writer, 1);
  CHECK_RC
  RC = xmlTextWriterSetIndentString(Writer, INDENTATIONSTRING);
  CHECK_RC

  RC = xmlTextWriterStartDocument(Writer, "1.0", ENCODING, nullptr);
  CHECK_RC
  RC = xmlTextWriterWriteDTD(Writer, charToUChar("vm"), nullptr,
                             charToUChar("vm.dtd"), nullptr);
  CHECK_RC
  RC = writeVm(Writer);

  return RC;
}

int FeatureModelXmlWriter::writeVm(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::VM);
  CHECK_RC
  RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::NAME,
                                   charToUChar(Fm.getName().data()));
  CHECK_RC
  RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::ROOT,
                                   charToUChar(Fm.getPath().string().data()));
  CHECK_RC
  if (!Fm.getCommit().empty()) {
    RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::COMMIT,
                                     charToUChar(Fm.getCommit().data()));
  }
  CHECK_RC
  RC = writeBinaryFeatures(Writer);
  CHECK_RC

  RC = writeNumericFeatures(Writer);
  CHECK_RC

  RC = writeBooleanConstraints(Writer);
  CHECK_RC

  // TODO mixed and nonNumeric constraints when supported

  RC = xmlTextWriterEndDocument(Writer); // VM
  return RC;
}

int FeatureModelXmlWriter::writeBinaryFeatures(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::BINARYOPTIONS);
  CHECK_RC

  for (Feature *F : Fm.features()) {
    if (llvm::isa<RootFeature>(F) || llvm::isa<BinaryFeature>(F)) {
      RC = writeFeature(Writer, *F);
      CHECK_RC
    }
  }

  RC = xmlTextWriterEndElement(Writer); // BINARYOPTIONS
  return RC;
}

int FeatureModelXmlWriter::writeNumericFeatures(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::NUMERICOPTIONS);
  CHECK_RC

  for (auto *F : Fm.features()) {
    if (llvm::isa<NumericFeature>(F)) {
      RC = writeFeature(Writer, *F);
      CHECK_RC
    }
  }

  RC = xmlTextWriterEndElement(Writer); // NUMERICOPTIONS
  return RC;
}

int FeatureModelXmlWriter::writeBooleanConstraints( // NOLINT
    xmlTextWriterPtr Writer) {                      // NOLINT
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::BOOLEANCONSTRAINTS);
  CHECK_RC

  // collect alternative groups
  struct FeatureCompare {
    bool operator()(Feature *F1, Feature *F2) const { return *F1 < *F2; }
  };
  std::set<std::set<Feature *, FeatureCompare>> AltGroups;
  //  for (auto *F : Fm.features()) {
  //    // skip empty groups
  //    if (F->alternatives().begin() == F->alternatives().end()) {
  //      continue;
  //    }
  //    std::set<Feature *, FeatureCompare> AltGroup(F->alternatives_begin(),
  //                                                 F->alternatives_end());
  //    AltGroup.insert(F);
  //    AltGroups.insert(std::move(AltGroup));
  //  }

  for (const auto &Group : AltGroups) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::CONSTRAINT);
    CHECK_RC

    auto F = Group.begin();
    RC = xmlTextWriterWriteString(Writer, charToUChar((*F)->getName().data()));
    CHECK_RC
    ++F;
    for (; F != Group.end(); ++F) {
      RC = xmlTextWriterWriteString(Writer, charToUChar(" | "));
      CHECK_RC
      RC =
          xmlTextWriterWriteString(Writer, charToUChar((*F)->getName().data()));
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // CONSTRAINT
    CHECK_RC
  }

  for (const auto &Constraint : Fm.constraints()) {
    if (auto *Implies = llvm::dyn_cast<ImpliesConstraint>(Constraint.get())) {
      if (llvm::isa_and_nonnull<PrimaryFeatureConstraint>(
              Implies->getLeftOperand()) &&
          llvm::isa_and_nonnull<PrimaryFeatureConstraint>(
              Implies->getRightOperand())) {
        // already covered by implied options
        continue;
      }
    }
    if (auto *Excludes = llvm::dyn_cast<ExcludesConstraint>(Constraint.get())) {
      if (llvm::isa_and_nonnull<PrimaryFeatureConstraint>(
              Excludes->getLeftOperand()) &&
          llvm::isa_and_nonnull<PrimaryFeatureConstraint>(
              Excludes->getRightOperand())) {
        // already covered by excluded options
        continue;
      }
    }

    RC = xmlTextWriterStartElement(Writer, XmlConstants::CONSTRAINT);
    CHECK_RC
    RC = xmlTextWriterWriteString(Writer,
                                  charToUChar(Constraint->toString().data()));
    CHECK_RC
    RC = xmlTextWriterEndElement(Writer); // CONSTRAINT
    CHECK_RC
  }

  RC = xmlTextWriterEndElement(Writer); // BOOLEANCONSTRAINT
  return RC;
}

int FeatureModelXmlWriter::writeFeature(xmlTextWriterPtr Writer,
                                        Feature &Feature1) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::CONFIGURATIONOPTION);
  CHECK_RC

  RC = xmlTextWriterWriteElement(Writer, XmlConstants::NAME,
                                 charToUChar(Feature1.getName().data()));
  CHECK_RC

  // parent
  if (Feature1.getParentFeature()) {
    RC = xmlTextWriterWriteElement(
        Writer, XmlConstants::PARENT,
        charToUChar(Feature1.getParentFeature()->getName().data()));
    CHECK_RC
  }

  // children
  auto FS = Feature1.getChildren<Feature>();
  auto Children = deprecated::OrderedFeatureVector(FS.begin(), FS.end());

  if (!Children.empty()) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::CHILDREN);
    CHECK_RC

    for (Feature *F : Children) {
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                     charToUChar(F->getName().data()));
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // CHILDREN
    CHECK_RC
  }

  // implications
  deprecated::OrderedFeatureVector Implications;

  for (const auto *C : Feature1.implications()) {
    if (const auto *LHS =
            llvm::dyn_cast<PrimaryFeatureConstraint>(C->getLeftOperand());
        LHS) {
      if (const auto *RHS =
              llvm::dyn_cast<PrimaryFeatureConstraint>(C->getRightOperand());
          RHS) {
        if (LHS->getFeature() &&
            LHS->getFeature()->getName() == Feature1.getName() &&
            RHS->getFeature()) {
          Implications.insert(RHS->getFeature());
        }
      }
    }
  }

  if (!Implications.empty()) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::IMPLIEDOPTIONS);
    CHECK_RC

    for (Feature *F : Implications) {
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                     charToUChar(F->getName().data()));
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // IMPLIEDOPTIONS
    CHECK_RC
  }

  // excludes
  deprecated::OrderedFeatureVector Excludes;

  if (llvm::isa_and_nonnull<Relationship>(Feature1.getParent())) {
    auto ES = Feature1.getParent()->getChildren<Feature>();
    ES.erase(&Feature1);
    Excludes.insert(ES.begin(), ES.end());
  }
  for (const auto *C : Feature1.excludes()) {
    if (const auto *LHS =
            llvm::dyn_cast<PrimaryFeatureConstraint>(C->getLeftOperand());
        LHS) {
      if (const auto *RHS =
              llvm::dyn_cast<PrimaryFeatureConstraint>(C->getRightOperand());
          RHS) {
        if (LHS->getFeature() &&
            LHS->getFeature()->getName() == Feature1.getName() &&
            RHS->getFeature()) {
          if (std::find(Excludes.begin(), Excludes.end(), RHS->getFeature()) ==
              Excludes.end()) {
            Excludes.insert(RHS->getFeature());
          }
        }
      }
    }
  }

  if (!Excludes.empty()) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::EXCLUDEDOPTIONS);
    CHECK_RC

    for (Feature *F : Excludes) {
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                     charToUChar(F->getName().data()));
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // EXCLUDEDOPTIONS
    CHECK_RC
  }

  // optional
  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::OPTIONAL,
      charToUChar((Feature1.isOptional() ? "True" : "False")));
  CHECK_RC

  // numeric elements
  if (auto *NF = llvm::dyn_cast<NumericFeature>(&Feature1)) {
    auto ValueVariant = NF->getValues();
    if (std::holds_alternative<std::pair<int, int>>(ValueVariant)) {
      auto [Min, Max] = std::get<std::pair<int, int>>(ValueVariant);
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::MINVALUE,
                                     charToUChar(std::to_string(Min).data()));
      CHECK_RC

      RC = xmlTextWriterWriteElement(Writer, XmlConstants::MAXVALUE,
                                     charToUChar(std::to_string(Max).data()));
      CHECK_RC
    } else {
      auto Values = std::get<std::vector<int>>(ValueVariant);
      std::string Str;
      std::for_each(std::begin(Values), std::end(Values) - 1, [&Str](int X) {
        Str.append(std::to_string(X));
        Str.push_back(';');
      });
      Str.append(std::to_string(Values.back()));

      RC = xmlTextWriterWriteElement(Writer, XmlConstants::VALUES,
                                     charToUChar(Str.data()));
      CHECK_RC
    }
  }

  // locations?
  if (Feature1.hasLocations()) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::LOCATIONS);
    CHECK_RC
    for (FeatureSourceRange &Fsr : Feature1.getLocations()) {
      RC = writeSourceRange(Writer, Fsr);
      CHECK_RC
    }
    RC = xmlTextWriterEndElement(Writer); // LOCATIONS
    CHECK_RC
  }

  RC = xmlTextWriterEndElement(Writer); // CONFIGURATIONOPTION
  return RC;
}

int FeatureModelXmlWriter::writeSourceRange(xmlTextWriterPtr Writer,
                                            FeatureSourceRange &Location) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::SOURCERANGE);
  CHECK_RC

  switch (Location.getCategory()) {
  case FeatureSourceRange::Category::necessary:
    RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::CATEGORY,
                                     XmlConstants::NECESSARY);
    break;
  case FeatureSourceRange::Category::inessential:
    RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::CATEGORY,
                                     XmlConstants::INESSENTIAL);
    break;
  }
  CHECK_RC

  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::PATH,
      charToUChar(Location.getPath().string().data()));
  CHECK_RC

  RC = xmlTextWriterStartElement(Writer, XmlConstants::START);
  CHECK_RC

  auto *Start = Location.getStart();
  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::LINE,
      BAD_CAST std::to_string(Start->getLineNumber()).data());
  CHECK_RC

  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN,
      BAD_CAST std::to_string(Start->getColumnOffset()).data());
  CHECK_RC

  RC = xmlTextWriterEndElement(Writer); // START
  CHECK_RC

  RC = xmlTextWriterStartElement(Writer, XmlConstants::END);
  CHECK_RC

  auto *End = Location.getEnd();
  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::LINE,
      BAD_CAST std::to_string(End->getLineNumber()).data());
  CHECK_RC

  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN,
      BAD_CAST std::to_string(End->getColumnOffset()).data());
  CHECK_RC

  RC = xmlTextWriterEndElement(Writer); // END
  CHECK_RC

  RC = xmlTextWriterEndElement(Writer); // SOURCERANGE
  return RC;
}

} // namespace vara::feature
