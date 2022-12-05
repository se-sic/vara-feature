#include "pybind11/attr.h"
#include "pybind_common.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModelBuilder.h"
#include "vara/Feature/FeatureModelTransaction.h"

#include "pybind11/detail/common.h"
#include "pybind11/pybind11.h"

#include <filesystem>
#include <iostream>

namespace vf = vara::feature;
namespace py = pybind11;

void init_feature_model_builder_module(py::module &M){
    py::class_<vf::FeatureModelBuilder>(M,  "FeatureModelBuilder")
        .def(
            "make_binary_feature",
            [](vf::FeatureModelBuilder &FMB, std::string feature_name, bool Opt){
                return FMB.makeFeature<vf::BinaryFeature>(feature_name); 
            }    
        )
///        .def(
///            "make_numeric_feature",
///            [](vf::FeatureModelBuilder &FMB, std::string feature_name, bool Opt){
///                return FMB.makeFeature<vf::NumericFeature>(feature_name); 
///            }    
///        )
        .def(
            "add_an_edge",
            [](vf::FeatureModelBuilder &FMB, std::string parent_name, std::string feature_name){
                return FMB.addEdge(parent_name, feature_name);
            }    
        )
        .def(
            "get_parents_name",
            [](vf::FeatureModelBuilder &FMB, std::string feature_name){
                return FMB.getParentName(feature_name);
            }    
        )
        .def(
            "emplace_relationship",
            [](vf::FeatureModelBuilder &FMB, vf::Relationship::RelationshipKind RK,
                                    std::string parent_name){
                return FMB.emplaceRelationship(RK, parent_name);
            }    
        )
///        .def(
///            "add_constraint",
///            [](vf::FeatureModelBuilder &FMB, std::unique_ptr<vf::FeatureModel::ConstraintTy> C){
///                return FMB.addConstraint(C);
///            }    
///        )                
        .def(
            "set_vm_name",
            [](vf::FeatureModelBuilder &FMB, std::string name){
                return FMB.setVmName(name);
            }    
        )                
        .def(
            "set_path",
            [](vf::FeatureModelBuilder &FMB, fs::path path){
                return FMB.setPath(path);
            }    
        )                
        .def(
            "set_commit",
            [](vf::FeatureModelBuilder &FMB, fs::path commit){
                return FMB.setCommit(commit);
            }    
        )                
        .def(
            "make_root",
            [](vf::FeatureModelBuilder &FMB, std::string name){
                return FMB.makeRoot(name);
            }    
        )
        .def(
            "build_feature_model",
            [](vf::FeatureModelBuilder &FMB){
                return FMB.buildFeatureModel();
            }    
        );                 
}