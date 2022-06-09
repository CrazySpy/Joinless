//
// Created by 蒋希文 on 2021/2/10.
//

#include "JoinLess.h"
#include <algorithm>
#include <unordered_set>
#include <numeric>

extern bool hasRelation(const InstanceType &, const InstanceType &);

JoinLess::JoinLess(std::vector<InstanceType> &&instances, std::vector<std::pair<InstanceNameType, InstanceNameType>> &&relations, double minPre)
    : _minPre(minPre),
      _relations(std::move(relations)) {
    for(auto &instance : instances) {
        auto feature = std::get<Feature>(instance);
        auto id = std::get<Id>(instance);

        _cliqueInstances[1][{feature}].push_back({id});
        _instances[feature][id] = instance;
    }
    ;

    // Generate P_1
    auto &prevalent = _prevalent[1];
    for(auto &instancePair : _instances) {
        prevalent.push_back({instancePair.first});
    }
}

void JoinLess::_generateStarNeighborhoods() {
    for(auto &relation : _relations) {
        auto &starCenter = relation.first;
        auto &starEdge = relation.second;

        _starNeighborhoods[starCenter.first][starCenter.second][starEdge.first].push_back(starEdge.second);
    }
}

bool JoinLess::_isSubsetPrevalentRecursive(
        ColocationType &candidate,
        unsigned int pos,
        unsigned int remainder,
        std::vector<FeatureType> &tmp) {
    int size = candidate.size() - 2;
    int size2 = candidate.size() - 1 - 2;

    if(!remainder) {
        auto &prevalent = _prevalent[candidate.size() - 1];
        return std::binary_search(prevalent.begin(), prevalent.end(), tmp);
    }
    if(pos == size) return true;
    if(remainder + pos > size) return true;

    if(_isSubsetPrevalentRecursive(candidate, pos + 1, remainder, tmp)) {
        tmp[size2 - remainder] = candidate[pos];
        return _isSubsetPrevalentRecursive(candidate, pos + 1, remainder - 1, tmp);
    }

    return false;
}

bool JoinLess::_isSubsetPrevalent(ColocationType &candidate) {
    unsigned int size = candidate.size();
    if(size <= 2) return true;

    std::vector<FeatureType> tmp(size - 1);
    tmp[size - 2] = candidate[size - 1];
    tmp[size - 3] = candidate[size - 2];

    return _isSubsetPrevalentRecursive(candidate, 0, size - 3, tmp);
}

ColocationSetType JoinLess::_generateCandidateColocations(int k) {
    auto &prevalent = _prevalent[k - 1];

    ColocationSetType candidates;
    for (auto it1 = prevalent.begin(); it1 != prevalent.end(); ++it1) {
        auto &colocation1 = (*it1);

        for(auto it2 = it1 + 1; it2 !=prevalent.end(); ++it2) {
            auto &colocation2 = (*it2);

            bool canMerge = true;
            for (unsigned int idx = 0; idx < k - 2; ++idx) {
                if (colocation1[idx] != colocation2[idx]) {
                    canMerge = false;
                    break;
                }
            }

            if (canMerge) {
                // Generate a candidate colocation by merging two colocations.
                ColocationType candidate(colocation1.begin(), colocation1.end() - 1);
                candidate.push_back(std::min(colocation1.back(), colocation2.back()));
                candidate.push_back(std::max(colocation1.back(), colocation2.back()));

                // Check whether all the subsets of the candidate are prevalent.
                // If not, prune it.
                if (_isSubsetPrevalent(candidate)) {
                    candidates.push_back(candidate);
                }
            }
        }
    }

    return candidates;
}

