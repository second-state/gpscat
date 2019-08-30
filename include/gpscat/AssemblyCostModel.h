#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

namespace gpscat {

using CostTy = std::int32_t;

class AssemblyCostModel {
public:
    AssemblyCostModel(const std::string &filename);

    CostTy get(const std::string &instName) const {
        return costModel.at(instName);
    }

    void set(const std::string &instName, const CostTy& cost) {
        costModel[instName] = cost;
    }

    bool hasInst(const std::string &instName) const {
        return costModel.find(instName) != costModel.end();
    }

private:
    std::unordered_map<std::string, CostTy> costModel;
};

} // end namespace gpscat
