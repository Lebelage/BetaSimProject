export module BetaSimLib.Statistics.ParquetSystem;
import std;
import BetaSimLib.Model.Stats.SimulationStatsInfo;
export namespace BetaSimLib::Statistics {

class ParquetSystem {
public:
  ParquetSystem() = default;
  ~ParquetSystem() = default;

public:
  std::expected<bool, std::string>
  Write(const std::string_view file_path,
        std::span<const DepthProfileModel> data) {}

private:
  std::string_view get_type_name(std::string_view type_name) const {
    const auto type = typeid(type_name).name();
    return type;
  }
};
} // namespace BetaSimLib::Statistics