//void JoinLess::_generateStarCenterSubsetInstancesRecursive(
//        std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &instances,
//        const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
//        int k,
//        int p,
//        int remainder,
//        std::vector<std::pair<FeatureType, InstanceIdType>> &tmp_instance,
//        ColocationType &tmp_colocation) {
//
//    if(!remainder) {
//        instances[tmp_colocation].push_back(tmp_instance);
//        return;
//    }
//
//    if(p + remainder > starNeighborhoods.size()) return;
//
//    if(starNeighborhoods[p].first != tmp_colocation[k - remainder - 1]) {
//        tmp_instance[k - remainder] = starNeighborhoods[p];
//        tmp_colocation[k - remainder] = starNeighborhoods[p].first;
//        _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder - 1, tmp_instance,
//                                                    tmp_colocation);
//    }
//
//    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, p + 1, remainder, tmp_instance, tmp_colocation);
//}
//
//std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>
//        JoinLess::_generateStarCenterSubsetInstances(
//                const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
//                int k) {
//    // Collect the star instances of colocations.
//    std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> instances;
//
//    std::vector<std::pair<FeatureType, InstanceIdType>> tmp_instance(k);
//    ColocationType tmp_colocation(k);
//
//    tmp_instance[0] = starNeighborhoods[0];
//    tmp_colocation[0] = starNeighborhoods[0].first;
//
//    _generateStarCenterSubsetInstancesRecursive(instances, starNeighborhoods, k, 1, k - 1, tmp_instance, tmp_colocation);
//
//    return instances;
//}

void JoinLess::_selectStarInstancesRecursive(int curFeatureId, const int k,
                                             const ColocationType &candidate,
                                             std::unordered_map<FeatureType, std::vector<InstanceIdType>> &starNeighborhoodsMap,
                                             std::vector<std::vector<InstanceIdType>> &starInstances,
                                             std::vector<InstanceIdType> &tmpInstance) {
    if(curFeatureId == k) {
        starInstances.push_back(tmpInstance);
        return;
    }

    const FeatureType curFeature = candidate[curFeatureId];
    for(int i = 0; i < starNeighborhoodsMap[curFeature].size(); ++i) {
        tmpInstance[curFeatureId] = starNeighborhoodsMap[curFeature][i];
        _selectStarInstancesRecursive(curFeatureId + 1, k, candidate, starNeighborhoodsMap, starInstances, tmpInstance);
    }
}

std::vector<std::vector<InstanceIdType>>
JoinLess::_selectStarInstances(const InstanceNameType &centerInstanceName,
                               std::unordered_map<FeatureType, std::vector<InstanceIdType>> &starNeighborhoodsMap,
                               const ColocationType &candidate) {
    int k = candidate.size();

    std::vector<std::vector<InstanceIdType>> starInstances;
    std::vector<InstanceIdType> tmpInstance(k);

    tmpInstance[0] = centerInstanceName.second;

    _selectStarInstancesRecursive(1, k, candidate, starNeighborhoodsMap, starInstances, tmpInstance);

    return starInstances;
}

std::vector<std::vector<InstanceIdType>> JoinLess::_filterStarInstances(const ColocationType &candidate) {
    auto starCenterFeature = candidate[0];

    std::vector<std::vector<InstanceIdType>> candidateStarInstances;
    for(auto &[starCenterId, starNeighborhoodsMap] : _starNeighborhoods[starCenterFeature]) {
        auto starInstances = _selectStarInstances({starCenterFeature, starCenterId}, starNeighborhoodsMap, candidate);

        candidateStarInstances.insert(candidateStarInstances.end(), starInstances.begin(), starInstances.end());
    }

    return candidateStarInstances;
}

double JoinLess::_calculateParticipationIndex(std::map<FeatureType, std::unordered_set<InstanceIdType>> &bitmap) {
    double participationIndex = 1;
    for(auto &[feature, idSet] : bitmap) {
        participationIndex = std::min(idSet.size() * 1.0 / _instances[feature].size(), participationIndex);
    }
    return participationIndex;
}

