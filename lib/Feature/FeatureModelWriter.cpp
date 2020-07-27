#include "vara/Feature/FeatureModelWriter.h"
#include "vara/Feature/FeatureModel.h"

#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

#include <iostream>
#include <regex>

#define CHECK_RC if (rc < 0) { return rc; }

namespace vara::feature {


bool FeatureModelXmlWriter::writeFeatureModel(string path) {
    int rc;
    xmlTextWriterPtr writer;
    xmlChar *tmp;

    writer = xmlNewTextWriterFilename(path.data(), 0);
    if (writer == nullptr) {
        return false;
    }

    rc = writeVm(writer);
    xmlFreeTextWriter(writer);
    // report failure/success
    return rc < 0;
}

int FeatureModelXmlWriter::writeVm(xmlTextWriterPtr writer) {
    int rc;
    // open vm node
    rc = xmlTextWriterStartElement(writer, VM);
    CHECK_RC
    rc = xmlTextWriterWriteAttribute(writer, NAME, BAD_CAST fm.getName().data());
    CHECK_RC
    rc = xmlTextWriterWriteAttribute(writer, ROOT, nullptr /*TODO where is this?*/);
    CHECK_RC

    // write BinaryFeatures
    rc = writeBinaryFeatures(writer);
    CHECK_RC

    // write NumericFeatures
    rc = writeNumericFeatures(writer);
    CHECK_RC

    // write Constraints
    rc = writeConstraints(writer);
    CHECK_RC

    rc = xmlTextWriterEndElement(writer); // VM
    return rc;
}

int FeatureModelXmlWriter::writeBinaryFeatures(xmlTextWriterPtr writer) {
    int rc;
    rc = xmlTextWriterStartElement(writer, BINARYOPTIONS);
    CHECK_RC
    for (Feature *f : fm.features()) {
        if (f->getKind() == Feature::FeatureKind::FK_BINARY) {
            rc = writeFeature(writer, *f);
        }
    }

    rc = xmlTextWriterEndElement(writer); // BINARYOPTIONS
    return rc;
}

int FeatureModelXmlWriter::writeNumericFeatures(xmlTextWriterPtr writer) {
    int rc;
    rc = xmlTextWriterStartElement(writer, NUMERICOPTIONS);
    CHECK_RC
    for (Feature *f : fm.features()) {
      if (f->getKind() == Feature::FeatureKind::FK_NUMERIC) {
        rc = writeFeature(writer, *f);
      }
    }

    rc = xmlTextWriterEndElement(writer); // NUMERICOPTIONS
    return rc;
}

int FeatureModelXmlWriter::writeConstraints(xmlTextWriterPtr writer) {
    int rc;
    return rc;
}

int FeatureModelXmlWriter::writeFeature(xmlTextWriterPtr writer, Feature &feature) {
    int rc;
    rc = xmlTextWriterStartElement(writer, CONFIGURATIONOPTION);
    CHECK_RC
    rc = xmlTextWriterWriteElement(writer, NAME, BAD_CAST feature.getName().data());
    CHECK_RC

    // optional
    rc = xmlTextWriterWriteElement(writer, OPTIONAL, BAD_CAST (feature.isOptional() ? "True" : "False"));
    CHECK_RC

    // parent
    if (not feature.isRoot()) {
        rc = xmlTextWriterWriteElement(writer, PARENT, BAD_CAST feature.getParent()->getName().data());
        CHECK_RC
    }

    // TODO are there no children noted in XML?

    // exclude
    rc = xmlTextWriterStartElement(writer, EXCLUDEDOPTIONS);
    CHECK_RC
    for (Feature *f : feature.children()) {
        rc = xmlTextWriterWriteElement(writer, OPTIONS, BAD_CAST f->getName().data());
        CHECK_RC
    }
    rc = xmlTextWriterEndElement(writer); // EXCLUDEOPTIONS
    CHECK_RC

    // TODO are there no implications?

    // location?
    // TODO REVIEW: ok in c++?
    if (FeatureSourceRange *fsr = feature.getFeatureSourceRange()) {
      rc = writeSourceRange(writer, *fsr);
      CHECK_RC
    }

    // if numeric: values and stepsize
    if (feature.getKind() == Feature::FeatureKind::FK_NUMERIC) {
      auto &nf = dynamic_cast<NumericFeature &>(feature);
      // TODO build expression
      // rc = xmlTextWriterWriteElement(writer, VALUES, nullptr)
    }

    rc = xmlTextWriterEndElement(writer); // CONFIGURATIONOPTION
    return rc;
}

int FeatureModelXmlWriter::writeSourceRange(xmlTextWriterPtr writer, FeatureSourceRange &fsr) {
    int rc;
    rc = xmlTextWriterStartElement(writer, LOCATION);
    CHECK_RC
    rc = xmlTextWriterWriteElement(writer, PATH, BAD_CAST fsr.getPath().string().data());
    CHECK_RC

    rc = xmlTextWriterStartElement(writer, START);
    CHECK_RC
    auto start = fsr.getStart();
    rc = xmlTextWriterWriteElement(writer, LINE, BAD_CAST std::to_string(start->getLineNumber()).data());
    CHECK_RC
    rc = xmlTextWriterWriteElement(writer, COLUMN, BAD_CAST std::to_string(start->getColumnOffset()).data());
    CHECK_RC
    rc = xmlTextWriterEndElement(writer); // START
    CHECK_RC

    rc = xmlTextWriterStartElement(writer, END);
    CHECK_RC
    auto end = fsr.getStart();
    rc = xmlTextWriterWriteElement(writer, LINE, BAD_CAST std::to_string(end->getLineNumber()).data());
    CHECK_RC
    rc = xmlTextWriterWriteElement(writer, COLUMN, BAD_CAST std::to_string(end->getColumnOffset()).data());
    CHECK_RC
    rc = xmlTextWriterEndElement(writer); // END
    CHECK_RC

    rc = xmlTextWriterEndElement(writer); // LOCATION
    return rc;
}


} // namespace vara::feature
