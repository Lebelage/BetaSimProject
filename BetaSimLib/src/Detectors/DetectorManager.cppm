export module BetaSim.Detectors.DetectorManger;
import std;
import BetaSimLib.Detectors.DetectorWrapper;
import BetaSimLib.Concepts.Detectors;
export namespace BetaSimLib::Detectors {
class DetectorManager {
#pragma region Constructors/Destructors
public:
  DetectorManager() { detWrapper = new DetectorWrapper(); };
  ~DetectorManager() {};

  DetectorManager(const DetectorManager &) = delete;
  DetectorManager &operator=(const DetectorManager &) = delete;

  DetectorManager(const DetectorManager &&) = delete;
  DetectorManager &operator=(const DetectorManager &&) = delete;
#pragma endregion

#pragma region Methods
public:
  template<Concepts::Detectors::DetectorConstructionConcept Detector>
       void SetDetector(std::unique_ptr<Detector> detector) {
    detWrapper->SetBuilder(std::move(detector));
  }

  DetectorWrapper *GetCurrentDetectorPointer() {
    return detWrapper;
  }
#pragma endregion

#pragma region Variables
private:
  DetectorWrapper *detWrapper;
#pragma endregion
};
} // namespace BetaSimLib::Detectors