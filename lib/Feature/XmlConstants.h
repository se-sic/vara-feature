#ifndef VARA_FEATURE_XMLCONSTANTS_H
#define VARA_FEATURE_XMLCONSTANTS_H

#include "libxml/tree.h"

namespace vara::feature {

inline static const std::string DtdRaw =
    "<!ELEMENT vm (binaryOptions, numericOptions?, booleanConstraints?, "
    "nonBooleanConstraints?, mixedConstraints?)>\n"
    "<!ATTLIST vm name CDATA #REQUIRED root CDATA #IMPLIED>\n"
    "<!ELEMENT binaryOptions (configurationOption*)>\n"
    "<!ELEMENT numericOptions (configurationOption*)>\n"
    "<!ELEMENT booleanConstraints (constraint*)>\n"
    "<!ELEMENT nonBooleanConstraints (constraint*)>\n"
    "<!ELEMENT mixedConstraints (constraint*)>\n"
    "<!ELEMENT configurationOption (name, outputString?, (prefix | preFix)?, "
    "(postfix | postFix)?, parent?, children?, impliedOptions?, "
    "excludedOptions?, defaultValue?, optional?, ((minValue, maxValue) | "
    "values)?, "
    "stepFunction?, location?)>\n"
    "<!ELEMENT constraint (#PCDATA)>\n"
    "<!ATTLIST constraint req CDATA #IMPLIED exprKind CDATA #IMPLIED>\n"
    "<!ELEMENT name (#PCDATA)>\n"
    "<!ELEMENT outputString (#PCDATA)>\n"
    "<!ELEMENT prefix (#PCDATA)>\n"
    "<!ELEMENT preFix (#PCDATA)>\n"
    "<!ELEMENT postfix (#PCDATA)>\n"
    "<!ELEMENT postFix (#PCDATA)>\n"
    "<!ELEMENT parent (#PCDATA)>\n"
    "<!ELEMENT children (options*)>\n"
    "<!ELEMENT impliedOptions (options*)>\n"
    "<!ELEMENT excludedOptions (options*)>\n"
    "<!ELEMENT defaultValue (#PCDATA)>\n"
    "<!ELEMENT options (#PCDATA)>\n"
    "<!ELEMENT optional (#PCDATA)>\n"
    "<!ELEMENT minValue (#PCDATA)>\n"
    "<!ELEMENT maxValue (#PCDATA)>\n"
    "<!ELEMENT values (#PCDATA)>\n"
    "<!ELEMENT stepFunction (#PCDATA)>\n"
    "<!ELEMENT location (path, start?, end?)>\n"
    "<!ELEMENT path (#PCDATA)>\n"
    "<!ELEMENT start (line, column)>\n"
    "<!ELEMENT end (line, column)>\n"
    "<!ELEMENT line (#PCDATA)>\n"
    "<!ELEMENT column (#PCDATA)>";

static constexpr xmlChar VM[] = "vm";
static constexpr xmlChar NAME[] = "name";
static constexpr xmlChar OPTIONAL[] = "optional";
static constexpr xmlChar PARENT[] = "parent";
static constexpr xmlChar EXCLUDEDOPTIONS[] = "excludedOptions";
static constexpr xmlChar OPTIONS[] = "options";
static constexpr xmlChar LOCATION[] = "location";
static constexpr xmlChar PATH[] = "path";
static constexpr xmlChar START[] = "start";
static constexpr xmlChar END[] = "end";
static constexpr xmlChar MINVALUE[] = "minValue";
static constexpr xmlChar MAXVALUE[] = "maxValue";
static constexpr xmlChar VALUES[] = "values";
static constexpr xmlChar CONFIGURATIONOPTION[] = "configurationOption";
static constexpr xmlChar CONSTRAINT[] = "constraint";
static constexpr xmlChar ROOT[] = "root";
static constexpr xmlChar BINARYOPTIONS[] = "binaryOptions";
static constexpr xmlChar NUMERICOPTIONS[] = "numericOptions";
static constexpr xmlChar BOOLEANCONSTRAINTS[] = "booleanConstraints";
static constexpr xmlChar LINE[] = "line";
static constexpr xmlChar COLUMN[] = "column";

} // namespace vara::feature

#endif //VARA_FEATURE_XMLCONSTANTS_H
