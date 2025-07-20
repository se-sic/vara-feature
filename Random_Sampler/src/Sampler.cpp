#include "Sampler.hpp"
#include "NodeType.hpp"

NodeType checkType( const FeatureModel &fd,  FeatureTreeNode *f) {
    // for(auto &rs : fd.relationships()) {
    //     if(rs->getParent() != f) {
    //         continue;
    //     }
    //     switch (rs->getKind()) {
    //         case Relationship::RelationshipKind::RK_ALTERNATIVE:
    //             return NodeType::XOR;
    //             break;
    //         case Relationship::RelationshipKind::RK_OR:
    //             return NodeType::OR;
    //             break;
    //         default:
    //             if(f->isLeaf()) {
    //                 return NodeType::LEAF; 
    //             }
    //             return NodeType::AND;
    //             break;
    //     }
    // }
    auto rltsp = f->getChildren<Relationship>();
     if (!rltsp.empty()) {
        auto *rel = *rltsp.begin();
            switch(rel->getKind()) {
                case Relationship::RelationshipKind::RK_ALTERNATIVE:
                    return NodeType::XOR;
                case Relationship::RelationshipKind::RK_OR:
                    return NodeType::OR;
                default:
                    if(f->isLeaf()) {
                        return NodeType::LEAF; 
                    }
                    return NodeType::AND; // fallback
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

        // if(children.size() == 0){
        //     if(sample.find(f) == sample.end()){ 
        //         std::cerr << "The leaf node" << f->getName().str() << "was not uncovered in the sample" << std::endl;
        //     }
        //     continue;
        // }

        if(!sample.at(f)) {
            auto children = f->getChildren<Feature>();
            for(Feature *c : children) {
                sample[c] = false;
            }
            continue;;
        }

        NodeType type = checkType(fd, f);

        switch(type) {
            case NodeType::AND: {
                std::cout << "Sampling AND node: " << f->getName().str() << std::endl;
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
                std::cout << "Sampling OR node: " << f->getName().str() << std::endl;
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

                std::discrete_distribution<int> dist(weights.begin(), weights.end());
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
            case NodeType::LEAF: {
                break;
            }
        }
    }
    return sample;
}


