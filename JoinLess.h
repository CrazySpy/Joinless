//
// Created by 蒋希文 on 2021/2/10.
//

#ifndef JOINLESS_JOINLESS_H
#define JOINLESS_JOINLESS_H

#include "Types.h"
#include <map>
#include <unordered_map>
#include <set>
#include <memory>
#include <unordered_set>

class JoinLess {
private:
    double _minPre;

    std::map<FeatureType, std::map<InstanceIdType, InstanceType>> _instances;
    std::map<unsigned int, std::vector<ColocationType>> _prevalent;

    std::map<ColocationType, double> _prevalentIndex;

    std::vector<std::pair<InstanceNameType, InstanceNameType>> _relations;
    std::map<FeatureType, std::map<InstanceIdType, std::unordered_map<FeatureType, std::vector<InstanceIdType>>>> _starNeighborhoods;

    std::map<unsigned int, std::map<ColocationType, std::vector<std::vector<InstanceIdType>>>> _cliqueInstances;

public:
    JoinLess(std::vector<InstanceType> &&instances, std::vector<std::pair<InstanceNameType, InstanceNameType>> &&relations, double minPre);

    std::vector<std::pair<ColocationType, double>> execute();

private:
    void _generateStarNeighborhoods();

    // Generate k-size candidate colocations according to (k-1)-size prevalent colocations.
    ColocationSetType _generateCandidateColocations(int k);

    // Check whether the candidate's subset is prevent.
    bool _isSubsetPrevalent(ColocationType &candidate);

    // Generate and check subset recursively.
    bool _isSubsetPrevalentRecursive(ColocationType &candidate, unsigned int pos, unsigned int remainder,
                                     std::vector<FeatureType> &tmp);

//    void _generateStarCenterSubsetInstancesRecursive(
//            std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>> &instances,
//            const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods, int k, int p, int remainder,
//            std::vector<std::pair<FeatureType, InstanceIdType>> &tmp_instance, ColocationType &tmp_colocation);
//
//    // Generate star instances according to starNeighborhoods vector which is a star neighborhoods' set of an instance.
//    std::map<ColocationType, std::vector<std::vector<std::pair<FeatureType, InstanceIdType>>>>
//    _generateStarCenterSubsetInstances(const std::vector<std::pair<FeatureType, InstanceIdType>> &starNeighborhoods,
//                                        int k);

    void _selectStarInstancesRecursive(int curFeatureId, const int k,
                                       const ColocationType &candidate,
                                       std::unordered_map<FeatureType, std::vector<InstanceIdType>> &starNeighborhoodsMap,
                                       std::vector<std::vector<InstanceIdType>> &starInstances,
                                       std::vector<InstanceIdType> &tmpInstance);

    std::vector<std::vector<InstanceIdType>> _selectStarInstances(
            const InstanceNameType &centerInstanceName,
            std::unordered_map<FeatureType, std::vector<InstanceIdType>> &starNeighborhoodsMap,
            const ColocationType &candidate);

    std::vector<std::vector<InstanceIdType>>
    _filterStarInstances(const ColocationType &candidate);

    double _calculateParticipationIndex(std::map<FeatureType, std::unordered_set<InstanceIdType>> &bitmap);

    // Prune candidates whose prevalence lower than min_pre according to their instances.
    // Current candidates' instances may not be a clique. So call the function 'coarse'.
    bool _isColocationCoarsePrevalent(const ColocationType &candidate,
                                      const std::vector<std::vector<InstanceIdType>> &cliqueInstances);

    // Filter clique instances to _cliqueInstances.
    void _filterCliqueInstances(const ColocationType &candidate,
                                const std::vector<std::vector<InstanceIdType>> &starInstances);

    void _selectPrevalentColocations(const ColocationType &candidate);
};


#endif //JOINLESS_JOINLESS_H
