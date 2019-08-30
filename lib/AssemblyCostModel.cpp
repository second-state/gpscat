#include <gpscat/AssemblyCostModel.h>
#include <csv-parser/csv.hpp>

#include <string>
#include <fstream>

namespace gpscat {

AssemblyCostModel::AssemblyCostModel(const std::string &filename) {
    // When the filename is invalid, csv::CSVReader will throw a std::runtime_error
    csv::CSVReader reader(filename);

    for(csv::CSVRow& row : reader) {
        costModel[row["Opcode"].get<>()] = row["Cost"].get<CostTy>();
    }
}

} // end namespace gpscat
