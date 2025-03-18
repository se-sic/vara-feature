
// Generated from UVLcpp.g4 by ANTLR 4.9.2


#include "UVLcppVisitor.h"

#include "UVLcppParser.h"


using namespace antlrcpp;
using namespace antlr4;

UVLcppParser::UVLcppParser(TokenStream *input) : Parser(input) {
  _interpreter = new atn::ParserATNSimulator(this, _atn, _decisionToDFA, _sharedContextCache);
}

UVLcppParser::~UVLcppParser() {
  delete _interpreter;
}

std::string UVLcppParser::getGrammarFileName() const {
  return "UVLcpp.g4";
}

const std::vector<std::string>& UVLcppParser::getRuleNames() const {
  return _ruleNames;
}

dfa::Vocabulary& UVLcppParser::getVocabulary() const {
  return _vocabulary;
}


//----------------- FeatureModelContext ------------------------------------------------------------------

UVLcppParser::FeatureModelContext::FeatureModelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::FeatureModelContext::EOF() {
  return getToken(UVLcppParser::EOF, 0);
}

UVLcppParser::NamespaceContext* UVLcppParser::FeatureModelContext::namespace() {
  return getRuleContext<UVLcppParser::NamespaceContext>(0);
}

std::vector<tree::TerminalNode *> UVLcppParser::FeatureModelContext::NEWLINE() {
  return getTokens(UVLcppParser::NEWLINE);
}

tree::TerminalNode* UVLcppParser::FeatureModelContext::NEWLINE(size_t i) {
  return getToken(UVLcppParser::NEWLINE, i);
}

UVLcppParser::IncludesContext* UVLcppParser::FeatureModelContext::includes() {
  return getRuleContext<UVLcppParser::IncludesContext>(0);
}

UVLcppParser::ImportsContext* UVLcppParser::FeatureModelContext::imports() {
  return getRuleContext<UVLcppParser::ImportsContext>(0);
}

UVLcppParser::FeaturesContext* UVLcppParser::FeatureModelContext::features() {
  return getRuleContext<UVLcppParser::FeaturesContext>(0);
}

UVLcppParser::ConstraintsContext* UVLcppParser::FeatureModelContext::constraints() {
  return getRuleContext<UVLcppParser::ConstraintsContext>(0);
}


size_t UVLcppParser::FeatureModelContext::getRuleIndex() const {
  return UVLcppParser::RuleFeatureModel;
}

antlrcpp::Any UVLcppParser::FeatureModelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFeatureModel(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::FeatureModelContext* UVLcppParser::featureModel() {
  FeatureModelContext *_localctx = _tracker.createInstance<FeatureModelContext>(_ctx, getState());
  enterRule(_localctx, 0, UVLcppParser::RuleFeatureModel);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(67);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__1) {
      setState(66);
      namespace();
    }
    setState(70);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      setState(69);
      match(UVLcppParser::NEWLINE);
      break;
    }

    default:
      break;
    }
    setState(73);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__0) {
      setState(72);
      includes();
    }
    setState(76);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 3, _ctx)) {
    case 1: {
      setState(75);
      match(UVLcppParser::NEWLINE);
      break;
    }

    default:
      break;
    }
    setState(79);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__2) {
      setState(78);
      imports();
    }
    setState(82);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 5, _ctx)) {
    case 1: {
      setState(81);
      match(UVLcppParser::NEWLINE);
      break;
    }

    default:
      break;
    }
    setState(85);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__4) {
      setState(84);
      features();
    }
    setState(88);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::NEWLINE) {
      setState(87);
      match(UVLcppParser::NEWLINE);
    }
    setState(91);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__7) {
      setState(90);
      constraints();
    }
    setState(93);
    match(UVLcppParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IncludesContext ------------------------------------------------------------------

UVLcppParser::IncludesContext::IncludesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::IncludesContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

tree::TerminalNode* UVLcppParser::IncludesContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

tree::TerminalNode* UVLcppParser::IncludesContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}

std::vector<UVLcppParser::IncludeLineContext *> UVLcppParser::IncludesContext::includeLine() {
  return getRuleContexts<UVLcppParser::IncludeLineContext>();
}

UVLcppParser::IncludeLineContext* UVLcppParser::IncludesContext::includeLine(size_t i) {
  return getRuleContext<UVLcppParser::IncludeLineContext>(i);
}


size_t UVLcppParser::IncludesContext::getRuleIndex() const {
  return UVLcppParser::RuleIncludes;
}

