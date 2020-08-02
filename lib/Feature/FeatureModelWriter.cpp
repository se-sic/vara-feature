#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModel.h"

#include "XmlConstants.h"

#include "libxml/xmlwriter.h"

#define CHECK_RC                                                               \
  if (Rc < 0) {                                                                \
    return Rc;                                                                 \
  }

namespace vara::feature {

static constexpr xmlChar IndentationString[] = "  ";

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

int FeatureModelXmlWriter::writeFeatureModel(char **Doc) {
  int Rc;
  xmlTextWriterPtr Writer;
  xmlBufferPtr Buf;

  Buf = xmlBufferCreate();
  Writer = xmlNewTextWriterMemory(Buf, 0);
  Rc = writeFeatureModel(Writer);

  xmlFreeTextWriter(Writer);
  // copy document into char buffer
  *Doc = static_cast<char *>(calloc(xmlBufferLength(Buf), 1));
  memcpy(*Doc, xmlBufferContent(Buf), xmlBufferLength(Buf));

  xmlBufferFree(Buf);
  return Rc;
}

int FeatureModelXmlWriter::writeFeatureModel(xmlTextWriterPtr Writer) {
  int Rc;

  Rc = xmlTextWriterSetIndent(Writer, 1);
  CHECK_RC
  Rc = xmlTextWriterSetIndentString(Writer, IndentationString);
  CHECK_RC

  Rc = xmlTextWriterStartDocument(Writer, "1.0", "UTF-8", nullptr);
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
  Rc = xmlTextWriterWriteAttribute(Writer, XmlConstants::NAME, BAD_CAST Fm.getName().data());
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

  // TODO (padupr) add alternatives
  for (auto *F : Fm.features()) {};
  // TODO (padupr) add constraints, after adding constraint accessor

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
      Writer, XmlConstants::OPTIONAL, BAD_CAST(Feature1.isOptional() ? "True" : "False"));
  CHECK_RC

  // parent
  if (not Feature1.isRoot()) {
    Rc = xmlTextWriterWriteElement(
        Writer, XmlConstants::PARENT, BAD_CAST Feature1.getParent()->getName().data());
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
  Rc = xmlTextWriterEndElement(Writer); // EXCLUDEOPTIONS
  CHECK_RC

  // location?
  if (FeatureSourceRange *Fsr = Feature1.getFeatureSourceRange()) {
    Rc = writeSourceRange(Writer, *Fsr);
    CHECK_RC
  }

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

      Rc = xmlTextWriterWriteElement(Writer, XmlConstants::VALUES, BAD_CAST Str.data());
      CHECK_RC
    }
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
      Writer, XmlConstants::LINE, BAD_CAST std::to_string(Start->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN, BAD_CAST std::to_string(Start->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // START
  CHECK_RC

  Rc = xmlTextWriterStartElement(Writer, XmlConstants::END);
  CHECK_RC
  auto *End = Location.getStart();
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::LINE, BAD_CAST std::to_string(End->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, XmlConstants::COLUMN, BAD_CAST std::to_string(End->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // END
  CHECK_RC

  Rc = xmlTextWriterEndElement(Writer); // LOCATION
  return Rc;
}

} // namespace vara::feature
