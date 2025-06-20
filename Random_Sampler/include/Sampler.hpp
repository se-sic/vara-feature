#ifndef RANDOM_SAMPLER_HPP
#define RANDOM_SAMPLER_HPP

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

unordered_map<Feature * , bool> sampleRandomly(const FeatureModel &fd, unordered_map<Feature *, int> &cc, random_device &rd);

#endif // RANDOM_SAMPLER_HPP