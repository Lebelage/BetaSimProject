export module BetaSimLib.Concepts.FileWorkers.FileWorkerConcept;
import std;
import BetaSimLib.Models.Stats.SimulationStatsInfo;

export namespace BetaSimLib::Concepts::FileWorkers {

using BetaSimLib::Models::Stats::DepthProfileModel;

template <typename T>
concept FileWorkerConcept = requires(T t, const DepthProfileModel &data) {
  { t.Initialize(data) } -> std::same_as<std::expected<bool, std::string>>;
  { t.Write(data) } -> std::same_as<void>;
};
} // namespace BetaSimLib::Concepts::FileWorkers