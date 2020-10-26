#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModel.h"

#include <llvm/Support/Casting.h>

#include "XmlConstants.h"

#include "libxml/xmlwriter.h"

#define CHECK_RC                                                               \
  if (RC < 0) {                                                                \
    return RC;                                                                 \
  }

namespace vara::feature {

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
  RC = writeVm(Writer);

  return RC;
}

int FeatureModelXmlWriter::writeVm(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::VM);
  CHECK_RC
  RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::NAME,
                                   BAD_CAST Fm.getName().data());
  CHECK_RC
  RC = xmlTextWriterWriteAttribute(Writer, XmlConstants::ROOT,
                                   BAD_CAST Fm.getPath().string().data());
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
    if (llvm::isa<BinaryFeature>(F)) {
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

int FeatureModelXmlWriter::writeBooleanConstraints(xmlTextWriterPtr Writer) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::BOOLEANCONSTRAINTS);
  CHECK_RC

  // collect alternative groups
  struct FeatureCompare {
    bool operator()(Feature *F1, Feature *F2) const { return *F1 < *F2; }
  };
  std::set<std::set<Feature *, FeatureCompare>> AltGroups;
  // TODO(s9latimm): Refactor with new Constraints representation
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
    RC = xmlTextWriterWriteString(Writer, BAD_CAST(*F)->getName().data());
    CHECK_RC
    ++F;
    for (; F != Group.end(); ++F) {
      RC = xmlTextWriterWriteString(Writer, BAD_CAST " | ");
      CHECK_RC
      RC = xmlTextWriterWriteString(Writer, BAD_CAST(*F)->getName().data());
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // CONSTRAINT
    CHECK_RC
  }

  // TODO write other boolean constraints if they are parsed

  RC = xmlTextWriterEndElement(Writer); // BOOLEANCONSTRAINT
  return RC;
}

int FeatureModelXmlWriter::writeFeature(xmlTextWriterPtr Writer,
                                        Feature &Feature1) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::CONFIGURATIONOPTION);
  CHECK_RC

  RC = xmlTextWriterWriteElement(Writer, XmlConstants::NAME,
                                 BAD_CAST Feature1.getName().data());
  CHECK_RC

  // parent
  if (!Feature1.isRoot()) {
    RC = xmlTextWriterWriteElement(
        Writer, XmlConstants::PARENT,
        BAD_CAST Feature1.getParentFeature()->getName().data());
    CHECK_RC
  }

  // children
  if (Feature1.begin() != Feature1.end()) {
    RC = xmlTextWriterStartElement(Writer, XmlConstants::CHILDREN);
    CHECK_RC

    OrderedFeatureVector Children{Feature1.begin(), Feature1.end()};
    for (Feature *F : Children) {
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                     BAD_CAST F->getName().data());
      CHECK_RC
    }

    RC = xmlTextWriterEndElement(Writer); // CHILDREN
    CHECK_RC
  }

  // TODO(s9latimm): Refactor with new Constraints representation
  // implications
  //  if (Feature1.implications_begin() != Feature1.implications_end()) {
  //    RC = xmlTextWriterStartElement(Writer, XmlConstants::IMPLIEDOPTIONS);
  //    CHECK_RC
  //
  //    OrderedFeatureVector Implies{Feature1.implications_begin(),
  //                                 Feature1.implications_end()};
  //    for (Feature *F : Implies) {
  //      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
  //                                     BAD_CAST F->getName().data());
  //      CHECK_RC
  //    }
  //
  //    RC = xmlTextWriterEndElement(Writer); // IMPLIEDOPTIONS
  //    CHECK_RC
  //  }

  // TODO(s9latimm): Refactor with new Constraints representation
  // excludes
  //  if (Feature1.excludes_begin() != Feature1.excludes_end()) {
  //    RC = xmlTextWriterStartElement(Writer, XmlConstants::EXCLUDEDOPTIONS);
  //    CHECK_RC
  //
  //    OrderedFeatureVector Exludes{Feature1.excludes_begin(),
  //                                 Feature1.excludes_end()};
  //    for (Feature *F : Exludes) {
  //      RC = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
  //                                     BAD_CAST F->getName().data());
  //      CHECK_RC
  //    }
  //
  //    RC = xmlTextWriterEndElement(Writer); // EXCLUDEDOPTIONS
  //    CHECK_RC
  //  }

  // optional
  RC = xmlTextWriterWriteElement(
      Writer, XmlConstants::OPTIONAL,
      BAD_CAST(Feature1.isOptional() ? "True" : "False"));
  CHECK_RC

  // numeric elements
  if (auto *NF = llvm::dyn_cast<NumericFeature>(&Feature1)) {
    auto ValueVariant = NF->getValues();
    if (std::holds_alternative<std::pair<int, int>>(ValueVariant)) {
      auto [Min, Max] = std::get<std::pair<int, int>>(ValueVariant);
      RC = xmlTextWriterWriteElement(Writer, XmlConstants::MINVALUE,
                                     BAD_CAST std::to_string(Min).data());
      CHECK_RC

      RC = xmlTextWriterWriteElement(Writer, XmlConstants::MAXVALUE,
                                     BAD_CAST std::to_string(Max).data());
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
                                     BAD_CAST Str.data());
      CHECK_RC
    }
  }

  // location?
  if (FeatureSourceRange *Fsr = Feature1.getFeatureSourceRange()) {
    RC = writeSourceRange(Writer, *Fsr);
    CHECK_RC
  }

  RC = xmlTextWriterEndElement(Writer); // CONFIGURATIONOPTION
  return RC;
}

int FeatureModelXmlWriter::writeSourceRange(xmlTextWriterPtr Writer,
                                            FeatureSourceRange &Location) {
  int RC;

  RC = xmlTextWriterStartElement(Writer, XmlConstants::LOCATION);
  CHECK_RC

  RC = xmlTextWriterWriteElement(Writer, XmlConstants::PATH,
                                 BAD_CAST Location.getPath().string().data());
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

  RC = xmlTextWriterEndElement(Writer); // LOCATION
  return RC;
}

} // namespace vara::feature
