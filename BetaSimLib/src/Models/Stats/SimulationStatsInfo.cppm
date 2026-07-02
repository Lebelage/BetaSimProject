export module BetaSimLib.Models.Stats.SimulationStatsInfo;
import std;
export namespace BetaSimLib::Models::Stats {
struct DepthProfileModel {
  std::uint64_t stepCount = 0;

  int layerId = -1;

  std::string layerName;
  std::string materialName;

  double depthMin = 0.0;
  double depthMax = 0.0;
  double depthCenter = 0.0;

  double totalEnergyDeposit = 0.0;
  double totalElectronHolePairs = 0.0;
};
} // namespace BetaSimLib::Models::Stats