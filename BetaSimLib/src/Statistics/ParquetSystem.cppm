module;
#include "arrow/api.h"
#include <arrow/type_fwd.h>
export module BetaSimLib.Statistics.ParquetSystem;
import std;
import BetaSimLib.Models.Stats.SimulationStatsInfo;

export namespace BetaSimLib::Statistics {

using namespace BetaSimLib::Models::Stats;

class ParquetSystem {
public:
  ParquetSystem() = default;
  ~ParquetSystem() = default;

public:
  std::expected<bool, std::string> Write(const std::string_view file_path, std::span<const DepthProfileModel> data) {

    if(data.empty()) return std::unexpected("Data is empty!");

    using Traits = ModelTraits<DepthProfileModel>;

    arrow::FieldVector arrow_fields;

    for (const auto& field : Traits::fields)
    {
      arrow_fields.push_back(arrow::field(std::string(field.name), get_arrow_type(field.type)));
    }
  }

private:
  std::shared_ptr<arrow::DataType> get_arrow_type(std::string_view type_name) const {
    if(type_name == "std::uint64_t") {
      return arrow::uint64();
    } else if(type_name == "int") {
      return arrow::int32();
    } else if(type_name == "double") {
      return arrow::float64();
    } else if(type_name == "std::string") {
      return arrow::utf8();
    } else {
      throw std::runtime_error("Unsupported type: " + std::string(type_name));
    }
  }
};
} // namespace BetaSimLib::Statistics