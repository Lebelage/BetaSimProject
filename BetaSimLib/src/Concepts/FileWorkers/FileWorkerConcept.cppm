export module BetaSimLib.Concepts.FileWorkers.FileWorkerConcept;
import std;
import BetaSimLib.Models.Stats.SimulationStatsInfo;

export namespace BetaSimLib::Concepts::FileWorkers {

using BetaSimLib::Models::Stats::DepthProfileModel;

template <typename T>
concept FileWorkerConcept = requires(T t, const std::string_view file_path,
                                     std::span<const DepthProfileModel> data) {
  {
    t.Write(file_path, data)
  } -> std::same_as<std::expected<bool, std::string>>;
};
} // namespace BetaSimLib::Concepts::FileWorkers