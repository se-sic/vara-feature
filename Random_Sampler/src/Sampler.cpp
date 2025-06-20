#include <vector>
#include <queue>
#include "vara/Feature/FeatureModelParser.h"
#include "random"
#include <unordered_map>
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Relationship.h"

using namespace vara::feature;
using std::vector;
using std::unique_ptr;
using std::random_device;
using std::unordered_map;
using std::queue;
using std::find;

enum class NodeType { AND, OR, XOR };

NodeType checkType(const FeatureModel &fd, const Feature *f) {
    auto relations = fd.relationships();
    for (const auto &rel : relations) {
        if (rel->getParent() == f) {
            switch (rel->getKind()) {
                case vara::feature::Relationship::RelationshipKind::RK_ALTERNATIVE:
                    return NodeType::XOR;
                case vara::feature::Relationship::RelationshipKind::RK_OR:
                    return NodeType::OR;
                default:
                    return NodeType::AND;
            }
        }
    }
    return NodeType::AND;
}

unordered_map<Feature * , bool> sampleRandomly(const FeatureModel &fd, unordered_map<Feature *, int> &cc, random_device &rd) {
    unordered_map<Feature *, bool> sample;
    Feature *root = fd.getRoot();
    sample[root] = true; 
    queue<Feature *> q;
    q.push(root);

    while(!q.empty()) {
        Feature* f = q.front();
        q.pop();
        auto children = f->getChildren<Feature>();

        if(children.size() == 0){
            if(sample.find(f) == sample.end()){ 
                std::cerr << "The leaf node" << f->getName().str() << "was not uncovered in the sample" << std::endl;
            }
            continue;
        }

        if(!sample.at(f)) {
            auto children = f->getChildren<Feature>();
            for(Feature *c : children) {
                sample[c] = false;
            }
            continue;;
        }

        NodeType type = checkType(fd, f);
        auto children = f->getChildren<Feature>();

        switch(type) {
            case NodeType::AND: {
                for(Feature *c : children) {
                    q.push(c);
                    if(!c->isOptional()){
                        sample[c] = true;
                    } else {
                        auto numCCchild = cc.find(c);
                        if(numCCchild != cc.end() && numCCchild->second > 0) {
                            std::bernoulli_distribution b(numCCchild->second / (numCCchild->second +1));
                            sample[c] = b(rd);
                        } else {
                            sample[c] = false;
                        }
                    }
                }
                break;
            }
             case NodeType::OR: {
                vector<Feature *> s;
                while(s.empty()) {
                    s.clear();
                    for(Feature *c: children) {
                        auto numCChild = cc.find(c);
                        if(numCChild != cc.end() && numCChild->second > 0) {
                            std::bernoulli_distribution b(numCChild->second / (numCChild->second + 1));
                            if(b(rd)) {
                                s.push_back(c);
                            }
                        }
                    }
                }
                for(Feature *c : children) {
                    if(find(s.begin(), s.end(), c) != s.end()) {
                        sample[c] = true;
                        q.push(c);
                    } else {
                        sample[c] = false;
                    }
                }
                break;
            }
            case NodeType::XOR: {
                vector<double> weights;
                std::vector<vara::feature::Feature*> childVec(children.begin(), children.end());
                for(Feature *c : children) {
                    auto numCCchild = cc.find(c);
                    weights.push_back(static_cast<double>(numCCchild->second));
                }

                std::discrete_distribution<double> dist(weights.begin(), weights.end());
                Feature *c_prime = childVec[dist(rd)];
                for(Feature *c: children) {
                    if(c == c_prime) {
                        sample[c] = true;
                        q.push(c);
                    } else {
                        sample[c] = false;
                    }
                }
                break;
            }
        }
    }
    return sample;
}


