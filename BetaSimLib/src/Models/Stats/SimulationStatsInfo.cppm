module;
#include "nlohmann/json.hpp"
export module BetaSimLib.Models.Stats.SimulationStatsInfo;
import std;
export namespace BetaSimLib::Models::Stats {
struct DepthProfileModel {
  std::uint64_t stepCount = 0;

  int layerId = -1;

  std::string layerName;
  std::string materialName;

  double depthMin_nm = 0.0;
  double depthMax_nm = 0.0;
  double depthCenter_nm = 0.0;

  double totalEnergyDeposit_mev = 0.0;
  double totalElectronHolePairs = 0.0;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DepthProfileModel, stepCount, layerId,
                                   layerName, materialName, depthMin_nm,
                                   depthMax_nm, depthCenter_nm,
                                   totalEnergyDeposit_mev,
                                   totalElectronHolePairs)
} // namespace BetaSimLib::Models::Stats