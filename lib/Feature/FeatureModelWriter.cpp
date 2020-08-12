#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModel.h"
#include <llvm/Support/Errc.h>

#include "XmlConstants.h"

#include "libxml/xmlwriter.h"

#define CHECK_RC                                                               \
  if (Rc < 0) {                                                                \
    return Rc;                                                                 \
  }

namespace vara::feature {

static constexpr xmlChar INDENTATIONSTRING[] = "  ";
static constexpr char ENCODING[] = "UTF-8";

int FeatureModelXmlWriter::writeFeatureModel(std::string Path) {
  int Rc;
  xmlTextWriterPtr Writer;

  Writer = xmlNewTextWriterFilename(Path.data(), 0);
  if (Writer == nullptr) {
    return false;
  }

  Rc = writeFeatureModel(Writer);
  xmlFreeTextWriter(Writer);
  return Rc;
}

std::optional<std::string> FeatureModelXmlWriter::writeFeatureModel() {
  int Rc;
  xmlTextWriterPtr Writer;
  xmlDocPtr Doc;

  Writer = xmlNewTextWriterDoc(&Doc, 0);
  if (Writer == nullptr) {
    return std::nullopt;
  }
  Rc = writeFeatureModel(Writer);
  if (Rc < 0) {
    return std::nullopt;
  }

  xmlChar *xmlbuff;
  int buffersize;
  xmlDocDumpMemoryEnc(Doc, &xmlbuff, &buffersize, ENCODING);

  std::string Str((char *)xmlbuff, buffersize);

  xmlFreeTextWriter(Writer);
  xmlFreeDoc(Doc);
  return Str;
}

int FeatureModelXmlWriter::writeFeatureModel(xmlTextWriterPtr Writer) {
  int Rc;

  Rc = xmlTextWriterSetIndent(Writer, 1);
  CHECK_RC
  Rc = xmlTextWriterSetIndentString(Writer, INDENTATIONSTRING);
  CHECK_RC

  Rc = xmlTextWriterStartDocument(Writer, "1.0", ENCODING, nullptr);
  CHECK_RC
  Rc = writeVm(Writer);
  // report failure/success
  return Rc;
}

int FeatureModelXmlWriter::writeVm(xmlTextWriterPtr Writer) {
  int Rc;
  // open vm node
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::VM);
  CHECK_RC
  Rc = xmlTextWriterWriteAttribute(Writer, XmlConstants::NAME,
                                   BAD_CAST Fm.getName().data());
  CHECK_RC
  Rc = xmlTextWriterWriteAttribute(Writer, XmlConstants::ROOT,
                                   BAD_CAST Fm.getPath().string().data());
  CHECK_RC

  // write BinaryFeatures
  Rc = writeBinaryFeatures(Writer);
  CHECK_RC

  // write NumericFeatures
  Rc = writeNumericFeatures(Writer);
  CHECK_RC

  // write Constraints
  Rc = writeBooleanConstraints(Writer);
  CHECK_RC

  Rc = xmlTextWriterEndDocument(Writer); // VM
  return Rc;
}

int FeatureModelXmlWriter::writeBinaryFeatures(xmlTextWriterPtr Writer) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::BINARYOPTIONS);
  CHECK_RC
  for (Feature *F : Fm.features()) {
    if (F->getKind() == Feature::FeatureKind::FK_BINARY) {
      Rc = writeFeature(Writer, *F);
      CHECK_RC
    }
  }

  Rc = xmlTextWriterEndElement(Writer); // BINARYOPTIONS
  return Rc;
}

int FeatureModelXmlWriter::writeNumericFeatures(xmlTextWriterPtr Writer) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::NUMERICOPTIONS);
  CHECK_RC
  for (auto *F : Fm.features()) {
    if (F->getKind() == Feature::FeatureKind::FK_NUMERIC) {
      Rc = writeFeature(Writer, *F);
      CHECK_RC
    }
  }

  Rc = xmlTextWriterEndElement(Writer); // NUMERICOPTIONS
  return Rc;
}

int FeatureModelXmlWriter::writeBooleanConstraints(xmlTextWriterPtr Writer) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::BOOLEANCONSTRAINTS);
  CHECK_RC

  // collect alternative groups
  std::set<std::set<Feature *>> AltGroups;
  for (auto *F : Fm.features()) {
    if (F->alternatives().empty()) {
      continue;
    }
    std::set<Feature *> AltGroup(F->alternatives().begin(),
                                 F->alternatives().end());
    AltGroup.insert(F);
    AltGroups.insert(AltGroup);
  };

  for (auto Group : AltGroups) {
    Rc = xmlTextWriterStartElement(Writer, XmlConstants::CONSTRAINT);
    CHECK_RC

    auto F = Group.begin();
    Rc = xmlTextWriterWriteString(Writer, BAD_CAST (*F)->getName().data());
    CHECK_RC
    for (; F != Group.end(); ++F) {
      Rc = xmlTextWriterWriteString(Writer, BAD_CAST " || ");
      CHECK_RC
      Rc = xmlTextWriterWriteString(Writer, BAD_CAST (*F)->getName().data());
      CHECK_RC
    }

    Rc = xmlTextWriterEndElement(Writer); // CONSTRAINT
    CHECK_RC
  }

  // TODO write boolean constraints if they are parsed

  Rc = xmlTextWriterEndElement(Writer); // BOOLEANCONSTRAINT
  return Rc;
}

