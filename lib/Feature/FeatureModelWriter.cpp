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
  Rc = xmlTextWriterStartElement(Writer, VM);
  CHECK_RC
  Rc = xmlTextWriterWriteAttribute(Writer, NAME, BAD_CAST Fm.getName().data());
  CHECK_RC
  Rc = xmlTextWriterWriteAttribute(Writer, ROOT,
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
  Rc = xmlTextWriterStartElement(Writer, BINARYOPTIONS);
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
  Rc = xmlTextWriterStartElement(Writer, NUMERICOPTIONS);
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
  Rc = xmlTextWriterStartElement(Writer, BOOLEANCONSTRAINTS);
  CHECK_RC

  for (auto *F : Fm.features()) {
    // TODO implement this

    Rc = xmlTextWriterWriteElement(Writer, CONSTRAINT, nullptr /*TODO*/);
    CHECK_RC
  }

  Rc = xmlTextWriterEndElement(Writer); // BOOLEANCONSTRAINT
  return Rc;
}

int FeatureModelXmlWriter::writeFeature(xmlTextWriterPtr Writer,
                                        Feature &Feature1) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, CONFIGURATIONOPTION);
  CHECK_RC
  Rc = xmlTextWriterWriteElement(Writer, NAME,
                                 BAD_CAST Feature1.getName().data());
  CHECK_RC

  // optional
  Rc = xmlTextWriterWriteElement(
      Writer, OPTIONAL, BAD_CAST(Feature1.isOptional() ? "True" : "False"));
  CHECK_RC

  // parent
  if (not Feature1.isRoot()) {
    Rc = xmlTextWriterWriteElement(
        Writer, PARENT, BAD_CAST Feature1.getParent()->getName().data());
    CHECK_RC
  }

  // TODO(christian) are there no children noted in XML?

  // exclude
  Rc = xmlTextWriterStartElement(Writer, EXCLUDEDOPTIONS);
  CHECK_RC
  for (Feature *F : Feature1.children()) {
    Rc = xmlTextWriterWriteElement(Writer, OPTIONS,
                                   BAD_CAST F->getName().data());
    CHECK_RC
  }
  Rc = xmlTextWriterEndElement(Writer); // EXCLUDEOPTIONS
  CHECK_RC

  // TODO(christian) are there no implications?

  // location?
  if (FeatureSourceRange *Fsr = Feature1.getFeatureSourceRange()) {
    Rc = writeSourceRange(Writer, *Fsr);
    CHECK_RC
  }

  // if numeric: values and stepsize
  if (Feature1.getKind() == Feature::FeatureKind::FK_NUMERIC) {
    auto &Nf = dynamic_cast<NumericFeature &>(Feature1);
    // TODO build expression
    // Rc = xmlTextWriterWriteElement(Writer, VALUES, nullptr)
  }

  Rc = xmlTextWriterEndElement(Writer); // CONFIGURATIONOPTION
  return Rc;
}

int FeatureModelXmlWriter::writeSourceRange(xmlTextWriterPtr Writer,
                                            FeatureSourceRange &Location) {
  int Rc;
  Rc = xmlTextWriterStartElement(Writer, LOCATION);
  CHECK_RC
  Rc = xmlTextWriterWriteElement(Writer, PATH,
                                 BAD_CAST Location.getPath().string().data());
  CHECK_RC

  Rc = xmlTextWriterStartElement(Writer, START);
  CHECK_RC
  auto *Start = Location.getStart();
  Rc = xmlTextWriterWriteElement(
      Writer, LINE, BAD_CAST std::to_string(Start->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, COLUMN, BAD_CAST std::to_string(Start->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // START
  CHECK_RC

  Rc = xmlTextWriterStartElement(Writer, END);
  CHECK_RC
  auto *End = Location.getStart();
  Rc = xmlTextWriterWriteElement(
      Writer, LINE, BAD_CAST std::to_string(End->getLineNumber()).data());
  CHECK_RC
  Rc = xmlTextWriterWriteElement(
      Writer, COLUMN, BAD_CAST std::to_string(End->getColumnOffset()).data());
  CHECK_RC
  Rc = xmlTextWriterEndElement(Writer); // END
  CHECK_RC

  Rc = xmlTextWriterEndElement(Writer); // LOCATION
  return Rc;
}

} // namespace vara::feature
