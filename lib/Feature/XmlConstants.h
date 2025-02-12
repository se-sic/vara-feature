#ifndef VARA_FEATURE_XMLCONSTANTS_H
#define VARA_FEATURE_XMLCONSTANTS_H

#include "libxml/tree.h"

namespace vara::feature {

class XmlConstants {
public:
  XmlConstants() = delete;
  XmlConstants(const XmlConstants &) = delete;
  XmlConstants &operator=(const XmlConstants &) = delete;
  XmlConstants(XmlConstants &&) noexcept = delete;
  XmlConstants &operator=(XmlConstants &&) noexcept = delete;
  ~XmlConstants() = delete;

  static constexpr xmlChar VM[] = "vm";
  static constexpr xmlChar NAME[] = "name";
  static constexpr xmlChar OUTPUTSTRING[] = "outputString";
  static constexpr xmlChar COMMIT[] = "commit";
  static constexpr xmlChar OPTIONAL[] = "optional";
  static constexpr xmlChar PARENT[] = "parent";
  static constexpr xmlChar CHILDREN[] = "children";
  static constexpr xmlChar IMPLIEDOPTIONS[] = "impliedOptions";
  static constexpr xmlChar EXCLUDEDOPTIONS[] = "excludedOptions";
  static constexpr xmlChar OPTIONS[] = "options";
  static constexpr xmlChar LOCATIONS[] = "locations";
  static constexpr xmlChar SOURCERANGE[] = "sourceRange";
  static constexpr xmlChar REVISIONRANGE[] = "revisionRange";
  static constexpr xmlChar INTRODUCED[] = "introduced";
  static constexpr xmlChar REMOVED[] = "removed";
  static constexpr xmlChar MEMBEROFFSET[] = "memberOffset";
  static constexpr xmlChar PATH[] = "path";
  static constexpr xmlChar START[] = "start";
  static constexpr xmlChar END[] = "end";
  static constexpr xmlChar CATEGORY[] = "category";
  static constexpr xmlChar NECESSARY[] = "necessary";
  static constexpr xmlChar INESSENTIAL[] = "inessential";
  static constexpr xmlChar MINVALUE[] = "minValue";
  static constexpr xmlChar MAXVALUE[] = "maxValue";
  static constexpr xmlChar STEPFUNCTION[] = "stepFunction";
  static constexpr xmlChar VALUES[] = "values";
  static constexpr xmlChar CONFIGURATIONOPTION[] = "configurationOption";
  static constexpr xmlChar CONSTRAINT[] = "constraint";
  static constexpr xmlChar REQ[] = "req";
  static constexpr xmlChar EXPRKIND[] = "exprKind";
  static constexpr xmlChar ROOT[] = "root";
  static constexpr xmlChar BINARYOPTIONS[] = "binaryOptions";
  static constexpr xmlChar NUMERICOPTIONS[] = "numericOptions";
  static constexpr xmlChar BOOLEANCONSTRAINTS[] = "booleanConstraints";
  static constexpr xmlChar MIXEDCONSTRAINTS[] = "mixedConstraints";
  static constexpr xmlChar NONBOOLEANCONSTRAINTS[] = "nonBooleanConstraints";
  static constexpr xmlChar LINE[] = "line";
  static constexpr xmlChar COLUMN[] = "column";
  static inline const std::string DtdRaw =
      "<!ELEMENT vm (binaryOptions, numericOptions?, booleanConstraints?, "
      "nonBooleanConstraints?, mixedConstraints?)>\n"
      "<!ATTLIST vm name CDATA #REQUIRED root CDATA #IMPLIED commit CDATA "
      "#IMPLIED>\n"
      "<!ELEMENT binaryOptions (configurationOption*)>\n"
      "<!ELEMENT numericOptions (configurationOption*)>\n"
      "<!ELEMENT booleanConstraints (constraint*)>\n"
      "<!ELEMENT nonBooleanConstraints (constraint*)>\n"
      "<!ELEMENT mixedConstraints (constraint*)>\n"
      "<!ELEMENT configurationOption (name, outputString?, (prefix | preFix)?, "
      "(postfix | postFix)?, parent?, children?, impliedOptions?, "
      "excludedOptions?, defaultValue?, optional?, ((minValue, maxValue) | "
      "values)?, "
      "stepFunction?, locations?)>\n"
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
      "<!ELEMENT locations (sourceRange*)>\n"
      "<!ELEMENT sourceRange (revisionRange?, path, start, end, "
      "memberOffset?)>\n"
      "<!ATTLIST sourceRange category (necessary|inessential) \"necessary\">\n"
      "<!ELEMENT revisionRange (introduced, removed?)>\n"
      "<!ELEMENT introduced (#PCDATA)>\n"
      "<!ELEMENT removed (#PCDATA)>\n"
      "<!ELEMENT path (#PCDATA)>\n"
      "<!ELEMENT start (line, column)>\n"
      "<!ELEMENT end (line, column)>\n"
      "<!ELEMENT line (#PCDATA)>\n"
      "<!ELEMENT column (#PCDATA)>\n"
      "<!ELEMENT memberOffset (#PCDATA)>";
};
} // namespace vara::feature

#endif // VARA_FEATURE_XMLCONSTANTS_H