int FeatureModelXmlWriter::writeFeature(xmlTextWriterPtr Writer,
                                        Feature &Feature1) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::CONFIGURATIONOPTION);
  CHECK_RC
  Rc = xmlTextWriterWriteElement(Writer, XmlConstants::NAME,
                                 BAD_CAST Feature1.getName().data());
  CHECK_RC

  // optional
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::OPTIONAL,
      BAD_CAST(Feature1.isOptional() ? "True" : "False"));
  CHECK_RC

  // parent
  if (not Feature1.isRoot()) {
    Rc = xmlTextWriterWriteElement(
        Writer, XmlConstants::PARENT,
        BAD_CAST Feature1.getParent()->getName().data());
    CHECK_RC
  }

  // children
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::CHILDREN);
  CHECK_RC
  for (Feature *F : Feature1.children()) {
    Rc = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                   BAD_CAST F->getName().data());
    CHECK_RC
  }
  Rc = xmlTextWriterEndElement(Writer); // CHILDREN
  CHECK_RC

  // exclude
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::EXCLUDEDOPTIONS);
  CHECK_RC
  for (Feature *F : Feature1.excludes()) {
    Rc = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                   BAD_CAST F->getName().data());
    CHECK_RC
  }
  Rc = xmlTextWriterEndElement(Writer); // EXCLUDEDOPTIONS
  CHECK_RC

  // implications
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::IMPLIEDOPTIONS);
  CHECK_RC
  for (Feature *F : Feature1.implications()) {
    Rc = xmlTextWriterWriteElement(Writer, XmlConstants::OPTIONS,
                                   BAD_CAST F->getName().data());
    CHECK_RC
  }
  Rc = xmlTextWriterEndElement(Writer); // IMPLIEDOPTIONS
  CHECK_RC

  // if numeric
  if (NumericFeature::classof(&Feature1)) {
    auto &Nf = dynamic_cast<NumericFeature &>(Feature1);
    auto ValueVariant = Nf.getValues();
    if (std::holds_alternative<std::pair<int, int>>(ValueVariant)) {
      auto [Min, Max] = std::get<std::pair<int, int>>(ValueVariant);
      Rc = xmlTextWriterWriteElement(Writer, XmlConstants::MINVALUE,
                                     BAD_CAST std::to_string(Min).data());
      CHECK_RC
      Rc = xmlTextWriterWriteElement(Writer, XmlConstants::MAXVALUE,
                                     BAD_CAST std::to_string(Max).data());
      CHECK_RC
    } else {
      auto Values = std::get<std::vector<int>>(ValueVariant);
      // join values to string
      std::string Str;
      std::for_each(std::begin(Values), std::end(Values) - 1, [&Str](int X) {
        Str.append(std::to_string(X));
        Str.push_back(';');
      });
      Str.append(std::to_string(Values.back()));

      Rc = xmlTextWriterWriteElement(Writer, XmlConstants::VALUES,
                                     BAD_CAST Str.data());
      CHECK_RC
    }
  }

  // location?
  if (FeatureSourceRange *Fsr = Feature1.getFeatureSourceRange()) {
    Rc = writeSourceRange(Writer, *Fsr);
    CHECK_RC
  }

  Rc = xmlTextWriterEndElement(Writer); // CONFIGURATIONOPTION
  return Rc;
}

int FeatureModelXmlWriter::writeSourceRange(xmlTextWriterPtr Writer,
                                            FeatureSourceRange &Location) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, XmlConstants::LOCATION);
  CHECK_RC
  Rc = xmlTextWriterWriteElement(Writer, XmlConstants::PATH,
                                 BAD_CAST Location.getPath().string().data());
  CHECK_RC

  Rc = xmlTextWriterStartElement(Writer, XmlConstants::START);
  CHECK_RC
  auto *Start = Location.getStart();
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::LINE,
      BAD_CAST std::to_string(Start->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN,
      BAD_CAST std::to_string(Start->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // START
  CHECK_RC

  Rc = xmlTextWriterStartElement(Writer, XmlConstants::END);
  CHECK_RC
  auto *End = Location.getStart();
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::LINE,
      BAD_CAST std::to_string(End->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN,
      BAD_CAST std::to_string(End->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // END
  CHECK_RC

  Rc = xmlTextWriterEndElement(Writer); // LOCATION
  return Rc;
}

} // namespace vara::feature