bool JoinLess::_isColocationCoarsePrevalent(const ColocationType &candidate,
                                            const std::vector<std::vector<InstanceIdType>> &cliqueInstances) {
    if(cliqueInstances.empty()) return false;

    std::map<FeatureType, std::unordered_set<InstanceIdType>> bitmap;

    for(auto &cliqueInstance : cliqueInstances) {
        for(int i = 0; i < cliqueInstance.size(); ++i) {
            auto feature = candidate[i];
            auto id = cliqueInstance[i];

            bitmap[feature].insert(id);
        }
    }

    double participationIndex = _calculateParticipationIndex(bitmap);
    return participationIndex >= _minPre;
}

void JoinLess::_filterCliqueInstances(const ColocationType &candidate,
                                      const std::vector<std::vector<InstanceIdType>> &starInstances) {
    int k = candidate.size();

    ColocationType starEdgeFeature(candidate.begin() + 1, candidate.end());

    for(auto &starInstance : starInstances) {
        // step 9
        if(k == 2) {
            _cliqueInstances[2][candidate].push_back(starInstance);
        }
        else {
            std::vector<InstanceIdType> starEdges((starInstance).begin() + 1, (starInstance).end());

            // If starEdges is a clique, then candidate is a clique.
            if (_cliqueInstances[k - 1].count(starEdgeFeature) &&
                std::binary_search(_cliqueInstances[k - 1][starEdgeFeature].begin(), _cliqueInstances[k - 1][starEdgeFeature].end(), starEdges)) {
                _cliqueInstances[k][candidate].push_back(starInstance);
            }
        }
    }
}

void JoinLess::_selectPrevalentColocations(const ColocationType &candidate) {
    int k = candidate.size();

    if(!_cliqueInstances.count(k) || !_cliqueInstances[k].count(candidate)) return;

    std::map<FeatureType , std::unordered_set<InstanceIdType>> bitmap;

    auto &cliqueInstances = _cliqueInstances[k][candidate];
    for(auto &rowInstance : cliqueInstances) {
        for (int i = 0; i < k; ++i) {
            auto &feature = candidate[i];
            auto &id = rowInstance[i];

            bitmap[feature].insert(id);
        }
    }

    double participationIndex = _calculateParticipationIndex(bitmap);
    if(participationIndex >= _minPre) {
        _prevalent[k].push_back(candidate);
        _prevalentIndex[candidate] = participationIndex;
    } else {
        _cliqueInstances[k].erase(candidate);
    }
}

std::vector<std::pair<ColocationType, double>> JoinLess::execute() {
    _generateStarNeighborhoods();
    _relations.clear();
    int k = 2;
    while(!_prevalent[k - 1].empty()) {
        ColocationSetType candidates = _generateCandidateColocations(k);

        for(auto &candidate : candidates) {
            auto SI = _filterStarInstances(candidate);
            if (k == 2) {
                // Step 9 is done in _filterCliqueInstances function.
                // The function do special when k = 2.
                _filterCliqueInstances(candidate, SI);
            } else {
                if(_isColocationCoarsePrevalent(candidate, SI)) {
                    // clique instances CI is the private member '_cliqueInstances'.
                    _filterCliqueInstances(candidate, SI);
                }
            }
            _selectPrevalentColocations(candidate);
        }
        _cliqueInstances.erase(k - 1);


        //_generateRules(k);
        std::cout << "step " << k << ":" << std::endl;
        for (auto& colocation : _prevalent[k]) {
            std::cout << std::accumulate(std::begin(colocation), std::end(colocation), std::string(),
                [](const std::string &partial, const FeatureType &feature) {
                    return partial + feature + ';';
                }) << std::endl;
        }
        std::cout << "Size : " << _prevalent[k].size() << std::endl;


        ++k;
    }

    std::vector<std::pair<ColocationType, double>> result;
    for(int i = 2; i <= k - 2; ++i) {
        for(auto &colocation : _prevalent[i]) {
            result.push_back({colocation, _prevalentIndex[colocation]});
        }
    }
    return result;
}
