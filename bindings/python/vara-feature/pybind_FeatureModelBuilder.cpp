#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Feature/FeatureModelTransaction.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "pybind_common.h"
#include <filesystem>
#include <memory>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_model_builder_module(py::module &M){
    py::class_<vf::FeatureModelBuilder>(M, "FeatureModelBuilder")
        .def(py::init([](){
                      return vf::FeatureModelBuilder();
                      }))
        .def(
            "make_binary_feature",
            [](vf::FeatureModelBuilder &FMB, std::string feature_name, bool Opt){
                return FMB.makeFeature<vf::BinaryFeature>(feature_name); 
            },
            R"pbdoc(Create a new binary `Feature` with the given `feature_name`.)pbdoc"
        )
        .def(
            "add_an_edge",
            [](vf::FeatureModelBuilder &FMB, std::string parent_name, std::string feature_name){
                return FMB.addEdge(parent_name, feature_name);
            },
            R"pbdoc(Create a new parent-child edge between the two features provided, `parent_name` and `feature_name`.)pbdoc"
        )
        .def(
            "emplace_relationship",
            [](vf::FeatureModelBuilder &FMB, std::string parent_name, std::string relation) {
                vf::Relationship::RelationshipKind RK;
                if (relation == "OR"){
                    RK = vf::Relationship::RelationshipKind::RK_OR;
                }
                else if (relation == "XOR"){
                    RK = vf::Relationship::RelationshipKind::RK_ALTERNATIVE;
                }
            return FMB.emplaceRelationship(RK, parent_name);
            },
            R"pbdoc(Create a relation `relation` among children of the parent feature `parent_name`.)pbdoc"
        )
        .def(
            "add_boolean_constraint",
            [](vf::FeatureModelBuilder &FMB, vf::Constraint &C){
                return FMB.addConstraint(std::make_unique<vf::FeatureModel::BooleanConstraint> (C.clone()) );
            },
            R"pbdoc(Add a new boolean constraint `C` to the feature model.)pbdoc"
        )
        .def(
            "add_non_boolean_constraint",
            [](vf::FeatureModelBuilder &FMB, vf::Constraint &C){
                return FMB.addConstraint(std::make_unique<vf::FeatureModel::NonBooleanConstraint> (C.clone()) );
            },
            R"pbdoc(Add a new non boolean constraint `C` to the feature model.)pbdoc"
        )
        .def(
            "set_vm_name",
            [](vf::FeatureModelBuilder &FMB, std::string name){
                return FMB.setVmName(name);
            },
            R"pbdoc(Set vm Name to `name`.)pbdoc"
        )
        .def(
            "set_path",
            [](vf::FeatureModelBuilder &FMB, fs::path path){
                return FMB.setPath(path);
            },
            R"pbdoc(Set path to `path`.)pbdoc"
        )
        .def(
            "set_commit",
            [](vf::FeatureModelBuilder &FMB, fs::path commit){
                return FMB.setCommit(commit);
            },
            R"pbdoc(Set commit to `commit`.)pbdoc"
        )
        .def(
            "make_root",
            [](vf::FeatureModelBuilder &FMB, std::string name){
                return FMB.makeRoot(name);
            },
            R"pbdoc(Make the feature `name` as the root of the feature model.)pbdoc"
        )
        .def(
            "build_feature_model",
            [](vf::FeatureModelBuilder &FMB){
                return FMB.buildFeatureModel();
            },
            R"pbdoc(Build the feature model.)pbdoc"
        );
}