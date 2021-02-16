#ifndef VARA_FEATURE_SXFMCONSTANTS_H
#define VARA_FEATURE_SXFMCONSTANTS_H

#include "libxml/tree.h"

namespace vara::feature {

class SxfmConstants {
public:
  SxfmConstants() = delete;
  SxfmConstants(const SxfmConstants &) = delete;
  SxfmConstants &operator=(const SxfmConstants &) = delete;
  SxfmConstants(SxfmConstants &&) noexcept = delete;
  SxfmConstants &operator=(SxfmConstants &&) noexcept = delete;
  ~SxfmConstants() = delete;
  static inline const int WILDCARD = -1;
  static constexpr xmlChar NAME[] = "name";
  static constexpr xmlChar FEATURE_TREE[] = "feature_tree";
  static constexpr xmlChar CONSTRAINTS[] = "constraints";
  static constexpr xmlChar INDENTATION[] = "indentation_string";
  static inline const std::string DtdRaw =
      "<!ELEMENT feature_model (meta?, feature_tree, constraints?)>\n"
      "<!ATTLIST feature_model\n"
      "   name CDATA #IMPLIED\n"
      ">\n"
      "<!ELEMENT meta (data*)>\n"
      "<!ELEMENT data (#PCDATA)>\n"
      "<!ATTLIST data name CDATA #REQUIRED>\n"
      "<!ELEMENT feature_tree (#PCDATA)>\n"
      "<!ATTLIST feature_tree\n"
      "   indentation_string CDATA #IMPLIED\n"
      ">\n"
      "<!ELEMENT constraints (#PCDATA)>";
};
} // namespace vara::feature

#endif // VARA_FEATURE_XMLCONSTANTS_H