antlrcpp::Any UVLcppParser::IncludesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitIncludes(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::IncludesContext* UVLcppParser::includes() {
  IncludesContext *_localctx = _tracker.createInstance<IncludesContext>(_ctx, getState());
  enterRule(_localctx, 2, UVLcppParser::RuleIncludes);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(95);
    match(UVLcppParser::T__0);
    setState(96);
    match(UVLcppParser::NEWLINE);
    setState(97);
    match(UVLcppParser::INDENT);
    setState(101);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__17)
      | (1ULL << UVLcppParser::T__18)
      | (1ULL << UVLcppParser::BOOLEAN_KEY))) != 0)) {
      setState(98);
      includeLine();
      setState(103);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(104);
    match(UVLcppParser::DEDENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IncludeLineContext ------------------------------------------------------------------

UVLcppParser::IncludeLineContext::IncludeLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::LanguageLevelContext* UVLcppParser::IncludeLineContext::languageLevel() {
  return getRuleContext<UVLcppParser::LanguageLevelContext>(0);
}

tree::TerminalNode* UVLcppParser::IncludeLineContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}


size_t UVLcppParser::IncludeLineContext::getRuleIndex() const {
  return UVLcppParser::RuleIncludeLine;
}

antlrcpp::Any UVLcppParser::IncludeLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitIncludeLine(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::IncludeLineContext* UVLcppParser::includeLine() {
  IncludeLineContext *_localctx = _tracker.createInstance<IncludeLineContext>(_ctx, getState());
  enterRule(_localctx, 4, UVLcppParser::RuleIncludeLine);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(106);
    languageLevel();
    setState(107);
    match(UVLcppParser::NEWLINE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamespaceContext ------------------------------------------------------------------

UVLcppParser::NamespaceContext::NamespaceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::ReferenceContext* UVLcppParser::NamespaceContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}


size_t UVLcppParser::NamespaceContext::getRuleIndex() const {
  return UVLcppParser::RuleNamespace;
}

antlrcpp::Any UVLcppParser::NamespaceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitNamespace(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::NamespaceContext* UVLcppParser::namespace() {
  NamespaceContext *_localctx = _tracker.createInstance<NamespaceContext>(_ctx, getState());
  enterRule(_localctx, 6, UVLcppParser::RuleNamespace);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(109);
    match(UVLcppParser::T__1);
    setState(110);
    reference();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportsContext ------------------------------------------------------------------

UVLcppParser::ImportsContext::ImportsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::ImportsContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

tree::TerminalNode* UVLcppParser::ImportsContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

tree::TerminalNode* UVLcppParser::ImportsContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}

std::vector<UVLcppParser::ImportLineContext *> UVLcppParser::ImportsContext::importLine() {
  return getRuleContexts<UVLcppParser::ImportLineContext>();
}

UVLcppParser::ImportLineContext* UVLcppParser::ImportsContext::importLine(size_t i) {
  return getRuleContext<UVLcppParser::ImportLineContext>(i);
}


size_t UVLcppParser::ImportsContext::getRuleIndex() const {
  return UVLcppParser::RuleImports;
}

antlrcpp::Any UVLcppParser::ImportsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitImports(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ImportsContext* UVLcppParser::imports() {
  ImportsContext *_localctx = _tracker.createInstance<ImportsContext>(_ctx, getState());
  enterRule(_localctx, 8, UVLcppParser::RuleImports);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(112);
    match(UVLcppParser::T__2);
    setState(113);
    match(UVLcppParser::NEWLINE);
    setState(114);
    match(UVLcppParser::INDENT);
    setState(118);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == UVLcppParser::ID_NOT_STRICT

    || _la == UVLcppParser::ID_STRICT) {
      setState(115);
      importLine();
      setState(120);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(121);
    match(UVLcppParser::DEDENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportLineContext ------------------------------------------------------------------

UVLcppParser::ImportLineContext::ImportLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::ImportLineContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

std::vector<UVLcppParser::ReferenceContext *> UVLcppParser::ImportLineContext::reference() {
  return getRuleContexts<UVLcppParser::ReferenceContext>();
}

UVLcppParser::ReferenceContext* UVLcppParser::ImportLineContext::reference(size_t i) {
  return getRuleContext<UVLcppParser::ReferenceContext>(i);
}


size_t UVLcppParser::ImportLineContext::getRuleIndex() const {
  return UVLcppParser::RuleImportLine;
}

antlrcpp::Any UVLcppParser::ImportLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitImportLine(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ImportLineContext* UVLcppParser::importLine() {
  ImportLineContext *_localctx = _tracker.createInstance<ImportLineContext>(_ctx, getState());
  enterRule(_localctx, 10, UVLcppParser::RuleImportLine);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(123);
    dynamic_cast<ImportLineContext *>(_localctx)->ns = reference();
    setState(126);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__3) {
      setState(124);
      match(UVLcppParser::T__3);
      setState(125);
      dynamic_cast<ImportLineContext *>(_localctx)->alias = reference();
    }
    setState(128);
    match(UVLcppParser::NEWLINE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FeaturesContext ------------------------------------------------------------------

UVLcppParser::FeaturesContext::FeaturesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::FeaturesContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

tree::TerminalNode* UVLcppParser::FeaturesContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

UVLcppParser::FeatureContext* UVLcppParser::FeaturesContext::feature() {
  return getRuleContext<UVLcppParser::FeatureContext>(0);
}

tree::TerminalNode* UVLcppParser::FeaturesContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}


size_t UVLcppParser::FeaturesContext::getRuleIndex() const {
  return UVLcppParser::RuleFeatures;
}

antlrcpp::Any UVLcppParser::FeaturesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFeatures(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::FeaturesContext* UVLcppParser::features() {
  FeaturesContext *_localctx = _tracker.createInstance<FeaturesContext>(_ctx, getState());
  enterRule(_localctx, 12, UVLcppParser::RuleFeatures);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(130);
    match(UVLcppParser::T__4);
    setState(131);
    match(UVLcppParser::NEWLINE);
    setState(132);
    match(UVLcppParser::INDENT);
    setState(133);
    feature();
    setState(134);
    match(UVLcppParser::DEDENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GroupContext ------------------------------------------------------------------

UVLcppParser::GroupContext::GroupContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::GroupContext::getRuleIndex() const {
  return UVLcppParser::RuleGroup;
}

void UVLcppParser::GroupContext::copyFrom(GroupContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AlternativeGroupContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::AlternativeGroupContext::ALTERNATIVE() {
  return getToken(UVLcppParser::ALTERNATIVE, 0);
}

UVLcppParser::GroupSpecContext* UVLcppParser::AlternativeGroupContext::groupSpec() {
  return getRuleContext<UVLcppParser::GroupSpecContext>(0);
}

UVLcppParser::AlternativeGroupContext::AlternativeGroupContext(GroupContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::AlternativeGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAlternativeGroup(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OptionalGroupContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::OptionalGroupContext::OPTIONAL() {
  return getToken(UVLcppParser::OPTIONAL, 0);
}

UVLcppParser::GroupSpecContext* UVLcppParser::OptionalGroupContext::groupSpec() {
  return getRuleContext<UVLcppParser::GroupSpecContext>(0);
}

UVLcppParser::OptionalGroupContext::OptionalGroupContext(GroupContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::OptionalGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitOptionalGroup(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MandatoryGroupContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::MandatoryGroupContext::MANDATORY() {
  return getToken(UVLcppParser::MANDATORY, 0);
}

UVLcppParser::GroupSpecContext* UVLcppParser::MandatoryGroupContext::groupSpec() {
  return getRuleContext<UVLcppParser::GroupSpecContext>(0);
}

UVLcppParser::MandatoryGroupContext::MandatoryGroupContext(GroupContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::MandatoryGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitMandatoryGroup(this);
  else
    return visitor->visitChildren(this);
}
//----------------- CardinalityGroupContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::CardinalityGroupContext::CARDINALITY() {
  return getToken(UVLcppParser::CARDINALITY, 0);
}

UVLcppParser::GroupSpecContext* UVLcppParser::CardinalityGroupContext::groupSpec() {
  return getRuleContext<UVLcppParser::GroupSpecContext>(0);
}

UVLcppParser::CardinalityGroupContext::CardinalityGroupContext(GroupContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::CardinalityGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitCardinalityGroup(this);
  else
    return visitor->visitChildren(this);
}
//----------------- OrGroupContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::OrGroupContext::ORGROUP() {
  return getToken(UVLcppParser::ORGROUP, 0);
}

UVLcppParser::GroupSpecContext* UVLcppParser::OrGroupContext::groupSpec() {
  return getRuleContext<UVLcppParser::GroupSpecContext>(0);
}

UVLcppParser::OrGroupContext::OrGroupContext(GroupContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::OrGroupContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitOrGroup(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::GroupContext* UVLcppParser::group() {
  GroupContext *_localctx = _tracker.createInstance<GroupContext>(_ctx, getState());
  enterRule(_localctx, 14, UVLcppParser::RuleGroup);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(146);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::ORGROUP: {
        _localctx = dynamic_cast<GroupContext *>(_tracker.createInstance<UVLcppParser::OrGroupContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(136);
        match(UVLcppParser::ORGROUP);
        setState(137);
        groupSpec();
        break;
      }

      case UVLcppParser::ALTERNATIVE: {
        _localctx = dynamic_cast<GroupContext *>(_tracker.createInstance<UVLcppParser::AlternativeGroupContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(138);
        match(UVLcppParser::ALTERNATIVE);
        setState(139);
        groupSpec();
        break;
      }

      case UVLcppParser::OPTIONAL: {
        _localctx = dynamic_cast<GroupContext *>(_tracker.createInstance<UVLcppParser::OptionalGroupContext>(_localctx));
        enterOuterAlt(_localctx, 3);
        setState(140);
        match(UVLcppParser::OPTIONAL);
        setState(141);
        groupSpec();
        break;
      }

      case UVLcppParser::MANDATORY: {
        _localctx = dynamic_cast<GroupContext *>(_tracker.createInstance<UVLcppParser::MandatoryGroupContext>(_localctx));
        enterOuterAlt(_localctx, 4);
        setState(142);
        match(UVLcppParser::MANDATORY);
        setState(143);
        groupSpec();
        break;
      }

      case UVLcppParser::CARDINALITY: {
        _localctx = dynamic_cast<GroupContext *>(_tracker.createInstance<UVLcppParser::CardinalityGroupContext>(_localctx));
        enterOuterAlt(_localctx, 5);
        setState(144);
        match(UVLcppParser::CARDINALITY);
        setState(145);
        groupSpec();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- GroupSpecContext ------------------------------------------------------------------

UVLcppParser::GroupSpecContext::GroupSpecContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::GroupSpecContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

tree::TerminalNode* UVLcppParser::GroupSpecContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

tree::TerminalNode* UVLcppParser::GroupSpecContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}

std::vector<UVLcppParser::FeatureContext *> UVLcppParser::GroupSpecContext::feature() {
  return getRuleContexts<UVLcppParser::FeatureContext>();
}

UVLcppParser::FeatureContext* UVLcppParser::GroupSpecContext::feature(size_t i) {
  return getRuleContext<UVLcppParser::FeatureContext>(i);
}


size_t UVLcppParser::GroupSpecContext::getRuleIndex() const {
  return UVLcppParser::RuleGroupSpec;
}

antlrcpp::Any UVLcppParser::GroupSpecContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitGroupSpec(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::GroupSpecContext* UVLcppParser::groupSpec() {
  GroupSpecContext *_localctx = _tracker.createInstance<GroupSpecContext>(_ctx, getState());
  enterRule(_localctx, 16, UVLcppParser::RuleGroupSpec);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(148);
    match(UVLcppParser::NEWLINE);
    setState(149);
    match(UVLcppParser::INDENT);
    setState(151); 
    _errHandler->sync(this);
    _la = _input->LA(1);
    do {
      setState(150);
      feature();
      setState(153); 
      _errHandler->sync(this);
      _la = _input->LA(1);
    } while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__14)
      | (1ULL << UVLcppParser::T__15)
      | (1ULL << UVLcppParser::T__16)
      | (1ULL << UVLcppParser::BOOLEAN_KEY)
      | (1ULL << UVLcppParser::ID_NOT_STRICT)
      | (1ULL << UVLcppParser::ID_STRICT))) != 0));
    setState(155);
    match(UVLcppParser::DEDENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FeatureContext ------------------------------------------------------------------

UVLcppParser::FeatureContext::FeatureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::ReferenceContext* UVLcppParser::FeatureContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

tree::TerminalNode* UVLcppParser::FeatureContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

UVLcppParser::FeatureTypeContext* UVLcppParser::FeatureContext::featureType() {
  return getRuleContext<UVLcppParser::FeatureTypeContext>(0);
}

UVLcppParser::FeatureCardinalityContext* UVLcppParser::FeatureContext::featureCardinality() {
  return getRuleContext<UVLcppParser::FeatureCardinalityContext>(0);
}

UVLcppParser::AttributesContext* UVLcppParser::FeatureContext::attributes() {
  return getRuleContext<UVLcppParser::AttributesContext>(0);
}

tree::TerminalNode* UVLcppParser::FeatureContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

tree::TerminalNode* UVLcppParser::FeatureContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}

std::vector<UVLcppParser::GroupContext *> UVLcppParser::FeatureContext::group() {
  return getRuleContexts<UVLcppParser::GroupContext>();
}

UVLcppParser::GroupContext* UVLcppParser::FeatureContext::group(size_t i) {
  return getRuleContext<UVLcppParser::GroupContext>(i);
}


size_t UVLcppParser::FeatureContext::getRuleIndex() const {
  return UVLcppParser::RuleFeature;
}

antlrcpp::Any UVLcppParser::FeatureContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFeature(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::FeatureContext* UVLcppParser::feature() {
  FeatureContext *_localctx = _tracker.createInstance<FeatureContext>(_ctx, getState());
  enterRule(_localctx, 18, UVLcppParser::RuleFeature);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(158);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__14)
      | (1ULL << UVLcppParser::T__15)
      | (1ULL << UVLcppParser::T__16)
      | (1ULL << UVLcppParser::BOOLEAN_KEY))) != 0)) {
      setState(157);
      featureType();
    }
    setState(160);
    reference();
    setState(162);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__5) {
      setState(161);
      featureCardinality();
    }
    setState(165);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::OPEN_BRACE) {
      setState(164);
      attributes();
    }
    setState(167);
    match(UVLcppParser::NEWLINE);
    setState(176);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::INDENT) {
      setState(168);
      match(UVLcppParser::INDENT);
      setState(170); 
      _errHandler->sync(this);
      _la = _input->LA(1);
      do {
        setState(169);
        group();
        setState(172); 
        _errHandler->sync(this);
        _la = _input->LA(1);
      } while ((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & ((1ULL << UVLcppParser::ORGROUP)
        | (1ULL << UVLcppParser::ALTERNATIVE)
        | (1ULL << UVLcppParser::OPTIONAL)
        | (1ULL << UVLcppParser::MANDATORY)
        | (1ULL << UVLcppParser::CARDINALITY))) != 0));
      setState(174);
      match(UVLcppParser::DEDENT);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FeatureCardinalityContext ------------------------------------------------------------------

UVLcppParser::FeatureCardinalityContext::FeatureCardinalityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::FeatureCardinalityContext::CARDINALITY() {
  return getToken(UVLcppParser::CARDINALITY, 0);
}


size_t UVLcppParser::FeatureCardinalityContext::getRuleIndex() const {
  return UVLcppParser::RuleFeatureCardinality;
}

antlrcpp::Any UVLcppParser::FeatureCardinalityContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFeatureCardinality(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::FeatureCardinalityContext* UVLcppParser::featureCardinality() {
  FeatureCardinalityContext *_localctx = _tracker.createInstance<FeatureCardinalityContext>(_ctx, getState());
  enterRule(_localctx, 20, UVLcppParser::RuleFeatureCardinality);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    match(UVLcppParser::T__5);
    setState(179);
    match(UVLcppParser::CARDINALITY);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributesContext ------------------------------------------------------------------

UVLcppParser::AttributesContext::AttributesContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::AttributesContext::OPEN_BRACE() {
  return getToken(UVLcppParser::OPEN_BRACE, 0);
}

tree::TerminalNode* UVLcppParser::AttributesContext::CLOSE_BRACE() {
  return getToken(UVLcppParser::CLOSE_BRACE, 0);
}

std::vector<UVLcppParser::AttributeContext *> UVLcppParser::AttributesContext::attribute() {
  return getRuleContexts<UVLcppParser::AttributeContext>();
}

UVLcppParser::AttributeContext* UVLcppParser::AttributesContext::attribute(size_t i) {
  return getRuleContext<UVLcppParser::AttributeContext>(i);
}

std::vector<tree::TerminalNode *> UVLcppParser::AttributesContext::COMMA() {
  return getTokens(UVLcppParser::COMMA);
}

tree::TerminalNode* UVLcppParser::AttributesContext::COMMA(size_t i) {
  return getToken(UVLcppParser::COMMA, i);
}


size_t UVLcppParser::AttributesContext::getRuleIndex() const {
  return UVLcppParser::RuleAttributes;
}

antlrcpp::Any UVLcppParser::AttributesContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAttributes(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::AttributesContext* UVLcppParser::attributes() {
  AttributesContext *_localctx = _tracker.createInstance<AttributesContext>(_ctx, getState());
  enterRule(_localctx, 22, UVLcppParser::RuleAttributes);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(181);
    match(UVLcppParser::OPEN_BRACE);
    setState(190);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__6)
      | (1ULL << UVLcppParser::T__7)
      | (1ULL << UVLcppParser::ID_NOT_STRICT)
      | (1ULL << UVLcppParser::ID_STRICT))) != 0)) {
      setState(182);
      attribute();
      setState(187);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == UVLcppParser::COMMA) {
        setState(183);
        match(UVLcppParser::COMMA);
        setState(184);
        attribute();
        setState(189);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(192);
    match(UVLcppParser::CLOSE_BRACE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeContext ------------------------------------------------------------------

UVLcppParser::AttributeContext::AttributeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::ValueAttributeContext* UVLcppParser::AttributeContext::valueAttribute() {
  return getRuleContext<UVLcppParser::ValueAttributeContext>(0);
}

UVLcppParser::ConstraintAttributeContext* UVLcppParser::AttributeContext::constraintAttribute() {
  return getRuleContext<UVLcppParser::ConstraintAttributeContext>(0);
}


size_t UVLcppParser::AttributeContext::getRuleIndex() const {
  return UVLcppParser::RuleAttribute;
}

antlrcpp::Any UVLcppParser::AttributeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAttribute(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::AttributeContext* UVLcppParser::attribute() {
  AttributeContext *_localctx = _tracker.createInstance<AttributeContext>(_ctx, getState());
  enterRule(_localctx, 24, UVLcppParser::RuleAttribute);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(196);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::ID_NOT_STRICT:
      case UVLcppParser::ID_STRICT: {
        enterOuterAlt(_localctx, 1);
        setState(194);
        valueAttribute();
        break;
      }

      case UVLcppParser::T__6:
      case UVLcppParser::T__7: {
        enterOuterAlt(_localctx, 2);
        setState(195);
        constraintAttribute();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueAttributeContext ------------------------------------------------------------------

UVLcppParser::ValueAttributeContext::ValueAttributeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::KeyContext* UVLcppParser::ValueAttributeContext::key() {
  return getRuleContext<UVLcppParser::KeyContext>(0);
}

UVLcppParser::ValueContext* UVLcppParser::ValueAttributeContext::value() {
  return getRuleContext<UVLcppParser::ValueContext>(0);
}


size_t UVLcppParser::ValueAttributeContext::getRuleIndex() const {
  return UVLcppParser::RuleValueAttribute;
}

antlrcpp::Any UVLcppParser::ValueAttributeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitValueAttribute(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ValueAttributeContext* UVLcppParser::valueAttribute() {
  ValueAttributeContext *_localctx = _tracker.createInstance<ValueAttributeContext>(_ctx, getState());
  enterRule(_localctx, 26, UVLcppParser::RuleValueAttribute);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(198);
    key();
    setState(200);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::OPEN_BRACK)
      | (1ULL << UVLcppParser::OPEN_BRACE)
      | (1ULL << UVLcppParser::FLOAT)
      | (1ULL << UVLcppParser::INTEGER)
      | (1ULL << UVLcppParser::BOOLEAN)
      | (1ULL << UVLcppParser::STRING))) != 0)) {
      setState(199);
      value();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- KeyContext ------------------------------------------------------------------

UVLcppParser::KeyContext::KeyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::IdContext* UVLcppParser::KeyContext::id() {
  return getRuleContext<UVLcppParser::IdContext>(0);
}


size_t UVLcppParser::KeyContext::getRuleIndex() const {
  return UVLcppParser::RuleKey;
}

antlrcpp::Any UVLcppParser::KeyContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitKey(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::KeyContext* UVLcppParser::key() {
  KeyContext *_localctx = _tracker.createInstance<KeyContext>(_ctx, getState());
  enterRule(_localctx, 28, UVLcppParser::RuleKey);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(202);
    id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ValueContext ------------------------------------------------------------------

UVLcppParser::ValueContext::ValueContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::ValueContext::BOOLEAN() {
  return getToken(UVLcppParser::BOOLEAN, 0);
}

tree::TerminalNode* UVLcppParser::ValueContext::FLOAT() {
  return getToken(UVLcppParser::FLOAT, 0);
}

tree::TerminalNode* UVLcppParser::ValueContext::INTEGER() {
  return getToken(UVLcppParser::INTEGER, 0);
}

tree::TerminalNode* UVLcppParser::ValueContext::STRING() {
  return getToken(UVLcppParser::STRING, 0);
}

UVLcppParser::AttributesContext* UVLcppParser::ValueContext::attributes() {
  return getRuleContext<UVLcppParser::AttributesContext>(0);
}

UVLcppParser::VectorContext* UVLcppParser::ValueContext::vector() {
  return getRuleContext<UVLcppParser::VectorContext>(0);
}


size_t UVLcppParser::ValueContext::getRuleIndex() const {
  return UVLcppParser::RuleValue;
}

antlrcpp::Any UVLcppParser::ValueContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitValue(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ValueContext* UVLcppParser::value() {
  ValueContext *_localctx = _tracker.createInstance<ValueContext>(_ctx, getState());
  enterRule(_localctx, 30, UVLcppParser::RuleValue);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(210);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::BOOLEAN: {
        enterOuterAlt(_localctx, 1);
        setState(204);
        match(UVLcppParser::BOOLEAN);
        break;
      }

      case UVLcppParser::FLOAT: {
        enterOuterAlt(_localctx, 2);
        setState(205);
        match(UVLcppParser::FLOAT);
        break;
      }

      case UVLcppParser::INTEGER: {
        enterOuterAlt(_localctx, 3);
        setState(206);
        match(UVLcppParser::INTEGER);
        break;
      }

      case UVLcppParser::STRING: {
        enterOuterAlt(_localctx, 4);
        setState(207);
        match(UVLcppParser::STRING);
        break;
      }

      case UVLcppParser::OPEN_BRACE: {
        enterOuterAlt(_localctx, 5);
        setState(208);
        attributes();
        break;
      }

      case UVLcppParser::OPEN_BRACK: {
        enterOuterAlt(_localctx, 6);
        setState(209);
        vector();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VectorContext ------------------------------------------------------------------

UVLcppParser::VectorContext::VectorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::VectorContext::OPEN_BRACK() {
  return getToken(UVLcppParser::OPEN_BRACK, 0);
}

tree::TerminalNode* UVLcppParser::VectorContext::CLOSE_BRACK() {
  return getToken(UVLcppParser::CLOSE_BRACK, 0);
}

std::vector<UVLcppParser::ValueContext *> UVLcppParser::VectorContext::value() {
  return getRuleContexts<UVLcppParser::ValueContext>();
}

UVLcppParser::ValueContext* UVLcppParser::VectorContext::value(size_t i) {
  return getRuleContext<UVLcppParser::ValueContext>(i);
}

std::vector<tree::TerminalNode *> UVLcppParser::VectorContext::COMMA() {
  return getTokens(UVLcppParser::COMMA);
}

tree::TerminalNode* UVLcppParser::VectorContext::COMMA(size_t i) {
  return getToken(UVLcppParser::COMMA, i);
}


size_t UVLcppParser::VectorContext::getRuleIndex() const {
  return UVLcppParser::RuleVector;
}

antlrcpp::Any UVLcppParser::VectorContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitVector(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::VectorContext* UVLcppParser::vector() {
  VectorContext *_localctx = _tracker.createInstance<VectorContext>(_ctx, getState());
  enterRule(_localctx, 32, UVLcppParser::RuleVector);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(212);
    match(UVLcppParser::OPEN_BRACK);
    setState(221);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::OPEN_BRACK)
      | (1ULL << UVLcppParser::OPEN_BRACE)
      | (1ULL << UVLcppParser::FLOAT)
      | (1ULL << UVLcppParser::INTEGER)
      | (1ULL << UVLcppParser::BOOLEAN)
      | (1ULL << UVLcppParser::STRING))) != 0)) {
      setState(213);
      value();
      setState(218);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == UVLcppParser::COMMA) {
        setState(214);
        match(UVLcppParser::COMMA);
        setState(215);
        value();
        setState(220);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(223);
    match(UVLcppParser::CLOSE_BRACK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstraintAttributeContext ------------------------------------------------------------------

UVLcppParser::ConstraintAttributeContext::ConstraintAttributeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::ConstraintAttributeContext::getRuleIndex() const {
  return UVLcppParser::RuleConstraintAttribute;
}

void UVLcppParser::ConstraintAttributeContext::copyFrom(ConstraintAttributeContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- ListConstraintAttributeContext ------------------------------------------------------------------

UVLcppParser::ConstraintListContext* UVLcppParser::ListConstraintAttributeContext::constraintList() {
  return getRuleContext<UVLcppParser::ConstraintListContext>(0);
}

UVLcppParser::ListConstraintAttributeContext::ListConstraintAttributeContext(ConstraintAttributeContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::ListConstraintAttributeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitListConstraintAttribute(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SingleConstraintAttributeContext ------------------------------------------------------------------

UVLcppParser::ConstraintContext* UVLcppParser::SingleConstraintAttributeContext::constraint() {
  return getRuleContext<UVLcppParser::ConstraintContext>(0);
}

UVLcppParser::SingleConstraintAttributeContext::SingleConstraintAttributeContext(ConstraintAttributeContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::SingleConstraintAttributeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitSingleConstraintAttribute(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::ConstraintAttributeContext* UVLcppParser::constraintAttribute() {
  ConstraintAttributeContext *_localctx = _tracker.createInstance<ConstraintAttributeContext>(_ctx, getState());
  enterRule(_localctx, 34, UVLcppParser::RuleConstraintAttribute);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(229);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::T__6: {
        _localctx = dynamic_cast<ConstraintAttributeContext *>(_tracker.createInstance<UVLcppParser::SingleConstraintAttributeContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(225);
        match(UVLcppParser::T__6);
        setState(226);
        constraint(0);
        break;
      }

      case UVLcppParser::T__7: {
        _localctx = dynamic_cast<ConstraintAttributeContext *>(_tracker.createInstance<UVLcppParser::ListConstraintAttributeContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(227);
        match(UVLcppParser::T__7);
        setState(228);
        constraintList();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstraintListContext ------------------------------------------------------------------

UVLcppParser::ConstraintListContext::ConstraintListContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::ConstraintListContext::OPEN_BRACK() {
  return getToken(UVLcppParser::OPEN_BRACK, 0);
}

tree::TerminalNode* UVLcppParser::ConstraintListContext::CLOSE_BRACK() {
  return getToken(UVLcppParser::CLOSE_BRACK, 0);
}

std::vector<UVLcppParser::ConstraintContext *> UVLcppParser::ConstraintListContext::constraint() {
  return getRuleContexts<UVLcppParser::ConstraintContext>();
}

UVLcppParser::ConstraintContext* UVLcppParser::ConstraintListContext::constraint(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintContext>(i);
}

std::vector<tree::TerminalNode *> UVLcppParser::ConstraintListContext::COMMA() {
  return getTokens(UVLcppParser::COMMA);
}

tree::TerminalNode* UVLcppParser::ConstraintListContext::COMMA(size_t i) {
  return getToken(UVLcppParser::COMMA, i);
}


size_t UVLcppParser::ConstraintListContext::getRuleIndex() const {
  return UVLcppParser::RuleConstraintList;
}

antlrcpp::Any UVLcppParser::ConstraintListContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitConstraintList(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ConstraintListContext* UVLcppParser::constraintList() {
  ConstraintListContext *_localctx = _tracker.createInstance<ConstraintListContext>(_ctx, getState());
  enterRule(_localctx, 36, UVLcppParser::RuleConstraintList);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(231);
    match(UVLcppParser::OPEN_BRACK);
    setState(240);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__8)
      | (1ULL << UVLcppParser::T__9)
      | (1ULL << UVLcppParser::T__10)
      | (1ULL << UVLcppParser::T__11)
      | (1ULL << UVLcppParser::T__12)
      | (1ULL << UVLcppParser::OPEN_PAREN)
      | (1ULL << UVLcppParser::NOT)
      | (1ULL << UVLcppParser::FLOAT)
      | (1ULL << UVLcppParser::INTEGER)
      | (1ULL << UVLcppParser::ID_NOT_STRICT)
      | (1ULL << UVLcppParser::ID_STRICT)
      | (1ULL << UVLcppParser::STRING))) != 0)) {
      setState(232);
      constraint(0);
      setState(237);
      _errHandler->sync(this);
      _la = _input->LA(1);
      while (_la == UVLcppParser::COMMA) {
        setState(233);
        match(UVLcppParser::COMMA);
        setState(234);
        constraint(0);
        setState(239);
        _errHandler->sync(this);
        _la = _input->LA(1);
      }
    }
    setState(242);
    match(UVLcppParser::CLOSE_BRACK);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstraintsContext ------------------------------------------------------------------

UVLcppParser::ConstraintsContext::ConstraintsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::ConstraintsContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}

tree::TerminalNode* UVLcppParser::ConstraintsContext::INDENT() {
  return getToken(UVLcppParser::INDENT, 0);
}

tree::TerminalNode* UVLcppParser::ConstraintsContext::DEDENT() {
  return getToken(UVLcppParser::DEDENT, 0);
}

std::vector<UVLcppParser::ConstraintLineContext *> UVLcppParser::ConstraintsContext::constraintLine() {
  return getRuleContexts<UVLcppParser::ConstraintLineContext>();
}

UVLcppParser::ConstraintLineContext* UVLcppParser::ConstraintsContext::constraintLine(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintLineContext>(i);
}


size_t UVLcppParser::ConstraintsContext::getRuleIndex() const {
  return UVLcppParser::RuleConstraints;
}

antlrcpp::Any UVLcppParser::ConstraintsContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitConstraints(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ConstraintsContext* UVLcppParser::constraints() {
  ConstraintsContext *_localctx = _tracker.createInstance<ConstraintsContext>(_ctx, getState());
  enterRule(_localctx, 38, UVLcppParser::RuleConstraints);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(244);
    match(UVLcppParser::T__7);
    setState(245);
    match(UVLcppParser::NEWLINE);
    setState(246);
    match(UVLcppParser::INDENT);
    setState(250);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__8)
      | (1ULL << UVLcppParser::T__9)
      | (1ULL << UVLcppParser::T__10)
      | (1ULL << UVLcppParser::T__11)
      | (1ULL << UVLcppParser::T__12)
      | (1ULL << UVLcppParser::OPEN_PAREN)
      | (1ULL << UVLcppParser::NOT)
      | (1ULL << UVLcppParser::FLOAT)
      | (1ULL << UVLcppParser::INTEGER)
      | (1ULL << UVLcppParser::ID_NOT_STRICT)
      | (1ULL << UVLcppParser::ID_STRICT)
      | (1ULL << UVLcppParser::STRING))) != 0)) {
      setState(247);
      constraintLine();
      setState(252);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(253);
    match(UVLcppParser::DEDENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstraintLineContext ------------------------------------------------------------------

UVLcppParser::ConstraintLineContext::ConstraintLineContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::ConstraintContext* UVLcppParser::ConstraintLineContext::constraint() {
  return getRuleContext<UVLcppParser::ConstraintContext>(0);
}

tree::TerminalNode* UVLcppParser::ConstraintLineContext::NEWLINE() {
  return getToken(UVLcppParser::NEWLINE, 0);
}


size_t UVLcppParser::ConstraintLineContext::getRuleIndex() const {
  return UVLcppParser::RuleConstraintLine;
}

antlrcpp::Any UVLcppParser::ConstraintLineContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitConstraintLine(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ConstraintLineContext* UVLcppParser::constraintLine() {
  ConstraintLineContext *_localctx = _tracker.createInstance<ConstraintLineContext>(_ctx, getState());
  enterRule(_localctx, 40, UVLcppParser::RuleConstraintLine);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(255);
    constraint(0);
    setState(256);
    match(UVLcppParser::NEWLINE);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConstraintContext ------------------------------------------------------------------

UVLcppParser::ConstraintContext::ConstraintContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::ConstraintContext::getRuleIndex() const {
  return UVLcppParser::RuleConstraint;
}

void UVLcppParser::ConstraintContext::copyFrom(ConstraintContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- OrConstraintContext ------------------------------------------------------------------

std::vector<UVLcppParser::ConstraintContext *> UVLcppParser::OrConstraintContext::constraint() {
  return getRuleContexts<UVLcppParser::ConstraintContext>();
}

UVLcppParser::ConstraintContext* UVLcppParser::OrConstraintContext::constraint(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintContext>(i);
}

tree::TerminalNode* UVLcppParser::OrConstraintContext::OR() {
  return getToken(UVLcppParser::OR, 0);
}

UVLcppParser::OrConstraintContext::OrConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::OrConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitOrConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EquationConstraintContext ------------------------------------------------------------------

UVLcppParser::EquationContext* UVLcppParser::EquationConstraintContext::equation() {
  return getRuleContext<UVLcppParser::EquationContext>(0);
}

UVLcppParser::EquationConstraintContext::EquationConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::EquationConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitEquationConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LiteralConstraintContext ------------------------------------------------------------------

UVLcppParser::ReferenceContext* UVLcppParser::LiteralConstraintContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

UVLcppParser::LiteralConstraintContext::LiteralConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::LiteralConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLiteralConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ParenthesisConstraintContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::ParenthesisConstraintContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

UVLcppParser::ConstraintContext* UVLcppParser::ParenthesisConstraintContext::constraint() {
  return getRuleContext<UVLcppParser::ConstraintContext>(0);
}

tree::TerminalNode* UVLcppParser::ParenthesisConstraintContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

UVLcppParser::ParenthesisConstraintContext::ParenthesisConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::ParenthesisConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitParenthesisConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotConstraintContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::NotConstraintContext::NOT() {
  return getToken(UVLcppParser::NOT, 0);
}

UVLcppParser::ConstraintContext* UVLcppParser::NotConstraintContext::constraint() {
  return getRuleContext<UVLcppParser::ConstraintContext>(0);
}

UVLcppParser::NotConstraintContext::NotConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::NotConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitNotConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AndConstraintContext ------------------------------------------------------------------

std::vector<UVLcppParser::ConstraintContext *> UVLcppParser::AndConstraintContext::constraint() {
  return getRuleContexts<UVLcppParser::ConstraintContext>();
}

UVLcppParser::ConstraintContext* UVLcppParser::AndConstraintContext::constraint(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintContext>(i);
}

tree::TerminalNode* UVLcppParser::AndConstraintContext::AND() {
  return getToken(UVLcppParser::AND, 0);
}

UVLcppParser::AndConstraintContext::AndConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::AndConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAndConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- EquivalenceConstraintContext ------------------------------------------------------------------

std::vector<UVLcppParser::ConstraintContext *> UVLcppParser::EquivalenceConstraintContext::constraint() {
  return getRuleContexts<UVLcppParser::ConstraintContext>();
}

UVLcppParser::ConstraintContext* UVLcppParser::EquivalenceConstraintContext::constraint(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintContext>(i);
}

tree::TerminalNode* UVLcppParser::EquivalenceConstraintContext::EQUIVALENCE() {
  return getToken(UVLcppParser::EQUIVALENCE, 0);
}

UVLcppParser::EquivalenceConstraintContext::EquivalenceConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::EquivalenceConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitEquivalenceConstraint(this);
  else
    return visitor->visitChildren(this);
}
//----------------- ImplicationConstraintContext ------------------------------------------------------------------

std::vector<UVLcppParser::ConstraintContext *> UVLcppParser::ImplicationConstraintContext::constraint() {
  return getRuleContexts<UVLcppParser::ConstraintContext>();
}

UVLcppParser::ConstraintContext* UVLcppParser::ImplicationConstraintContext::constraint(size_t i) {
  return getRuleContext<UVLcppParser::ConstraintContext>(i);
}

tree::TerminalNode* UVLcppParser::ImplicationConstraintContext::IMPLICATION() {
  return getToken(UVLcppParser::IMPLICATION, 0);
}

UVLcppParser::ImplicationConstraintContext::ImplicationConstraintContext(ConstraintContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::ImplicationConstraintContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitImplicationConstraint(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ConstraintContext* UVLcppParser::constraint() {
   return constraint(0);
}

UVLcppParser::ConstraintContext* UVLcppParser::constraint(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  UVLcppParser::ConstraintContext *_localctx = _tracker.createInstance<ConstraintContext>(_ctx, parentState);
  UVLcppParser::ConstraintContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 42;
  enterRecursionRule(_localctx, 42, UVLcppParser::RuleConstraint, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(267);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 30, _ctx)) {
    case 1: {
      _localctx = _tracker.createInstance<EquationConstraintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;

      setState(259);
      equation();
      break;
    }

    case 2: {
      _localctx = _tracker.createInstance<LiteralConstraintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(260);
      reference();
      break;
    }

    case 3: {
      _localctx = _tracker.createInstance<ParenthesisConstraintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(261);
      match(UVLcppParser::OPEN_PAREN);
      setState(262);
      constraint(0);
      setState(263);
      match(UVLcppParser::CLOSE_PAREN);
      break;
    }

    case 4: {
      _localctx = _tracker.createInstance<NotConstraintContext>(_localctx);
      _ctx = _localctx;
      previousContext = _localctx;
      setState(265);
      match(UVLcppParser::NOT);
      setState(266);
      constraint(5);
      break;
    }

    default:
      break;
    }
    _ctx->stop = _input->LT(-1);
    setState(283);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(281);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 31, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<AndConstraintContext>(_tracker.createInstance<ConstraintContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleConstraint);
          setState(269);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(270);
          match(UVLcppParser::AND);
          setState(271);
          constraint(5);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<OrConstraintContext>(_tracker.createInstance<ConstraintContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleConstraint);
          setState(272);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(273);
          match(UVLcppParser::OR);
          setState(274);
          constraint(4);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<ImplicationConstraintContext>(_tracker.createInstance<ConstraintContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleConstraint);
          setState(275);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(276);
          match(UVLcppParser::IMPLICATION);
          setState(277);
          constraint(3);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<EquivalenceConstraintContext>(_tracker.createInstance<ConstraintContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleConstraint);
          setState(278);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(279);
          match(UVLcppParser::EQUIVALENCE);
          setState(280);
          constraint(2);
          break;
        }

        default:
          break;
        } 
      }
      setState(285);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 32, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- EquationContext ------------------------------------------------------------------

UVLcppParser::EquationContext::EquationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::EquationContext::getRuleIndex() const {
  return UVLcppParser::RuleEquation;
}

void UVLcppParser::EquationContext::copyFrom(EquationContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- EqualEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::EqualEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::EqualEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::EqualEquationContext::EQUAL() {
  return getToken(UVLcppParser::EQUAL, 0);
}

UVLcppParser::EqualEquationContext::EqualEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::EqualEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitEqualEquation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LowerEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::LowerEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::LowerEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::LowerEquationContext::LOWER() {
  return getToken(UVLcppParser::LOWER, 0);
}

UVLcppParser::LowerEquationContext::LowerEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::LowerEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLowerEquation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LowerEqualsEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::LowerEqualsEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::LowerEqualsEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::LowerEqualsEquationContext::LOWER_EQUALS() {
  return getToken(UVLcppParser::LOWER_EQUALS, 0);
}

UVLcppParser::LowerEqualsEquationContext::LowerEqualsEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::LowerEqualsEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLowerEqualsEquation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GreaterEqualsEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::GreaterEqualsEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::GreaterEqualsEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::GreaterEqualsEquationContext::GREATER_EQUALS() {
  return getToken(UVLcppParser::GREATER_EQUALS, 0);
}

UVLcppParser::GreaterEqualsEquationContext::GreaterEqualsEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::GreaterEqualsEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitGreaterEqualsEquation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- GreaterEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::GreaterEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::GreaterEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::GreaterEquationContext::GREATER() {
  return getToken(UVLcppParser::GREATER, 0);
}

UVLcppParser::GreaterEquationContext::GreaterEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::GreaterEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitGreaterEquation(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NotEqualsEquationContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::NotEqualsEquationContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::NotEqualsEquationContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::NotEqualsEquationContext::NOT_EQUALS() {
  return getToken(UVLcppParser::NOT_EQUALS, 0);
}

UVLcppParser::NotEqualsEquationContext::NotEqualsEquationContext(EquationContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::NotEqualsEquationContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitNotEqualsEquation(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::EquationContext* UVLcppParser::equation() {
  EquationContext *_localctx = _tracker.createInstance<EquationContext>(_ctx, getState());
  enterRule(_localctx, 44, UVLcppParser::RuleEquation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(310);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 33, _ctx)) {
    case 1: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::EqualEquationContext>(_localctx));
      enterOuterAlt(_localctx, 1);
      setState(286);
      expression(0);
      setState(287);
      match(UVLcppParser::EQUAL);
      setState(288);
      expression(0);
      break;
    }

    case 2: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::LowerEquationContext>(_localctx));
      enterOuterAlt(_localctx, 2);
      setState(290);
      expression(0);
      setState(291);
      match(UVLcppParser::LOWER);
      setState(292);
      expression(0);
      break;
    }

    case 3: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::GreaterEquationContext>(_localctx));
      enterOuterAlt(_localctx, 3);
      setState(294);
      expression(0);
      setState(295);
      match(UVLcppParser::GREATER);
      setState(296);
      expression(0);
      break;
    }

    case 4: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::LowerEqualsEquationContext>(_localctx));
      enterOuterAlt(_localctx, 4);
      setState(298);
      expression(0);
      setState(299);
      match(UVLcppParser::LOWER_EQUALS);
      setState(300);
      expression(0);
      break;
    }

    case 5: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::GreaterEqualsEquationContext>(_localctx));
      enterOuterAlt(_localctx, 5);
      setState(302);
      expression(0);
      setState(303);
      match(UVLcppParser::GREATER_EQUALS);
      setState(304);
      expression(0);
      break;
    }

    case 6: {
      _localctx = dynamic_cast<EquationContext *>(_tracker.createInstance<UVLcppParser::NotEqualsEquationContext>(_localctx));
      enterOuterAlt(_localctx, 6);
      setState(306);
      expression(0);
      setState(307);
      match(UVLcppParser::NOT_EQUALS);
      setState(308);
      expression(0);
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ExpressionContext ------------------------------------------------------------------

UVLcppParser::ExpressionContext::ExpressionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::ExpressionContext::getRuleIndex() const {
  return UVLcppParser::RuleExpression;
}

void UVLcppParser::ExpressionContext::copyFrom(ExpressionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- BracketExpressionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::BracketExpressionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

UVLcppParser::ExpressionContext* UVLcppParser::BracketExpressionContext::expression() {
  return getRuleContext<UVLcppParser::ExpressionContext>(0);
}

tree::TerminalNode* UVLcppParser::BracketExpressionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

UVLcppParser::BracketExpressionContext::BracketExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::BracketExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitBracketExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AggregateFunctionExpressionContext ------------------------------------------------------------------

UVLcppParser::AggregateFunctionContext* UVLcppParser::AggregateFunctionExpressionContext::aggregateFunction() {
  return getRuleContext<UVLcppParser::AggregateFunctionContext>(0);
}

UVLcppParser::AggregateFunctionExpressionContext::AggregateFunctionExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::AggregateFunctionExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAggregateFunctionExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FloatLiteralExpressionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::FloatLiteralExpressionContext::FLOAT() {
  return getToken(UVLcppParser::FLOAT, 0);
}

UVLcppParser::FloatLiteralExpressionContext::FloatLiteralExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::FloatLiteralExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFloatLiteralExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- StringLiteralExpressionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::StringLiteralExpressionContext::STRING() {
  return getToken(UVLcppParser::STRING, 0);
}

UVLcppParser::StringLiteralExpressionContext::StringLiteralExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::StringLiteralExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitStringLiteralExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- AddExpressionContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::AddExpressionContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::AddExpressionContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::AddExpressionContext::ADD() {
  return getToken(UVLcppParser::ADD, 0);
}

UVLcppParser::AddExpressionContext::AddExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::AddExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAddExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- IntegerLiteralExpressionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::IntegerLiteralExpressionContext::INTEGER() {
  return getToken(UVLcppParser::INTEGER, 0);
}

UVLcppParser::IntegerLiteralExpressionContext::IntegerLiteralExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::IntegerLiteralExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitIntegerLiteralExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- LiteralExpressionContext ------------------------------------------------------------------

UVLcppParser::ReferenceContext* UVLcppParser::LiteralExpressionContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

UVLcppParser::LiteralExpressionContext::LiteralExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::LiteralExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLiteralExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- DivExpressionContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::DivExpressionContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::DivExpressionContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::DivExpressionContext::DIV() {
  return getToken(UVLcppParser::DIV, 0);
}

UVLcppParser::DivExpressionContext::DivExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::DivExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitDivExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SubExpressionContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::SubExpressionContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::SubExpressionContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::SubExpressionContext::SUB() {
  return getToken(UVLcppParser::SUB, 0);
}

UVLcppParser::SubExpressionContext::SubExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::SubExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitSubExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- MulExpressionContext ------------------------------------------------------------------

std::vector<UVLcppParser::ExpressionContext *> UVLcppParser::MulExpressionContext::expression() {
  return getRuleContexts<UVLcppParser::ExpressionContext>();
}

UVLcppParser::ExpressionContext* UVLcppParser::MulExpressionContext::expression(size_t i) {
  return getRuleContext<UVLcppParser::ExpressionContext>(i);
}

tree::TerminalNode* UVLcppParser::MulExpressionContext::MUL() {
  return getToken(UVLcppParser::MUL, 0);
}

UVLcppParser::MulExpressionContext::MulExpressionContext(ExpressionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::MulExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitMulExpression(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ExpressionContext* UVLcppParser::expression() {
   return expression(0);
}

UVLcppParser::ExpressionContext* UVLcppParser::expression(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  UVLcppParser::ExpressionContext *_localctx = _tracker.createInstance<ExpressionContext>(_ctx, parentState);
  UVLcppParser::ExpressionContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 46;
  enterRecursionRule(_localctx, 46, UVLcppParser::RuleExpression, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(322);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::FLOAT: {
        _localctx = _tracker.createInstance<FloatLiteralExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;

        setState(313);
        match(UVLcppParser::FLOAT);
        break;
      }

      case UVLcppParser::INTEGER: {
        _localctx = _tracker.createInstance<IntegerLiteralExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(314);
        match(UVLcppParser::INTEGER);
        break;
      }

      case UVLcppParser::STRING: {
        _localctx = _tracker.createInstance<StringLiteralExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(315);
        match(UVLcppParser::STRING);
        break;
      }

      case UVLcppParser::T__8:
      case UVLcppParser::T__9:
      case UVLcppParser::T__10:
      case UVLcppParser::T__11:
      case UVLcppParser::T__12: {
        _localctx = _tracker.createInstance<AggregateFunctionExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(316);
        aggregateFunction();
        break;
      }

      case UVLcppParser::ID_NOT_STRICT:
      case UVLcppParser::ID_STRICT: {
        _localctx = _tracker.createInstance<LiteralExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(317);
        reference();
        break;
      }

      case UVLcppParser::OPEN_PAREN: {
        _localctx = _tracker.createInstance<BracketExpressionContext>(_localctx);
        _ctx = _localctx;
        previousContext = _localctx;
        setState(318);
        match(UVLcppParser::OPEN_PAREN);
        setState(319);
        expression(0);
        setState(320);
        match(UVLcppParser::CLOSE_PAREN);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(338);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        setState(336);
        _errHandler->sync(this);
        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 35, _ctx)) {
        case 1: {
          auto newContext = _tracker.createInstance<AddExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(324);

          if (!(precpred(_ctx, 4))) throw FailedPredicateException(this, "precpred(_ctx, 4)");
          setState(325);
          match(UVLcppParser::ADD);
          setState(326);
          expression(5);
          break;
        }

        case 2: {
          auto newContext = _tracker.createInstance<SubExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(327);

          if (!(precpred(_ctx, 3))) throw FailedPredicateException(this, "precpred(_ctx, 3)");
          setState(328);
          match(UVLcppParser::SUB);
          setState(329);
          expression(4);
          break;
        }

        case 3: {
          auto newContext = _tracker.createInstance<MulExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(330);

          if (!(precpred(_ctx, 2))) throw FailedPredicateException(this, "precpred(_ctx, 2)");
          setState(331);
          match(UVLcppParser::MUL);
          setState(332);
          expression(3);
          break;
        }

        case 4: {
          auto newContext = _tracker.createInstance<DivExpressionContext>(_tracker.createInstance<ExpressionContext>(parentContext, parentState));
          _localctx = newContext;
          pushNewRecursionContext(newContext, startState, RuleExpression);
          setState(333);

          if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
          setState(334);
          match(UVLcppParser::DIV);
          setState(335);
          expression(2);
          break;
        }

        default:
          break;
        } 
      }
      setState(340);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 36, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- AggregateFunctionContext ------------------------------------------------------------------

UVLcppParser::AggregateFunctionContext::AggregateFunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::AggregateFunctionContext::getRuleIndex() const {
  return UVLcppParser::RuleAggregateFunction;
}

void UVLcppParser::AggregateFunctionContext::copyFrom(AggregateFunctionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- AvgAggregateFunctionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::AvgAggregateFunctionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

std::vector<UVLcppParser::ReferenceContext *> UVLcppParser::AvgAggregateFunctionContext::reference() {
  return getRuleContexts<UVLcppParser::ReferenceContext>();
}

UVLcppParser::ReferenceContext* UVLcppParser::AvgAggregateFunctionContext::reference(size_t i) {
  return getRuleContext<UVLcppParser::ReferenceContext>(i);
}

tree::TerminalNode* UVLcppParser::AvgAggregateFunctionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

tree::TerminalNode* UVLcppParser::AvgAggregateFunctionContext::COMMA() {
  return getToken(UVLcppParser::COMMA, 0);
}

UVLcppParser::AvgAggregateFunctionContext::AvgAggregateFunctionContext(AggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::AvgAggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitAvgAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}
//----------------- NumericAggregateFunctionExpressionContext ------------------------------------------------------------------

UVLcppParser::NumericAggregateFunctionContext* UVLcppParser::NumericAggregateFunctionExpressionContext::numericAggregateFunction() {
  return getRuleContext<UVLcppParser::NumericAggregateFunctionContext>(0);
}

UVLcppParser::NumericAggregateFunctionExpressionContext::NumericAggregateFunctionExpressionContext(AggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::NumericAggregateFunctionExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitNumericAggregateFunctionExpression(this);
  else
    return visitor->visitChildren(this);
}
//----------------- SumAggregateFunctionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::SumAggregateFunctionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

std::vector<UVLcppParser::ReferenceContext *> UVLcppParser::SumAggregateFunctionContext::reference() {
  return getRuleContexts<UVLcppParser::ReferenceContext>();
}

UVLcppParser::ReferenceContext* UVLcppParser::SumAggregateFunctionContext::reference(size_t i) {
  return getRuleContext<UVLcppParser::ReferenceContext>(i);
}

tree::TerminalNode* UVLcppParser::SumAggregateFunctionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

tree::TerminalNode* UVLcppParser::SumAggregateFunctionContext::COMMA() {
  return getToken(UVLcppParser::COMMA, 0);
}

UVLcppParser::SumAggregateFunctionContext::SumAggregateFunctionContext(AggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::SumAggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitSumAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}
//----------------- StringAggregateFunctionExpressionContext ------------------------------------------------------------------

UVLcppParser::StringAggregateFunctionContext* UVLcppParser::StringAggregateFunctionExpressionContext::stringAggregateFunction() {
  return getRuleContext<UVLcppParser::StringAggregateFunctionContext>(0);
}

UVLcppParser::StringAggregateFunctionExpressionContext::StringAggregateFunctionExpressionContext(AggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::StringAggregateFunctionExpressionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitStringAggregateFunctionExpression(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::AggregateFunctionContext* UVLcppParser::aggregateFunction() {
  AggregateFunctionContext *_localctx = _tracker.createInstance<AggregateFunctionContext>(_ctx, getState());
  enterRule(_localctx, 48, UVLcppParser::RuleAggregateFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(363);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::T__8: {
        _localctx = dynamic_cast<AggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::SumAggregateFunctionContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(341);
        match(UVLcppParser::T__8);
        setState(342);
        match(UVLcppParser::OPEN_PAREN);
        setState(346);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 37, _ctx)) {
        case 1: {
          setState(343);
          reference();
          setState(344);
          match(UVLcppParser::COMMA);
          break;
        }

        default:
          break;
        }
        setState(348);
        reference();
        setState(349);
        match(UVLcppParser::CLOSE_PAREN);
        break;
      }

      case UVLcppParser::T__9: {
        _localctx = dynamic_cast<AggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::AvgAggregateFunctionContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(351);
        match(UVLcppParser::T__9);
        setState(352);
        match(UVLcppParser::OPEN_PAREN);
        setState(356);
        _errHandler->sync(this);

        switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 38, _ctx)) {
        case 1: {
          setState(353);
          reference();
          setState(354);
          match(UVLcppParser::COMMA);
          break;
        }

        default:
          break;
        }
        setState(358);
        reference();
        setState(359);
        match(UVLcppParser::CLOSE_PAREN);
        break;
      }

      case UVLcppParser::T__10: {
        _localctx = dynamic_cast<AggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::StringAggregateFunctionExpressionContext>(_localctx));
        enterOuterAlt(_localctx, 3);
        setState(361);
        stringAggregateFunction();
        break;
      }

      case UVLcppParser::T__11:
      case UVLcppParser::T__12: {
        _localctx = dynamic_cast<AggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::NumericAggregateFunctionExpressionContext>(_localctx));
        enterOuterAlt(_localctx, 4);
        setState(362);
        numericAggregateFunction();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StringAggregateFunctionContext ------------------------------------------------------------------

UVLcppParser::StringAggregateFunctionContext::StringAggregateFunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::StringAggregateFunctionContext::getRuleIndex() const {
  return UVLcppParser::RuleStringAggregateFunction;
}

void UVLcppParser::StringAggregateFunctionContext::copyFrom(StringAggregateFunctionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- LengthAggregateFunctionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::LengthAggregateFunctionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

UVLcppParser::ReferenceContext* UVLcppParser::LengthAggregateFunctionContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

tree::TerminalNode* UVLcppParser::LengthAggregateFunctionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

UVLcppParser::LengthAggregateFunctionContext::LengthAggregateFunctionContext(StringAggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::LengthAggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLengthAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::StringAggregateFunctionContext* UVLcppParser::stringAggregateFunction() {
  StringAggregateFunctionContext *_localctx = _tracker.createInstance<StringAggregateFunctionContext>(_ctx, getState());
  enterRule(_localctx, 50, UVLcppParser::RuleStringAggregateFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    _localctx = dynamic_cast<StringAggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::LengthAggregateFunctionContext>(_localctx));
    enterOuterAlt(_localctx, 1);
    setState(365);
    match(UVLcppParser::T__10);
    setState(366);
    match(UVLcppParser::OPEN_PAREN);
    setState(367);
    reference();
    setState(368);
    match(UVLcppParser::CLOSE_PAREN);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NumericAggregateFunctionContext ------------------------------------------------------------------

UVLcppParser::NumericAggregateFunctionContext::NumericAggregateFunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::NumericAggregateFunctionContext::getRuleIndex() const {
  return UVLcppParser::RuleNumericAggregateFunction;
}

void UVLcppParser::NumericAggregateFunctionContext::copyFrom(NumericAggregateFunctionContext *ctx) {
  ParserRuleContext::copyFrom(ctx);
}

//----------------- CeilAggregateFunctionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::CeilAggregateFunctionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

UVLcppParser::ReferenceContext* UVLcppParser::CeilAggregateFunctionContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

tree::TerminalNode* UVLcppParser::CeilAggregateFunctionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

UVLcppParser::CeilAggregateFunctionContext::CeilAggregateFunctionContext(NumericAggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::CeilAggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitCeilAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}
//----------------- FloorAggregateFunctionContext ------------------------------------------------------------------

tree::TerminalNode* UVLcppParser::FloorAggregateFunctionContext::OPEN_PAREN() {
  return getToken(UVLcppParser::OPEN_PAREN, 0);
}

UVLcppParser::ReferenceContext* UVLcppParser::FloorAggregateFunctionContext::reference() {
  return getRuleContext<UVLcppParser::ReferenceContext>(0);
}

tree::TerminalNode* UVLcppParser::FloorAggregateFunctionContext::CLOSE_PAREN() {
  return getToken(UVLcppParser::CLOSE_PAREN, 0);
}

UVLcppParser::FloorAggregateFunctionContext::FloorAggregateFunctionContext(NumericAggregateFunctionContext *ctx) { copyFrom(ctx); }

antlrcpp::Any UVLcppParser::FloorAggregateFunctionContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFloorAggregateFunction(this);
  else
    return visitor->visitChildren(this);
}
UVLcppParser::NumericAggregateFunctionContext* UVLcppParser::numericAggregateFunction() {
  NumericAggregateFunctionContext *_localctx = _tracker.createInstance<NumericAggregateFunctionContext>(_ctx, getState());
  enterRule(_localctx, 52, UVLcppParser::RuleNumericAggregateFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(380);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case UVLcppParser::T__11: {
        _localctx = dynamic_cast<NumericAggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::FloorAggregateFunctionContext>(_localctx));
        enterOuterAlt(_localctx, 1);
        setState(370);
        match(UVLcppParser::T__11);
        setState(371);
        match(UVLcppParser::OPEN_PAREN);
        setState(372);
        reference();
        setState(373);
        match(UVLcppParser::CLOSE_PAREN);
        break;
      }

      case UVLcppParser::T__12: {
        _localctx = dynamic_cast<NumericAggregateFunctionContext *>(_tracker.createInstance<UVLcppParser::CeilAggregateFunctionContext>(_localctx));
        enterOuterAlt(_localctx, 2);
        setState(375);
        match(UVLcppParser::T__12);
        setState(376);
        match(UVLcppParser::OPEN_PAREN);
        setState(377);
        reference();
        setState(378);
        match(UVLcppParser::CLOSE_PAREN);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReferenceContext ------------------------------------------------------------------

UVLcppParser::ReferenceContext::ReferenceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<UVLcppParser::IdContext *> UVLcppParser::ReferenceContext::id() {
  return getRuleContexts<UVLcppParser::IdContext>();
}

UVLcppParser::IdContext* UVLcppParser::ReferenceContext::id(size_t i) {
  return getRuleContext<UVLcppParser::IdContext>(i);
}


size_t UVLcppParser::ReferenceContext::getRuleIndex() const {
  return UVLcppParser::RuleReference;
}

antlrcpp::Any UVLcppParser::ReferenceContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitReference(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::ReferenceContext* UVLcppParser::reference() {
  ReferenceContext *_localctx = _tracker.createInstance<ReferenceContext>(_ctx, getState());
  enterRule(_localctx, 54, UVLcppParser::RuleReference);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(387);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(382);
        id();
        setState(383);
        match(UVLcppParser::T__13); 
      }
      setState(389);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 41, _ctx);
    }
    setState(390);
    id();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- IdContext ------------------------------------------------------------------

UVLcppParser::IdContext::IdContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::IdContext::ID_STRICT() {
  return getToken(UVLcppParser::ID_STRICT, 0);
}

tree::TerminalNode* UVLcppParser::IdContext::ID_NOT_STRICT() {
  return getToken(UVLcppParser::ID_NOT_STRICT, 0);
}


size_t UVLcppParser::IdContext::getRuleIndex() const {
  return UVLcppParser::RuleId;
}

antlrcpp::Any UVLcppParser::IdContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitId(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::IdContext* UVLcppParser::id() {
  IdContext *_localctx = _tracker.createInstance<IdContext>(_ctx, getState());
  enterRule(_localctx, 56, UVLcppParser::RuleId);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(392);
    _la = _input->LA(1);
    if (!(_la == UVLcppParser::ID_NOT_STRICT

    || _la == UVLcppParser::ID_STRICT)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FeatureTypeContext ------------------------------------------------------------------

UVLcppParser::FeatureTypeContext::FeatureTypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::FeatureTypeContext::BOOLEAN_KEY() {
  return getToken(UVLcppParser::BOOLEAN_KEY, 0);
}


size_t UVLcppParser::FeatureTypeContext::getRuleIndex() const {
  return UVLcppParser::RuleFeatureType;
}

antlrcpp::Any UVLcppParser::FeatureTypeContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitFeatureType(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::FeatureTypeContext* UVLcppParser::featureType() {
  FeatureTypeContext *_localctx = _tracker.createInstance<FeatureTypeContext>(_ctx, getState());
  enterRule(_localctx, 58, UVLcppParser::RuleFeatureType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(394);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__14)
      | (1ULL << UVLcppParser::T__15)
      | (1ULL << UVLcppParser::T__16)
      | (1ULL << UVLcppParser::BOOLEAN_KEY))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- LanguageLevelContext ------------------------------------------------------------------

UVLcppParser::LanguageLevelContext::LanguageLevelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

UVLcppParser::MajorLevelContext* UVLcppParser::LanguageLevelContext::majorLevel() {
  return getRuleContext<UVLcppParser::MajorLevelContext>(0);
}

UVLcppParser::MinorLevelContext* UVLcppParser::LanguageLevelContext::minorLevel() {
  return getRuleContext<UVLcppParser::MinorLevelContext>(0);
}

tree::TerminalNode* UVLcppParser::LanguageLevelContext::MUL() {
  return getToken(UVLcppParser::MUL, 0);
}


size_t UVLcppParser::LanguageLevelContext::getRuleIndex() const {
  return UVLcppParser::RuleLanguageLevel;
}

antlrcpp::Any UVLcppParser::LanguageLevelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitLanguageLevel(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::LanguageLevelContext* UVLcppParser::languageLevel() {
  LanguageLevelContext *_localctx = _tracker.createInstance<LanguageLevelContext>(_ctx, getState());
  enterRule(_localctx, 60, UVLcppParser::RuleLanguageLevel);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(396);
    majorLevel();
    setState(402);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == UVLcppParser::T__13) {
      setState(397);
      match(UVLcppParser::T__13);
      setState(400);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case UVLcppParser::T__19:
        case UVLcppParser::T__20:
        case UVLcppParser::T__21:
        case UVLcppParser::T__22: {
          setState(398);
          minorLevel();
          break;
        }

        case UVLcppParser::MUL: {
          setState(399);
          match(UVLcppParser::MUL);
          break;
        }

      default:
        throw NoViableAltException(this);
      }
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MajorLevelContext ------------------------------------------------------------------

UVLcppParser::MajorLevelContext::MajorLevelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* UVLcppParser::MajorLevelContext::BOOLEAN_KEY() {
  return getToken(UVLcppParser::BOOLEAN_KEY, 0);
}


size_t UVLcppParser::MajorLevelContext::getRuleIndex() const {
  return UVLcppParser::RuleMajorLevel;
}

antlrcpp::Any UVLcppParser::MajorLevelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitMajorLevel(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::MajorLevelContext* UVLcppParser::majorLevel() {
  MajorLevelContext *_localctx = _tracker.createInstance<MajorLevelContext>(_ctx, getState());
  enterRule(_localctx, 62, UVLcppParser::RuleMajorLevel);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(404);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__17)
      | (1ULL << UVLcppParser::T__18)
      | (1ULL << UVLcppParser::BOOLEAN_KEY))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- MinorLevelContext ------------------------------------------------------------------

UVLcppParser::MinorLevelContext::MinorLevelContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}


size_t UVLcppParser::MinorLevelContext::getRuleIndex() const {
  return UVLcppParser::RuleMinorLevel;
}

antlrcpp::Any UVLcppParser::MinorLevelContext::accept(tree::ParseTreeVisitor *visitor) {
  if (auto parserVisitor = dynamic_cast<UVLcppVisitor*>(visitor))
    return parserVisitor->visitMinorLevel(this);
  else
    return visitor->visitChildren(this);
}

UVLcppParser::MinorLevelContext* UVLcppParser::minorLevel() {
  MinorLevelContext *_localctx = _tracker.createInstance<MinorLevelContext>(_ctx, getState());
  enterRule(_localctx, 64, UVLcppParser::RuleMinorLevel);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(406);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & ((1ULL << UVLcppParser::T__19)
      | (1ULL << UVLcppParser::T__20)
      | (1ULL << UVLcppParser::T__21)
      | (1ULL << UVLcppParser::T__22))) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool UVLcppParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 21: return constraintSempred(dynamic_cast<ConstraintContext *>(context), predicateIndex);
    case 23: return expressionSempred(dynamic_cast<ExpressionContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool UVLcppParser::constraintSempred(ConstraintContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 4);
    case 1: return precpred(_ctx, 3);
    case 2: return precpred(_ctx, 2);
    case 3: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

bool UVLcppParser::expressionSempred(ExpressionContext *_localctx, size_t predicateIndex) {
  switch (predicateIndex) {
    case 4: return precpred(_ctx, 4);
    case 5: return precpred(_ctx, 3);
    case 6: return precpred(_ctx, 2);
    case 7: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

// Static vars and initialization.
std::vector<dfa::DFA> UVLcppParser::_decisionToDFA;
atn::PredictionContextCache UVLcppParser::_sharedContextCache;

// We own the ATN which in turn owns the ATN states.
atn::ATN UVLcppParser::_atn;
std::vector<uint16_t> UVLcppParser::_serializedATN;

std::vector<std::string> UVLcppParser::_ruleNames = {
  "featureModel", "includes", "includeLine", "namespace", "imports", "importLine", 
  "features", "group", "groupSpec", "feature", "featureCardinality", "attributes", 
  "attribute", "valueAttribute", "key", "value", "vector", "constraintAttribute", 
  "constraintList", "constraints", "constraintLine", "constraint", "equation", 
  "expression", "aggregateFunction", "stringAggregateFunction", "numericAggregateFunction", 
  "reference", "id", "featureType", "languageLevel", "majorLevel", "minorLevel"
};

std::vector<std::string> UVLcppParser::_literalNames = {
  "", "'include'", "'namespace'", "'imports'", "'as'", "'features'", "'cardinality'", 
  "'constraint'", "'constraints'", "'sum'", "'avg'", "'len'", "'floor'", 
  "'ceil'", "'.'", "'String'", "'Integer'", "'Real'", "'Arithmetic'", "'Type'", 
  "'group-cardinality'", "'feature-cardinality'", "'aggregate-function'", 
  "'string-constraints'", "'('", "')'", "'['", "']'", "'{'", "'}'", "'/*'", 
  "'*/'", "'<INDENT>'", "'<DEDENT>'", "'or'", "'alternative'", "'optional'", 
  "'mandatory'", "", "'!'", "'&'", "'|'", "'<=>'", "'=>'", "'=='", "'<'", 
  "'<='", "'>'", "'>='", "'!='", "'/'", "'*'", "'+'", "'-'", "", "", "", 
  "'Boolean'", "','"
};

std::vector<std::string> UVLcppParser::_symbolicNames = {
  "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", 
  "", "", "", "", "", "", "OPEN_PAREN", "CLOSE_PAREN", "OPEN_BRACK", "CLOSE_BRACK", 
  "OPEN_BRACE", "CLOSE_BRACE", "OPEN_COMMENT", "CLOSE_COMMENT", "INDENT", 
  "DEDENT", "ORGROUP", "ALTERNATIVE", "OPTIONAL", "MANDATORY", "CARDINALITY", 
  "NOT", "AND", "OR", "EQUIVALENCE", "IMPLICATION", "EQUAL", "LOWER", "LOWER_EQUALS", 
  "GREATER", "GREATER_EQUALS", "NOT_EQUALS", "DIV", "MUL", "ADD", "SUB", 
  "FLOAT", "INTEGER", "BOOLEAN", "BOOLEAN_KEY", "COMMA", "ID_NOT_STRICT", 
  "ID_STRICT", "STRING", "SKIP_", "NEWLINE"
};

dfa::Vocabulary UVLcppParser::_vocabulary(_literalNames, _symbolicNames);

std::vector<std::string> UVLcppParser::_tokenNames;

UVLcppParser::Initializer::Initializer() {
	for (size_t i = 0; i < _symbolicNames.size(); ++i) {
		std::string name = _vocabulary.getLiteralName(i);
		if (name.empty()) {
			name = _vocabulary.getSymbolicName(i);
		}

		if (name.empty()) {
			_tokenNames.push_back("<INVALID>");
		} else {
      _tokenNames.push_back(name);
    }
	}

  static const uint16_t serializedATNSegment0[] = {
    0x3, 0x608b, 0xa72a, 0x8133, 0xb9ed, 0x417c, 0x3be7, 0x7786, 0x5964, 
       0x3, 0x41, 0x19b, 0x4, 0x2, 0x9, 0x2, 0x4, 0x3, 0x9, 0x3, 0x4, 0x4, 
       0x9, 0x4, 0x4, 0x5, 0x9, 0x5, 0x4, 0x6, 0x9, 0x6, 0x4, 0x7, 0x9, 
       0x7, 0x4, 0x8, 0x9, 0x8, 0x4, 0x9, 0x9, 0x9, 0x4, 0xa, 0x9, 0xa, 
       0x4, 0xb, 0x9, 0xb, 0x4, 0xc, 0x9, 0xc, 0x4, 0xd, 0x9, 0xd, 0x4, 
       0xe, 0x9, 0xe, 0x4, 0xf, 0x9, 0xf, 0x4, 0x10, 0x9, 0x10, 0x4, 0x11, 
       0x9, 0x11, 0x4, 0x12, 0x9, 0x12, 0x4, 0x13, 0x9, 0x13, 0x4, 0x14, 
       0x9, 0x14, 0x4, 0x15, 0x9, 0x15, 0x4, 0x16, 0x9, 0x16, 0x4, 0x17, 
       0x9, 0x17, 0x4, 0x18, 0x9, 0x18, 0x4, 0x19, 0x9, 0x19, 0x4, 0x1a, 
       0x9, 0x1a, 0x4, 0x1b, 0x9, 0x1b, 0x4, 0x1c, 0x9, 0x1c, 0x4, 0x1d, 
       0x9, 0x1d, 0x4, 0x1e, 0x9, 0x1e, 0x4, 0x1f, 0x9, 0x1f, 0x4, 0x20, 
       0x9, 0x20, 0x4, 0x21, 0x9, 0x21, 0x4, 0x22, 0x9, 0x22, 0x3, 0x2, 
       0x5, 0x2, 0x46, 0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0x49, 0xa, 0x2, 0x3, 
       0x2, 0x5, 0x2, 0x4c, 0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0x4f, 0xa, 0x2, 
       0x3, 0x2, 0x5, 0x2, 0x52, 0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0x55, 0xa, 
       0x2, 0x3, 0x2, 0x5, 0x2, 0x58, 0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0x5b, 
       0xa, 0x2, 0x3, 0x2, 0x5, 0x2, 0x5e, 0xa, 0x2, 0x3, 0x2, 0x3, 0x2, 
       0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x7, 0x3, 0x66, 0xa, 0x3, 
       0xc, 0x3, 0xe, 0x3, 0x69, 0xb, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 
       0x3, 0x4, 0x3, 0x4, 0x3, 0x5, 0x3, 0x5, 0x3, 0x5, 0x3, 0x6, 0x3, 
       0x6, 0x3, 0x6, 0x3, 0x6, 0x7, 0x6, 0x77, 0xa, 0x6, 0xc, 0x6, 0xe, 
       0x6, 0x7a, 0xb, 0x6, 0x3, 0x6, 0x3, 0x6, 0x3, 0x7, 0x3, 0x7, 0x3, 
       0x7, 0x5, 0x7, 0x81, 0xa, 0x7, 0x3, 0x7, 0x3, 0x7, 0x3, 0x8, 0x3, 
       0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x8, 0x3, 0x9, 0x3, 0x9, 
       0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 0x9, 0x3, 
       0x9, 0x3, 0x9, 0x5, 0x9, 0x95, 0xa, 0x9, 0x3, 0xa, 0x3, 0xa, 0x3, 
       0xa, 0x6, 0xa, 0x9a, 0xa, 0xa, 0xd, 0xa, 0xe, 0xa, 0x9b, 0x3, 0xa, 
       0x3, 0xa, 0x3, 0xb, 0x5, 0xb, 0xa1, 0xa, 0xb, 0x3, 0xb, 0x3, 0xb, 
       0x5, 0xb, 0xa5, 0xa, 0xb, 0x3, 0xb, 0x5, 0xb, 0xa8, 0xa, 0xb, 0x3, 
       0xb, 0x3, 0xb, 0x3, 0xb, 0x6, 0xb, 0xad, 0xa, 0xb, 0xd, 0xb, 0xe, 
       0xb, 0xae, 0x3, 0xb, 0x3, 0xb, 0x5, 0xb, 0xb3, 0xa, 0xb, 0x3, 0xc, 
       0x3, 0xc, 0x3, 0xc, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xd, 0x7, 
       0xd, 0xbc, 0xa, 0xd, 0xc, 0xd, 0xe, 0xd, 0xbf, 0xb, 0xd, 0x5, 0xd, 
       0xc1, 0xa, 0xd, 0x3, 0xd, 0x3, 0xd, 0x3, 0xe, 0x3, 0xe, 0x5, 0xe, 
       0xc7, 0xa, 0xe, 0x3, 0xf, 0x3, 0xf, 0x5, 0xf, 0xcb, 0xa, 0xf, 0x3, 
       0x10, 0x3, 0x10, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 0x11, 0x3, 
       0x11, 0x3, 0x11, 0x5, 0x11, 0xd5, 0xa, 0x11, 0x3, 0x12, 0x3, 0x12, 
       0x3, 0x12, 0x3, 0x12, 0x7, 0x12, 0xdb, 0xa, 0x12, 0xc, 0x12, 0xe, 
       0x12, 0xde, 0xb, 0x12, 0x5, 0x12, 0xe0, 0xa, 0x12, 0x3, 0x12, 0x3, 
       0x12, 0x3, 0x13, 0x3, 0x13, 0x3, 0x13, 0x3, 0x13, 0x5, 0x13, 0xe8, 
       0xa, 0x13, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x14, 0x7, 0x14, 
       0xee, 0xa, 0x14, 0xc, 0x14, 0xe, 0x14, 0xf1, 0xb, 0x14, 0x5, 0x14, 
       0xf3, 0xa, 0x14, 0x3, 0x14, 0x3, 0x14, 0x3, 0x15, 0x3, 0x15, 0x3, 
       0x15, 0x3, 0x15, 0x7, 0x15, 0xfb, 0xa, 0x15, 0xc, 0x15, 0xe, 0x15, 
       0xfe, 0xb, 0x15, 0x3, 0x15, 0x3, 0x15, 0x3, 0x16, 0x3, 0x16, 0x3, 
       0x16, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 
       0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x5, 0x17, 0x10e, 0xa, 0x17, 
       0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 
       0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 0x3, 0x17, 
       0x7, 0x17, 0x11c, 0xa, 0x17, 0xc, 0x17, 0xe, 0x17, 0x11f, 0xb, 0x17, 
       0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
       0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
       0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
       0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 0x3, 0x18, 
       0x5, 0x18, 0x139, 0xa, 0x18, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 
       0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 
       0x19, 0x5, 0x19, 0x145, 0xa, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 
       0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 
       0x3, 0x19, 0x3, 0x19, 0x3, 0x19, 0x7, 0x19, 0x153, 0xa, 0x19, 0xc, 
       0x19, 0xe, 0x19, 0x156, 0xb, 0x19, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 
       0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x15d, 0xa, 0x1a, 0x3, 0x1a, 0x3, 
       0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 
       0x1a, 0x5, 0x1a, 0x167, 0xa, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 0x3, 0x1a, 
       0x3, 0x1a, 0x3, 0x1a, 0x5, 0x1a, 0x16e, 0xa, 0x1a, 0x3, 0x1b, 0x3, 
       0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1b, 0x3, 0x1c, 0x3, 0x1c, 0x3, 
       0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 0x1c, 0x3, 
       0x1c, 0x3, 0x1c, 0x5, 0x1c, 0x17f, 0xa, 0x1c, 0x3, 0x1d, 0x3, 0x1d, 
       0x3, 0x1d, 0x7, 0x1d, 0x184, 0xa, 0x1d, 0xc, 0x1d, 0xe, 0x1d, 0x187, 
       0xb, 0x1d, 0x3, 0x1d, 0x3, 0x1d, 0x3, 0x1e, 0x3, 0x1e, 0x3, 0x1f, 
       0x3, 0x1f, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x3, 0x20, 0x5, 0x20, 
       0x193, 0xa, 0x20, 0x5, 0x20, 0x195, 0xa, 0x20, 0x3, 0x21, 0x3, 0x21, 
       0x3, 0x22, 0x3, 0x22, 0x3, 0x22, 0x2, 0x4, 0x2c, 0x30, 0x23, 0x2, 
       0x4, 0x6, 0x8, 0xa, 0xc, 0xe, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 
       0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 
       0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, 0x40, 0x42, 0x2, 0x6, 0x3, 
       0x2, 0x3d, 0x3e, 0x4, 0x2, 0x11, 0x13, 0x3b, 0x3b, 0x4, 0x2, 0x14, 
       0x15, 0x3b, 0x3b, 0x3, 0x2, 0x16, 0x19, 0x2, 0x1bc, 0x2, 0x45, 0x3, 
       0x2, 0x2, 0x2, 0x4, 0x61, 0x3, 0x2, 0x2, 0x2, 0x6, 0x6c, 0x3, 0x2, 
       0x2, 0x2, 0x8, 0x6f, 0x3, 0x2, 0x2, 0x2, 0xa, 0x72, 0x3, 0x2, 0x2, 
       0x2, 0xc, 0x7d, 0x3, 0x2, 0x2, 0x2, 0xe, 0x84, 0x3, 0x2, 0x2, 0x2, 
       0x10, 0x94, 0x3, 0x2, 0x2, 0x2, 0x12, 0x96, 0x3, 0x2, 0x2, 0x2, 0x14, 
       0xa0, 0x3, 0x2, 0x2, 0x2, 0x16, 0xb4, 0x3, 0x2, 0x2, 0x2, 0x18, 0xb7, 
       0x3, 0x2, 0x2, 0x2, 0x1a, 0xc6, 0x3, 0x2, 0x2, 0x2, 0x1c, 0xc8, 0x3, 
       0x2, 0x2, 0x2, 0x1e, 0xcc, 0x3, 0x2, 0x2, 0x2, 0x20, 0xd4, 0x3, 0x2, 
       0x2, 0x2, 0x22, 0xd6, 0x3, 0x2, 0x2, 0x2, 0x24, 0xe7, 0x3, 0x2, 0x2, 
       0x2, 0x26, 0xe9, 0x3, 0x2, 0x2, 0x2, 0x28, 0xf6, 0x3, 0x2, 0x2, 0x2, 
       0x2a, 0x101, 0x3, 0x2, 0x2, 0x2, 0x2c, 0x10d, 0x3, 0x2, 0x2, 0x2, 
       0x2e, 0x138, 0x3, 0x2, 0x2, 0x2, 0x30, 0x144, 0x3, 0x2, 0x2, 0x2, 
       0x32, 0x16d, 0x3, 0x2, 0x2, 0x2, 0x34, 0x16f, 0x3, 0x2, 0x2, 0x2, 
       0x36, 0x17e, 0x3, 0x2, 0x2, 0x2, 0x38, 0x185, 0x3, 0x2, 0x2, 0x2, 
       0x3a, 0x18a, 0x3, 0x2, 0x2, 0x2, 0x3c, 0x18c, 0x3, 0x2, 0x2, 0x2, 
       0x3e, 0x18e, 0x3, 0x2, 0x2, 0x2, 0x40, 0x196, 0x3, 0x2, 0x2, 0x2, 
       0x42, 0x198, 0x3, 0x2, 0x2, 0x2, 0x44, 0x46, 0x5, 0x8, 0x5, 0x2, 
       0x45, 0x44, 0x3, 0x2, 0x2, 0x2, 0x45, 0x46, 0x3, 0x2, 0x2, 0x2, 0x46, 
       0x48, 0x3, 0x2, 0x2, 0x2, 0x47, 0x49, 0x7, 0x41, 0x2, 0x2, 0x48, 
       0x47, 0x3, 0x2, 0x2, 0x2, 0x48, 0x49, 0x3, 0x2, 0x2, 0x2, 0x49, 0x4b, 
       0x3, 0x2, 0x2, 0x2, 0x4a, 0x4c, 0x5, 0x4, 0x3, 0x2, 0x4b, 0x4a, 0x3, 
       0x2, 0x2, 0x2, 0x4b, 0x4c, 0x3, 0x2, 0x2, 0x2, 0x4c, 0x4e, 0x3, 0x2, 
       0x2, 0x2, 0x4d, 0x4f, 0x7, 0x41, 0x2, 0x2, 0x4e, 0x4d, 0x3, 0x2, 
       0x2, 0x2, 0x4e, 0x4f, 0x3, 0x2, 0x2, 0x2, 0x4f, 0x51, 0x3, 0x2, 0x2, 
       0x2, 0x50, 0x52, 0x5, 0xa, 0x6, 0x2, 0x51, 0x50, 0x3, 0x2, 0x2, 0x2, 
       0x51, 0x52, 0x3, 0x2, 0x2, 0x2, 0x52, 0x54, 0x3, 0x2, 0x2, 0x2, 0x53, 
       0x55, 0x7, 0x41, 0x2, 0x2, 0x54, 0x53, 0x3, 0x2, 0x2, 0x2, 0x54, 
       0x55, 0x3, 0x2, 0x2, 0x2, 0x55, 0x57, 0x3, 0x2, 0x2, 0x2, 0x56, 0x58, 
       0x5, 0xe, 0x8, 0x2, 0x57, 0x56, 0x3, 0x2, 0x2, 0x2, 0x57, 0x58, 0x3, 
       0x2, 0x2, 0x2, 0x58, 0x5a, 0x3, 0x2, 0x2, 0x2, 0x59, 0x5b, 0x7, 0x41, 
       0x2, 0x2, 0x5a, 0x59, 0x3, 0x2, 0x2, 0x2, 0x5a, 0x5b, 0x3, 0x2, 0x2, 
       0x2, 0x5b, 0x5d, 0x3, 0x2, 0x2, 0x2, 0x5c, 0x5e, 0x5, 0x28, 0x15, 
       0x2, 0x5d, 0x5c, 0x3, 0x2, 0x2, 0x2, 0x5d, 0x5e, 0x3, 0x2, 0x2, 0x2, 
       0x5e, 0x5f, 0x3, 0x2, 0x2, 0x2, 0x5f, 0x60, 0x7, 0x2, 0x2, 0x3, 0x60, 
       0x3, 0x3, 0x2, 0x2, 0x2, 0x61, 0x62, 0x7, 0x3, 0x2, 0x2, 0x62, 0x63, 
       0x7, 0x41, 0x2, 0x2, 0x63, 0x67, 0x7, 0x22, 0x2, 0x2, 0x64, 0x66, 
       0x5, 0x6, 0x4, 0x2, 0x65, 0x64, 0x3, 0x2, 0x2, 0x2, 0x66, 0x69, 0x3, 
       0x2, 0x2, 0x2, 0x67, 0x65, 0x3, 0x2, 0x2, 0x2, 0x67, 0x68, 0x3, 0x2, 
       0x2, 0x2, 0x68, 0x6a, 0x3, 0x2, 0x2, 0x2, 0x69, 0x67, 0x3, 0x2, 0x2, 
       0x2, 0x6a, 0x6b, 0x7, 0x23, 0x2, 0x2, 0x6b, 0x5, 0x3, 0x2, 0x2, 0x2, 
       0x6c, 0x6d, 0x5, 0x3e, 0x20, 0x2, 0x6d, 0x6e, 0x7, 0x41, 0x2, 0x2, 
       0x6e, 0x7, 0x3, 0x2, 0x2, 0x2, 0x6f, 0x70, 0x7, 0x4, 0x2, 0x2, 0x70, 
       0x71, 0x5, 0x38, 0x1d, 0x2, 0x71, 0x9, 0x3, 0x2, 0x2, 0x2, 0x72, 
       0x73, 0x7, 0x5, 0x2, 0x2, 0x73, 0x74, 0x7, 0x41, 0x2, 0x2, 0x74, 
       0x78, 0x7, 0x22, 0x2, 0x2, 0x75, 0x77, 0x5, 0xc, 0x7, 0x2, 0x76, 
       0x75, 0x3, 0x2, 0x2, 0x2, 0x77, 0x7a, 0x3, 0x2, 0x2, 0x2, 0x78, 0x76, 
       0x3, 0x2, 0x2, 0x2, 0x78, 0x79, 0x3, 0x2, 0x2, 0x2, 0x79, 0x7b, 0x3, 
       0x2, 0x2, 0x2, 0x7a, 0x78, 0x3, 0x2, 0x2, 0x2, 0x7b, 0x7c, 0x7, 0x23, 
       0x2, 0x2, 0x7c, 0xb, 0x3, 0x2, 0x2, 0x2, 0x7d, 0x80, 0x5, 0x38, 0x1d, 
       0x2, 0x7e, 0x7f, 0x7, 0x6, 0x2, 0x2, 0x7f, 0x81, 0x5, 0x38, 0x1d, 
       0x2, 0x80, 0x7e, 0x3, 0x2, 0x2, 0x2, 0x80, 0x81, 0x3, 0x2, 0x2, 0x2, 
       0x81, 0x82, 0x3, 0x2, 0x2, 0x2, 0x82, 0x83, 0x7, 0x41, 0x2, 0x2, 
       0x83, 0xd, 0x3, 0x2, 0x2, 0x2, 0x84, 0x85, 0x7, 0x7, 0x2, 0x2, 0x85, 
       0x86, 0x7, 0x41, 0x2, 0x2, 0x86, 0x87, 0x7, 0x22, 0x2, 0x2, 0x87, 
       0x88, 0x5, 0x14, 0xb, 0x2, 0x88, 0x89, 0x7, 0x23, 0x2, 0x2, 0x89, 
       0xf, 0x3, 0x2, 0x2, 0x2, 0x8a, 0x8b, 0x7, 0x24, 0x2, 0x2, 0x8b, 0x95, 
       0x5, 0x12, 0xa, 0x2, 0x8c, 0x8d, 0x7, 0x25, 0x2, 0x2, 0x8d, 0x95, 
       0x5, 0x12, 0xa, 0x2, 0x8e, 0x8f, 0x7, 0x26, 0x2, 0x2, 0x8f, 0x95, 
       0x5, 0x12, 0xa, 0x2, 0x90, 0x91, 0x7, 0x27, 0x2, 0x2, 0x91, 0x95, 
       0x5, 0x12, 0xa, 0x2, 0x92, 0x93, 0x7, 0x28, 0x2, 0x2, 0x93, 0x95, 
       0x5, 0x12, 0xa, 0x2, 0x94, 0x8a, 0x3, 0x2, 0x2, 0x2, 0x94, 0x8c, 
       0x3, 0x2, 0x2, 0x2, 0x94, 0x8e, 0x3, 0x2, 0x2, 0x2, 0x94, 0x90, 0x3, 
       0x2, 0x2, 0x2, 0x94, 0x92, 0x3, 0x2, 0x2, 0x2, 0x95, 0x11, 0x3, 0x2, 
       0x2, 0x2, 0x96, 0x97, 0x7, 0x41, 0x2, 0x2, 0x97, 0x99, 0x7, 0x22, 
       0x2, 0x2, 0x98, 0x9a, 0x5, 0x14, 0xb, 0x2, 0x99, 0x98, 0x3, 0x2, 
       0x2, 0x2, 0x9a, 0x9b, 0x3, 0x2, 0x2, 0x2, 0x9b, 0x99, 0x3, 0x2, 0x2, 
       0x2, 0x9b, 0x9c, 0x3, 0x2, 0x2, 0x2, 0x9c, 0x9d, 0x3, 0x2, 0x2, 0x2, 
       0x9d, 0x9e, 0x7, 0x23, 0x2, 0x2, 0x9e, 0x13, 0x3, 0x2, 0x2, 0x2, 
       0x9f, 0xa1, 0x5, 0x3c, 0x1f, 0x2, 0xa0, 0x9f, 0x3, 0x2, 0x2, 0x2, 
       0xa0, 0xa1, 0x3, 0x2, 0x2, 0x2, 0xa1, 0xa2, 0x3, 0x2, 0x2, 0x2, 0xa2, 
       0xa4, 0x5, 0x38, 0x1d, 0x2, 0xa3, 0xa5, 0x5, 0x16, 0xc, 0x2, 0xa4, 
       0xa3, 0x3, 0x2, 0x2, 0x2, 0xa4, 0xa5, 0x3, 0x2, 0x2, 0x2, 0xa5, 0xa7, 
       0x3, 0x2, 0x2, 0x2, 0xa6, 0xa8, 0x5, 0x18, 0xd, 0x2, 0xa7, 0xa6, 
       0x3, 0x2, 0x2, 0x2, 0xa7, 0xa8, 0x3, 0x2, 0x2, 0x2, 0xa8, 0xa9, 0x3, 
       0x2, 0x2, 0x2, 0xa9, 0xb2, 0x7, 0x41, 0x2, 0x2, 0xaa, 0xac, 0x7, 
       0x22, 0x2, 0x2, 0xab, 0xad, 0x5, 0x10, 0x9, 0x2, 0xac, 0xab, 0x3, 
       0x2, 0x2, 0x2, 0xad, 0xae, 0x3, 0x2, 0x2, 0x2, 0xae, 0xac, 0x3, 0x2, 
       0x2, 0x2, 0xae, 0xaf, 0x3, 0x2, 0x2, 0x2, 0xaf, 0xb0, 0x3, 0x2, 0x2, 
       0x2, 0xb0, 0xb1, 0x7, 0x23, 0x2, 0x2, 0xb1, 0xb3, 0x3, 0x2, 0x2, 
       0x2, 0xb2, 0xaa, 0x3, 0x2, 0x2, 0x2, 0xb2, 0xb3, 0x3, 0x2, 0x2, 0x2, 
       0xb3, 0x15, 0x3, 0x2, 0x2, 0x2, 0xb4, 0xb5, 0x7, 0x8, 0x2, 0x2, 0xb5, 
       0xb6, 0x7, 0x28, 0x2, 0x2, 0xb6, 0x17, 0x3, 0x2, 0x2, 0x2, 0xb7, 
       0xc0, 0x7, 0x1e, 0x2, 0x2, 0xb8, 0xbd, 0x5, 0x1a, 0xe, 0x2, 0xb9, 
       0xba, 0x7, 0x3c, 0x2, 0x2, 0xba, 0xbc, 0x5, 0x1a, 0xe, 0x2, 0xbb, 
       0xb9, 0x3, 0x2, 0x2, 0x2, 0xbc, 0xbf, 0x3, 0x2, 0x2, 0x2, 0xbd, 0xbb, 
       0x3, 0x2, 0x2, 0x2, 0xbd, 0xbe, 0x3, 0x2, 0x2, 0x2, 0xbe, 0xc1, 0x3, 
       0x2, 0x2, 0x2, 0xbf, 0xbd, 0x3, 0x2, 0x2, 0x2, 0xc0, 0xb8, 0x3, 0x2, 
       0x2, 0x2, 0xc0, 0xc1, 0x3, 0x2, 0x2, 0x2, 0xc1, 0xc2, 0x3, 0x2, 0x2, 
       0x2, 0xc2, 0xc3, 0x7, 0x1f, 0x2, 0x2, 0xc3, 0x19, 0x3, 0x2, 0x2, 
       0x2, 0xc4, 0xc7, 0x5, 0x1c, 0xf, 0x2, 0xc5, 0xc7, 0x5, 0x24, 0x13, 
       0x2, 0xc6, 0xc4, 0x3, 0x2, 0x2, 0x2, 0xc6, 0xc5, 0x3, 0x2, 0x2, 0x2, 
       0xc7, 0x1b, 0x3, 0x2, 0x2, 0x2, 0xc8, 0xca, 0x5, 0x1e, 0x10, 0x2, 
       0xc9, 0xcb, 0x5, 0x20, 0x11, 0x2, 0xca, 0xc9, 0x3, 0x2, 0x2, 0x2, 
       0xca, 0xcb, 0x3, 0x2, 0x2, 0x2, 0xcb, 0x1d, 0x3, 0x2, 0x2, 0x2, 0xcc, 
       0xcd, 0x5, 0x3a, 0x1e, 0x2, 0xcd, 0x1f, 0x3, 0x2, 0x2, 0x2, 0xce, 
       0xd5, 0x7, 0x3a, 0x2, 0x2, 0xcf, 0xd5, 0x7, 0x38, 0x2, 0x2, 0xd0, 
       0xd5, 0x7, 0x39, 0x2, 0x2, 0xd1, 0xd5, 0x7, 0x3f, 0x2, 0x2, 0xd2, 
       0xd5, 0x5, 0x18, 0xd, 0x2, 0xd3, 0xd5, 0x5, 0x22, 0x12, 0x2, 0xd4, 
       0xce, 0x3, 0x2, 0x2, 0x2, 0xd4, 0xcf, 0x3, 0x2, 0x2, 0x2, 0xd4, 0xd0, 
       0x3, 0x2, 0x2, 0x2, 0xd4, 0xd1, 0x3, 0x2, 0x2, 0x2, 0xd4, 0xd2, 0x3, 
       0x2, 0x2, 0x2, 0xd4, 0xd3, 0x3, 0x2, 0x2, 0x2, 0xd5, 0x21, 0x3, 0x2, 
       0x2, 0x2, 0xd6, 0xdf, 0x7, 0x1c, 0x2, 0x2, 0xd7, 0xdc, 0x5, 0x20, 
       0x11, 0x2, 0xd8, 0xd9, 0x7, 0x3c, 0x2, 0x2, 0xd9, 0xdb, 0x5, 0x20, 
       0x11, 0x2, 0xda, 0xd8, 0x3, 0x2, 0x2, 0x2, 0xdb, 0xde, 0x3, 0x2, 
       0x2, 0x2, 0xdc, 0xda, 0x3, 0x2, 0x2, 0x2, 0xdc, 0xdd, 0x3, 0x2, 0x2, 
       0x2, 0xdd, 0xe0, 0x3, 0x2, 0x2, 0x2, 0xde, 0xdc, 0x3, 0x2, 0x2, 0x2, 
       0xdf, 0xd7, 0x3, 0x2, 0x2, 0x2, 0xdf, 0xe0, 0x3, 0x2, 0x2, 0x2, 0xe0, 
       0xe1, 0x3, 0x2, 0x2, 0x2, 0xe1, 0xe2, 0x7, 0x1d, 0x2, 0x2, 0xe2, 
       0x23, 0x3, 0x2, 0x2, 0x2, 0xe3, 0xe4, 0x7, 0x9, 0x2, 0x2, 0xe4, 0xe8, 
       0x5, 0x2c, 0x17, 0x2, 0xe5, 0xe6, 0x7, 0xa, 0x2, 0x2, 0xe6, 0xe8, 
       0x5, 0x26, 0x14, 0x2, 0xe7, 0xe3, 0x3, 0x2, 0x2, 0x2, 0xe7, 0xe5, 
       0x3, 0x2, 0x2, 0x2, 0xe8, 0x25, 0x3, 0x2, 0x2, 0x2, 0xe9, 0xf2, 0x7, 
       0x1c, 0x2, 0x2, 0xea, 0xef, 0x5, 0x2c, 0x17, 0x2, 0xeb, 0xec, 0x7, 
       0x3c, 0x2, 0x2, 0xec, 0xee, 0x5, 0x2c, 0x17, 0x2, 0xed, 0xeb, 0x3, 
       0x2, 0x2, 0x2, 0xee, 0xf1, 0x3, 0x2, 0x2, 0x2, 0xef, 0xed, 0x3, 0x2, 
       0x2, 0x2, 0xef, 0xf0, 0x3, 0x2, 0x2, 0x2, 0xf0, 0xf3, 0x3, 0x2, 0x2, 
       0x2, 0xf1, 0xef, 0x3, 0x2, 0x2, 0x2, 0xf2, 0xea, 0x3, 0x2, 0x2, 0x2, 
       0xf2, 0xf3, 0x3, 0x2, 0x2, 0x2, 0xf3, 0xf4, 0x3, 0x2, 0x2, 0x2, 0xf4, 
       0xf5, 0x7, 0x1d, 0x2, 0x2, 0xf5, 0x27, 0x3, 0x2, 0x2, 0x2, 0xf6, 
       0xf7, 0x7, 0xa, 0x2, 0x2, 0xf7, 0xf8, 0x7, 0x41, 0x2, 0x2, 0xf8, 
       0xfc, 0x7, 0x22, 0x2, 0x2, 0xf9, 0xfb, 0x5, 0x2a, 0x16, 0x2, 0xfa, 
       0xf9, 0x3, 0x2, 0x2, 0x2, 0xfb, 0xfe, 0x3, 0x2, 0x2, 0x2, 0xfc, 0xfa, 
       0x3, 0x2, 0x2, 0x2, 0xfc, 0xfd, 0x3, 0x2, 0x2, 0x2, 0xfd, 0xff, 0x3, 
       0x2, 0x2, 0x2, 0xfe, 0xfc, 0x3, 0x2, 0x2, 0x2, 0xff, 0x100, 0x7, 
       0x23, 0x2, 0x2, 0x100, 0x29, 0x3, 0x2, 0x2, 0x2, 0x101, 0x102, 0x5, 
       0x2c, 0x17, 0x2, 0x102, 0x103, 0x7, 0x41, 0x2, 0x2, 0x103, 0x2b, 
       0x3, 0x2, 0x2, 0x2, 0x104, 0x105, 0x8, 0x17, 0x1, 0x2, 0x105, 0x10e, 
       0x5, 0x2e, 0x18, 0x2, 0x106, 0x10e, 0x5, 0x38, 0x1d, 0x2, 0x107, 
       0x108, 0x7, 0x1a, 0x2, 0x2, 0x108, 0x109, 0x5, 0x2c, 0x17, 0x2, 0x109, 
       0x10a, 0x7, 0x1b, 0x2, 0x2, 0x10a, 0x10e, 0x3, 0x2, 0x2, 0x2, 0x10b, 
       0x10c, 0x7, 0x29, 0x2, 0x2, 0x10c, 0x10e, 0x5, 0x2c, 0x17, 0x7, 0x10d, 
       0x104, 0x3, 0x2, 0x2, 0x2, 0x10d, 0x106, 0x3, 0x2, 0x2, 0x2, 0x10d, 
       0x107, 0x3, 0x2, 0x2, 0x2, 0x10d, 0x10b, 0x3, 0x2, 0x2, 0x2, 0x10e, 
       0x11d, 0x3, 0x2, 0x2, 0x2, 0x10f, 0x110, 0xc, 0x6, 0x2, 0x2, 0x110, 
       0x111, 0x7, 0x2a, 0x2, 0x2, 0x111, 0x11c, 0x5, 0x2c, 0x17, 0x7, 0x112, 
       0x113, 0xc, 0x5, 0x2, 0x2, 0x113, 0x114, 0x7, 0x2b, 0x2, 0x2, 0x114, 
       0x11c, 0x5, 0x2c, 0x17, 0x6, 0x115, 0x116, 0xc, 0x4, 0x2, 0x2, 0x116, 
       0x117, 0x7, 0x2d, 0x2, 0x2, 0x117, 0x11c, 0x5, 0x2c, 0x17, 0x5, 0x118, 
       0x119, 0xc, 0x3, 0x2, 0x2, 0x119, 0x11a, 0x7, 0x2c, 0x2, 0x2, 0x11a, 
       0x11c, 0x5, 0x2c, 0x17, 0x4, 0x11b, 0x10f, 0x3, 0x2, 0x2, 0x2, 0x11b, 
       0x112, 0x3, 0x2, 0x2, 0x2, 0x11b, 0x115, 0x3, 0x2, 0x2, 0x2, 0x11b, 
       0x118, 0x3, 0x2, 0x2, 0x2, 0x11c, 0x11f, 0x3, 0x2, 0x2, 0x2, 0x11d, 
       0x11b, 0x3, 0x2, 0x2, 0x2, 0x11d, 0x11e, 0x3, 0x2, 0x2, 0x2, 0x11e, 
       0x2d, 0x3, 0x2, 0x2, 0x2, 0x11f, 0x11d, 0x3, 0x2, 0x2, 0x2, 0x120, 
       0x121, 0x5, 0x30, 0x19, 0x2, 0x121, 0x122, 0x7, 0x2e, 0x2, 0x2, 0x122, 
       0x123, 0x5, 0x30, 0x19, 0x2, 0x123, 0x139, 0x3, 0x2, 0x2, 0x2, 0x124, 
       0x125, 0x5, 0x30, 0x19, 0x2, 0x125, 0x126, 0x7, 0x2f, 0x2, 0x2, 0x126, 
       0x127, 0x5, 0x30, 0x19, 0x2, 0x127, 0x139, 0x3, 0x2, 0x2, 0x2, 0x128, 
       0x129, 0x5, 0x30, 0x19, 0x2, 0x129, 0x12a, 0x7, 0x31, 0x2, 0x2, 0x12a, 
       0x12b, 0x5, 0x30, 0x19, 0x2, 0x12b, 0x139, 0x3, 0x2, 0x2, 0x2, 0x12c, 
       0x12d, 0x5, 0x30, 0x19, 0x2, 0x12d, 0x12e, 0x7, 0x30, 0x2, 0x2, 0x12e, 
       0x12f, 0x5, 0x30, 0x19, 0x2, 0x12f, 0x139, 0x3, 0x2, 0x2, 0x2, 0x130, 
       0x131, 0x5, 0x30, 0x19, 0x2, 0x131, 0x132, 0x7, 0x32, 0x2, 0x2, 0x132, 
       0x133, 0x5, 0x30, 0x19, 0x2, 0x133, 0x139, 0x3, 0x2, 0x2, 0x2, 0x134, 
       0x135, 0x5, 0x30, 0x19, 0x2, 0x135, 0x136, 0x7, 0x33, 0x2, 0x2, 0x136, 
       0x137, 0x5, 0x30, 0x19, 0x2, 0x137, 0x139, 0x3, 0x2, 0x2, 0x2, 0x138, 
       0x120, 0x3, 0x2, 0x2, 0x2, 0x138, 0x124, 0x3, 0x2, 0x2, 0x2, 0x138, 
       0x128, 0x3, 0x2, 0x2, 0x2, 0x138, 0x12c, 0x3, 0x2, 0x2, 0x2, 0x138, 
       0x130, 0x3, 0x2, 0x2, 0x2, 0x138, 0x134, 0x3, 0x2, 0x2, 0x2, 0x139, 
       0x2f, 0x3, 0x2, 0x2, 0x2, 0x13a, 0x13b, 0x8, 0x19, 0x1, 0x2, 0x13b, 
       0x145, 0x7, 0x38, 0x2, 0x2, 0x13c, 0x145, 0x7, 0x39, 0x2, 0x2, 0x13d, 
       0x145, 0x7, 0x3f, 0x2, 0x2, 0x13e, 0x145, 0x5, 0x32, 0x1a, 0x2, 0x13f, 
       0x145, 0x5, 0x38, 0x1d, 0x2, 0x140, 0x141, 0x7, 0x1a, 0x2, 0x2, 0x141, 
       0x142, 0x5, 0x30, 0x19, 0x2, 0x142, 0x143, 0x7, 0x1b, 0x2, 0x2, 0x143, 
       0x145, 0x3, 0x2, 0x2, 0x2, 0x144, 0x13a, 0x3, 0x2, 0x2, 0x2, 0x144, 
       0x13c, 0x3, 0x2, 0x2, 0x2, 0x144, 0x13d, 0x3, 0x2, 0x2, 0x2, 0x144, 
       0x13e, 0x3, 0x2, 0x2, 0x2, 0x144, 0x13f, 0x3, 0x2, 0x2, 0x2, 0x144, 
       0x140, 0x3, 0x2, 0x2, 0x2, 0x145, 0x154, 0x3, 0x2, 0x2, 0x2, 0x146, 
       0x147, 0xc, 0x6, 0x2, 0x2, 0x147, 0x148, 0x7, 0x36, 0x2, 0x2, 0x148, 
       0x153, 0x5, 0x30, 0x19, 0x7, 0x149, 0x14a, 0xc, 0x5, 0x2, 0x2, 0x14a, 
       0x14b, 0x7, 0x37, 0x2, 0x2, 0x14b, 0x153, 0x5, 0x30, 0x19, 0x6, 0x14c, 
       0x14d, 0xc, 0x4, 0x2, 0x2, 0x14d, 0x14e, 0x7, 0x35, 0x2, 0x2, 0x14e, 
       0x153, 0x5, 0x30, 0x19, 0x5, 0x14f, 0x150, 0xc, 0x3, 0x2, 0x2, 0x150, 
       0x151, 0x7, 0x34, 0x2, 0x2, 0x151, 0x153, 0x5, 0x30, 0x19, 0x4, 0x152, 
       0x146, 0x3, 0x2, 0x2, 0x2, 0x152, 0x149, 0x3, 0x2, 0x2, 0x2, 0x152, 
       0x14c, 0x3, 0x2, 0x2, 0x2, 0x152, 0x14f, 0x3, 0x2, 0x2, 0x2, 0x153, 
       0x156, 0x3, 0x2, 0x2, 0x2, 0x154, 0x152, 0x3, 0x2, 0x2, 0x2, 0x154, 
       0x155, 0x3, 0x2, 0x2, 0x2, 0x155, 0x31, 0x3, 0x2, 0x2, 0x2, 0x156, 
       0x154, 0x3, 0x2, 0x2, 0x2, 0x157, 0x158, 0x7, 0xb, 0x2, 0x2, 0x158, 
       0x15c, 0x7, 0x1a, 0x2, 0x2, 0x159, 0x15a, 0x5, 0x38, 0x1d, 0x2, 0x15a, 
       0x15b, 0x7, 0x3c, 0x2, 0x2, 0x15b, 0x15d, 0x3, 0x2, 0x2, 0x2, 0x15c, 
       0x159, 0x3, 0x2, 0x2, 0x2, 0x15c, 0x15d, 0x3, 0x2, 0x2, 0x2, 0x15d, 
       0x15e, 0x3, 0x2, 0x2, 0x2, 0x15e, 0x15f, 0x5, 0x38, 0x1d, 0x2, 0x15f, 
       0x160, 0x7, 0x1b, 0x2, 0x2, 0x160, 0x16e, 0x3, 0x2, 0x2, 0x2, 0x161, 
       0x162, 0x7, 0xc, 0x2, 0x2, 0x162, 0x166, 0x7, 0x1a, 0x2, 0x2, 0x163, 
       0x164, 0x5, 0x38, 0x1d, 0x2, 0x164, 0x165, 0x7, 0x3c, 0x2, 0x2, 0x165, 
       0x167, 0x3, 0x2, 0x2, 0x2, 0x166, 0x163, 0x3, 0x2, 0x2, 0x2, 0x166, 
       0x167, 0x3, 0x2, 0x2, 0x2, 0x167, 0x168, 0x3, 0x2, 0x2, 0x2, 0x168, 
       0x169, 0x5, 0x38, 0x1d, 0x2, 0x169, 0x16a, 0x7, 0x1b, 0x2, 0x2, 0x16a, 
       0x16e, 0x3, 0x2, 0x2, 0x2, 0x16b, 0x16e, 0x5, 0x34, 0x1b, 0x2, 0x16c, 
       0x16e, 0x5, 0x36, 0x1c, 0x2, 0x16d, 0x157, 0x3, 0x2, 0x2, 0x2, 0x16d, 
       0x161, 0x3, 0x2, 0x2, 0x2, 0x16d, 0x16b, 0x3, 0x2, 0x2, 0x2, 0x16d, 
       0x16c, 0x3, 0x2, 0x2, 0x2, 0x16e, 0x33, 0x3, 0x2, 0x2, 0x2, 0x16f, 
       0x170, 0x7, 0xd, 0x2, 0x2, 0x170, 0x171, 0x7, 0x1a, 0x2, 0x2, 0x171, 
       0x172, 0x5, 0x38, 0x1d, 0x2, 0x172, 0x173, 0x7, 0x1b, 0x2, 0x2, 0x173, 
       0x35, 0x3, 0x2, 0x2, 0x2, 0x174, 0x175, 0x7, 0xe, 0x2, 0x2, 0x175, 
       0x176, 0x7, 0x1a, 0x2, 0x2, 0x176, 0x177, 0x5, 0x38, 0x1d, 0x2, 0x177, 
       0x178, 0x7, 0x1b, 0x2, 0x2, 0x178, 0x17f, 0x3, 0x2, 0x2, 0x2, 0x179, 
       0x17a, 0x7, 0xf, 0x2, 0x2, 0x17a, 0x17b, 0x7, 0x1a, 0x2, 0x2, 0x17b, 
       0x17c, 0x5, 0x38, 0x1d, 0x2, 0x17c, 0x17d, 0x7, 0x1b, 0x2, 0x2, 0x17d, 
       0x17f, 0x3, 0x2, 0x2, 0x2, 0x17e, 0x174, 0x3, 0x2, 0x2, 0x2, 0x17e, 
       0x179, 0x3, 0x2, 0x2, 0x2, 0x17f, 0x37, 0x3, 0x2, 0x2, 0x2, 0x180, 
       0x181, 0x5, 0x3a, 0x1e, 0x2, 0x181, 0x182, 0x7, 0x10, 0x2, 0x2, 0x182, 
       0x184, 0x3, 0x2, 0x2, 0x2, 0x183, 0x180, 0x3, 0x2, 0x2, 0x2, 0x184, 
       0x187, 0x3, 0x2, 0x2, 0x2, 0x185, 0x183, 0x3, 0x2, 0x2, 0x2, 0x185, 
       0x186, 0x3, 0x2, 0x2, 0x2, 0x186, 0x188, 0x3, 0x2, 0x2, 0x2, 0x187, 
       0x185, 0x3, 0x2, 0x2, 0x2, 0x188, 0x189, 0x5, 0x3a, 0x1e, 0x2, 0x189, 
       0x39, 0x3, 0x2, 0x2, 0x2, 0x18a, 0x18b, 0x9, 0x2, 0x2, 0x2, 0x18b, 
       0x3b, 0x3, 0x2, 0x2, 0x2, 0x18c, 0x18d, 0x9, 0x3, 0x2, 0x2, 0x18d, 
       0x3d, 0x3, 0x2, 0x2, 0x2, 0x18e, 0x194, 0x5, 0x40, 0x21, 0x2, 0x18f, 
       0x192, 0x7, 0x10, 0x2, 0x2, 0x190, 0x193, 0x5, 0x42, 0x22, 0x2, 0x191, 
       0x193, 0x7, 0x35, 0x2, 0x2, 0x192, 0x190, 0x3, 0x2, 0x2, 0x2, 0x192, 
       0x191, 0x3, 0x2, 0x2, 0x2, 0x193, 0x195, 0x3, 0x2, 0x2, 0x2, 0x194, 
       0x18f, 0x3, 0x2, 0x2, 0x2, 0x194, 0x195, 0x3, 0x2, 0x2, 0x2, 0x195, 
       0x3f, 0x3, 0x2, 0x2, 0x2, 0x196, 0x197, 0x9, 0x4, 0x2, 0x2, 0x197, 
       0x41, 0x3, 0x2, 0x2, 0x2, 0x198, 0x199, 0x9, 0x5, 0x2, 0x2, 0x199, 
       0x43, 0x3, 0x2, 0x2, 0x2, 0x2e, 0x45, 0x48, 0x4b, 0x4e, 0x51, 0x54, 
       0x57, 0x5a, 0x5d, 0x67, 0x78, 0x80, 0x94, 0x9b, 0xa0, 0xa4, 0xa7, 
       0xae, 0xb2, 0xbd, 0xc0, 0xc6, 0xca, 0xd4, 0xdc, 0xdf, 0xe7, 0xef, 
       0xf2, 0xfc, 0x10d, 0x11b, 0x11d, 0x138, 0x144, 0x152, 0x154, 0x15c, 
       0x166, 0x16d, 0x17e, 0x185, 0x192, 0x194, 
  };

  _serializedATN.insert(_serializedATN.end(), serializedATNSegment0,
    serializedATNSegment0 + sizeof(serializedATNSegment0) / sizeof(serializedATNSegment0[0]));


  atn::ATNDeserializer deserializer;
  _atn = deserializer.deserialize(_serializedATN);

  size_t count = _atn.getNumberOfDecisions();
  _decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    _decisionToDFA.emplace_back(_atn.getDecisionState(i), i);
  }
}

UVLcppParser::Initializer UVLcppParser::_init;
