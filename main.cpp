#include <iostream>
#include "JoinLess.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <tuple>
#include <chrono>
#include <algorithm>

#include "CSVReader/CSVReader.h"

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

using namespace std;

double maxDistance;

int main(int argc, char **argv) {
    if(argc != 4) {
        cout << "Argument number must be 3" << endl;
        cout << argv[0] << " [minimum_prevalence] [dataset_path] [relation_path]" << endl;
        return 0;
    }
    double minPre = stod(argv[1]);
    string datasetPath(argv[2]);
    string relationPath(argv[3]);

    std::vector<InstanceType> instances;
    CSVReader instanceReader(datasetPath);
    while(instanceReader.hasNext()) {
        auto line = instanceReader.getNextRecord();
        auto feature = line[0];
        auto id = stoul(line[1]);
        auto X = stod(line[2]);
        auto Y = stod(line[3]);

        instances.push_back({feature, id, {X, Y}});
    }
    cout << instances.size() <<endl;

    std::vector<std::pair<InstanceNameType, InstanceNameType>> relations;
    CSVReader relationReader(relationPath);
    while(relationReader.hasNext()) {
        auto line = relationReader.getNextRecord();
        auto feature1 = line[0];
        auto feature2 = line[2];
        auto id1 = stoul(line[1]);
        auto id2 = stoul(line[3]);

        relations.push_back({{feature1, id1}, {feature2, id2}});
    }
    cout << relations.size() << endl;

    high_resolution_clock::time_point beginTime = high_resolution_clock::now();

    JoinLess joinLess(std::move(instances), std::move(relations), minPre);
    auto colocations = joinLess.execute();

    high_resolution_clock::time_point endTime = high_resolution_clock::now();
    milliseconds timeInterval = std::chrono::duration_cast<milliseconds>(endTime - beginTime);

    ofstream result_ofs("colocation_result.txt");
    for(auto &[colocation, PI] : colocations) {
        bool isFirst = true;
        for(auto feature : colocation) {
            if(!isFirst) {
                result_ofs << ' ';
            } else {
                isFirst = false;
            }
            result_ofs << feature;
        }
        result_ofs << endl;
    }

    std::cout << timeInterval.count() << "ms\n";

    return 0;
}
/*
    instances.push_back({1, 'A', {0, 2}});
    instances.push_back({2, 'A', {11, 3}});
    instances.push_back({3, 'A', {12,3}});
    instances.push_back({4, 'A', {10, 4}});

    instances.push_back({1, 'B', {0, 1}});
    instances.push_back({2, 'B', {13,1.5}});
    instances.push_back({3, 'B', {20, 5}});
    instances.push_back({4, 'B', {11,2}});
    instances.push_back({5, 'B', {31,11}});

    instances.push_back({1, 'C', {12,1.5}});
    instances.push_back({2, 'C', {0, 3}});
    instances.push_back({3, 'C', {30, 10}});
 */