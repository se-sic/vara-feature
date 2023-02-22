//
// Created by simon on 29.11.22.
//

#include "FeatureAddDialog.h"
#include "graph/FeatureModelGraph.h"
#include "graph/FeatureNode.h"
using vara::feature::Feature;
using vara::feature::FeatureModel;
FeatureAddDialog::FeatureAddDialog(FeatureModelGraph *Graph, QWidget *Parent,
                                   Feature *ParentFeature)
    : QDialog(Parent) {
  setupUi(this);
  NodeNames = QStringList();
  if (!ParentFeature) {
    for (const auto &Node : *Graph->getNodes()) {
      NodeNames.push_back(Node->getQName());
    }
  } else {
    NodeNames.push_back(QString::fromStdString(ParentFeature->getName().str()));
    Nodes->setEnabled(false);
  }
  this->Nodes->addItems(NodeNames);
  this->FeatureKind->addItems(QStringList{"Binary", "Numeric"});
  stepOperator->addItems(QStringList{"Add +", "Multiply *", "Exp ^"});
  NumericFeature->setVisible(false);
  connect(FeatureKind, &QComboBox::activated, this,
          &FeatureAddDialog::featureType);
}

QString FeatureAddDialog::getName() { return name->text(); }

QString FeatureAddDialog::getParent() { return Nodes->currentText(); }

void FeatureAddDialog::featureType(int index) {
  if (index == 1) {
    NumericFeature->setVisible(true);
  } else {
    NumericFeature->setVisible(false);
  }
}
vara::feature::Feature::FeatureKind FeatureAddDialog::getFeatureKind() {
  return vara::feature::Feature::FeatureKind(FeatureKind->currentIndex());
}
bool FeatureAddDialog::isOptional() { return optinalCheck->isChecked(); }
QString FeatureAddDialog::getOutpuString() { return outpuString->text(); }

std::vector<int64_t> stringToIntVector(string &Input) {
  std::stringstream InStream(Input);
  std::vector<int64_t> Out{};

  for (std::string Substring; std::getline(InStream, Substring, ',');) {
    Out.push_back(std::stoi(Substring));
  }
  return Out;
}

std::unique_ptr<Feature> FeatureAddDialog::getFeature() {
  const std::string Name = getName().toStdString();
  const bool Optional = isOptional();
  const std::string OutputString = getOutpuString().toStdString();
  vara::feature::NumericFeature::ValuesVariantType ValueRange;
  switch (getFeatureKind()) {

  case Feature::FeatureKind::FK_BINARY:
    return std::make_unique<vara::feature::BinaryFeature>(
        Name, Optional, std::vector<vara::feature::FeatureSourceRange>(),
        OutputString);
  case Feature::FeatureKind::FK_NUMERIC: {
    std::unique_ptr<vara::feature::StepFunction> SF{};
    if (range->isChecked()) {
      ValueRange = vara::feature::NumericFeature::ValueRangeType(min->value(),
                                                                 max->value());
      if (lhs->isChecked()) {
        SF = std::make_unique<vara::feature::StepFunction>(
            stepOperant->value(), vara::feature::StepFunction::StepOperation(
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
  default:
    return std::make_unique<Feature>(Name);
  }
}
