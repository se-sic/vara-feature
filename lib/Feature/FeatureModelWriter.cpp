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

    writer = xmlNewTextWriterFilename(path, 0)  // TODO: convert string to char *
    if (writer == NULL) {
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
    rc = xmlTextWriterWriteAttribute(writer, NAME, BAD_CAST fm.getName());
    CHECK_RC
    rc = xmlTextWriterWriteAttribute(writer, ROOT, BAD_CAST fm.getRoot());
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
    // close vm node
    rc = xmlTextWriterEndElement(writer);
    return rc;
}

int FeatureModelXmlWriter::writeBinaryFeatures(xmlTextWriterPtr writer) {
    int rc;
    rc = xmlTextWriterStartElement(writer, BINARYOPTIONS);
    CHECK_RC
    for (Feature f : fm.features()) {
        if (f->getKind() == Feature::FeatureKind::FK_BINARY) {
            rc = writeFeature(writer, *f)
        }
    }

    rc = xmlTextWriterEndElement(writer);
    return rc;
}

int FeatureModelXmlWriter::writeNumericFeatures(xmlTextWriterPtr writer) {
    int rc;
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
    rc = xmlTextWriterWriteElement(writer, NAME, feature.getName());
    CHECK_RC

    // optional
    rc = xmlTextWriterWriteElement(writer, OPTIONAL, BAD_CAST (feature.isOptional() ? "True" : "False"));
    CHECK_RC

    // parent
    if (not feature.isRoot()) {
        rc = xmlTextWriterWriteElement(writer, PARENT, feature.getParent()->getName());
        CHECK_RC
    }

    // TODO are there no children noted in XML?

    // exclude
    rc = xmlTextWriterStartElement(writer, EXCLUDEDOPTIONS);
    CHECK_RC
    for (Feature *f : feature.children()) {
        rc = xmlTextWriterWriteElement(writer, OPTIONS, f->getName());
        CHECK_RC
    }
    rc = xmlTextWriterEndElement(writer);
    CHECK_RC

    // TODO are there no implications?

    // location?

    // if numeric: values and stepsize
    if (feature.getKind() == Feature::FeatureKind::FK_NUMERIC) {

    }

    rc = xmlTextWriterEndElement(writer);
    return rc;
}

int FeatureModelXmlWriter::writerSourceRange(xmlTextWriterPtr writer, FeatureSourceRange &fsr) {
    int rc;

    return rc;
}


} // namespace vara::feature
