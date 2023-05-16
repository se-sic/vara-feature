#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"

using vara::feature::Feature;
FeatureAddDialog::FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent,
                                   Feature *ParentFeature)
    : QDialog(Parent) {
  setupUi(this);
  NodeNames = QStringList();
  if (ParentFeature) {
    NodeNames.push_back(QString::fromStdString(ParentFeature->getName().str()));
    Nodes->setEnabled(false);
  } else {
    for (const auto &Node : *Graph->getNodes()) {
      NodeNames.push_back(Node->getQName());
    }
  }
  this->Nodes->addItems(NodeNames);
  this->FeatureKind->addItems(QStringList{"Binary", "Numeric"});
  stepOperator->addItems(QStringList{"Add +", "Multiply *", "Exp ^"});
  NumericFeature->setVisible(false);
  connect(FeatureKind, QOverload<int>::of(&QComboBox::activated), this,
          &FeatureAddDialog::featureType);
}

QString FeatureAddDialog::getName() const { return name->text(); }

QString FeatureAddDialog::getParent() const { return Nodes->currentText(); }

void FeatureAddDialog::featureType(int Index) {
  if (Index == 1) {
    NumericFeature->setVisible(true);
  } else {
    NumericFeature->setVisible(false);
  }
}

vara::feature::Feature::FeatureKind FeatureAddDialog::getFeatureKind() {
  return vara::feature::Feature::FeatureKind(FeatureKind->currentIndex());
}

bool FeatureAddDialog::isOptional() const { return optionalCheck->isChecked(); }

QString FeatureAddDialog::getOutputString() const {
  return outputString->text();
}

std::vector<int64_t> stringToIntVector(string &Input) {
  std::stringstream InStream(Input);
  std::vector<int64_t> Out{};

  for (std::string Substring; std::getline(InStream, Substring, ',');) {
    Out.push_back(std::stoi(Substring));
  }
  return Out;
}

/// Retrieve the Feature defined by the dialog this should only be called after
/// the dialog was accepted
std::unique_ptr<Feature> FeatureAddDialog::getFeature() {
  const std::string Name = getName().toStdString();
  const bool Optional = isOptional();
  const std::string OutputString = getOutputString().toStdString();
  switch (getFeatureKind()) {
  case Feature::FeatureKind::FK_BINARY:
    return std::make_unique<vara::feature::BinaryFeature>(
        Name, Optional, std::vector<vara::feature::FeatureSourceRange>(),
        OutputString);
  case Feature::FeatureKind::FK_NUMERIC: {
    return getNumericFeature();
  }
  default:
    return std::make_unique<Feature>(Name);
  }
}

std::unique_ptr<Feature> FeatureAddDialog::getNumericFeature() const {
  const std::string Name = getName().toStdString();
  const bool Optional = isOptional();
  const std::string OutputString = getOutputString().toStdString();
  std::unique_ptr<vara::feature::StepFunction> SF{};
  vara::feature::NumericFeature::ValuesVariantType ValueRange;

  if (range->isChecked()) {
    ValueRange = vara::feature::NumericFeature::ValueRangeType(min->value(),
                                                               max->value());
    if (lhs->isChecked()) {
      SF = std::make_unique<vara::feature::StepFunction>(
          stepOperand->value(), vara::feature::StepFunction::StepOperation(
                                    stepOperator->currentIndex()));
    }
  } else {
    auto ValueString = values->text().toStdString();
    ValueRange = stringToIntVector(ValueString);
  }
  return std::make_unique<vara::feature::NumericFeature>(
      Name, ValueRange, Optional,
      std::vector<vara::feature::FeatureSourceRange>(), OutputString,
      std::move(SF));
}
