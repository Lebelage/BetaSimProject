//
// Created by Sonora on 27.06.2026.
//
export module BetaSimLib.Statistics.SimulationStatisticsService;

import std;
import Geant4.Externals;

export namespace BetaSimLib::Statistics {

struct DetectorStepRecord {
    std::uint64_t depthStep = 0;

    int layerId = -1;

    std::string layerName;
    std::string materialName;

    double energyDeposit = 0.0;
    double electronHolePairs = 0.0;

    std::uint64_t eventIndex = 0;
};

struct ElectronSpectrumRecord {
    double kineticEnergy = 0.0;
};

class SimulationStatisticsService {
#pragma region Singleton

public:
    static SimulationStatisticsService& Instance() {
        static SimulationStatisticsService instance;
        return instance;
    }

    SimulationStatisticsService(const SimulationStatisticsService&) = delete;
    SimulationStatisticsService& operator=(const SimulationStatisticsService&) = delete;

    SimulationStatisticsService(SimulationStatisticsService&&) = delete;
    SimulationStatisticsService& operator=(SimulationStatisticsService&&) = delete;

private:
    SimulationStatisticsService() {
        ConfigureElectronSpectrum(
            0.0 * Geant4::keV,
            100.0 * Geant4::keV,
            100
        );
    }

    ~SimulationStatisticsService() = default;

#pragma endregion

#pragma region Public Methods

public:
    void Reset() {
        std::lock_guard lock(mutex);

        detectorStepRecords.clear();
        electronSpectrumRecords.clear();
        eventDepthStepCounters.clear();

        std::fill(
            electronSpectrumBins.begin(),
            electronSpectrumBins.end(),
            0
        );
    }

    void ConfigureElectronSpectrum(
        double minEnergy,
        double maxEnergy,
        std::size_t binCount
    ) {
        std::lock_guard lock(mutex);

        spectrumMinEnergy = minEnergy;
        spectrumMaxEnergy = maxEnergy;

        if (binCount == 0) {
            binCount = 1;
        }

        electronSpectrumBins.assign(binCount, 0);
    }

    void RecordDetectorStep(
        std::uint64_t eventIndex,
        int layerId,
        std::string layerName,
        std::string materialName,
        double energyDeposit,
        double electronHolePairs
    ) {
        if (energyDeposit <= 0.0 && electronHolePairs <= 0.0) {
            return;
        }

        std::lock_guard lock(mutex);

        DetectorStepRecord record;

        // depthStep — это глубина/номер шага внутри конкретного event.
        record.depthStep = ++eventDepthStepCounters[eventIndex];

        record.layerId = layerId;
        record.layerName = std::move(layerName);
        record.materialName = std::move(materialName);

        record.energyDeposit = energyDeposit;
        record.electronHolePairs = electronHolePairs;

        record.eventIndex = eventIndex;

        detectorStepRecords.push_back(std::move(record));
    }

    void RecordElectronEntryEnergy(double kineticEnergy) {
        if (kineticEnergy < 0.0) {
            return;
        }

        std::lock_guard lock(mutex);

        electronSpectrumRecords.push_back(
            ElectronSpectrumRecord{
                .kineticEnergy = kineticEnergy
            }
        );

        if (electronSpectrumBins.empty()) {
            return;
        }

        if (spectrumMaxEnergy <= spectrumMinEnergy) {
            return;
        }

        if (kineticEnergy < spectrumMinEnergy || kineticEnergy >= spectrumMaxEnergy) {
            return;
        }

        const auto fraction =
            (kineticEnergy - spectrumMinEnergy) /
            (spectrumMaxEnergy - spectrumMinEnergy);

        auto bin =
            static_cast<std::size_t>(
                fraction * static_cast<double>(electronSpectrumBins.size())
            );

        if (bin >= electronSpectrumBins.size()) {
            bin = electronSpectrumBins.size() - 1;
        }

        electronSpectrumBins[bin]++;
    }

    void PrintSummary() const {
        std::lock_guard lock(mutex);

        Geant4::cout()
            << Geant4::endl
            << "================ BetaSim Statistics ================"
            << Geant4::endl;

        Geant4::cout()
            << "Detector step records: "
            << detectorStepRecords.size()
            << Geant4::endl;

        Geant4::cout()
            << "Electron spectrum records: "
            << electronSpectrumRecords.size()
            << Geant4::endl;

        Geant4::cout()
            << "===================================================="
            << Geant4::endl;
    }

    void ExportToCsvFiles(const std::string& outputDirectory) const {
        std::lock_guard lock(mutex);

        std::filesystem::create_directories(outputDirectory);

        const auto detectorStepsPath =
            outputDirectory + "/detector_steps.csv";

        const auto electronSpectrumPath =
            outputDirectory + "/electron_spectrum.csv";

        ExportDetectorSteps(detectorStepsPath);
        ExportElectronSpectrum(electronSpectrumPath);

        Geant4::cout()
            << "[Statistics] Exported detector steps to: "
            << detectorStepsPath
            << Geant4::endl;

        Geant4::cout()
            << "[Statistics] Exported electron spectrum to: "
            << electronSpectrumPath
            << Geant4::endl;
    }

#pragma endregion

#pragma region CSV Helpers

private:
    static std::string EscapeCsv(const std::string& value) {
        bool needQuotes = false;

        for (char c : value) {
            if (c == ',' || c == '"' || c == '\n' || c == '\r') {
                needQuotes = true;
                break;
            }
        }

        if (!needQuotes) {
            return value;
        }

        std::string result;
        result.reserve(value.size() + 2);

        result += '"';

        for (char c : value) {
            if (c == '"') {
                result += "\"\"";
            } else {
                result += c;
            }
        }

        result += '"';

        return result;
    }

    static void WriteCsvString(
        std::ofstream& file,
        const std::string& value
    ) {
        file << EscapeCsv(value);
    }

    static void WriteCsvNumber(
        std::ofstream& file,
        double value
    ) {
        file << std::setprecision(15) << value;
    }

    static void WriteCsvInteger(
        std::ofstream& file,
        std::uint64_t value
    ) {
        file << value;
    }

#pragma endregion

#pragma region Export

private:
    void ExportDetectorSteps(const std::string& filePath) const {
        std::ofstream file(filePath);

        if (!file.is_open()) {
            Geant4::G4Exception(
                "SimulationStatisticsService",
                "DetectorStepsExportFailed",
                Geant4::G4ExceptionSeverity::JustWarning,
                ("Cannot open file: " + filePath).c_str()
            );

            return;
        }

        file
            << "depth_step,"
            << "layer_id,"
            << "layer_name,"
            << "material,"
            << "energy_deposit_eV,"
            << "energy_deposit_keV,"
            << "electron_hole_pairs,"
            << "event_id"
            << "\n";

        for (const auto& record : detectorStepRecords) {
            WriteCsvInteger(file, record.depthStep);
            file << ",";

            file << record.layerId;
            file << ",";

            WriteCsvString(file, record.layerName);
            file << ",";

            WriteCsvString(file, record.materialName);
            file << ",";

            WriteCsvNumber(file, record.energyDeposit / Geant4::eV);
            file << ",";

            WriteCsvNumber(file, record.energyDeposit / Geant4::keV);
            file << ",";

            WriteCsvNumber(file, record.electronHolePairs);
            file << ",";

            WriteCsvInteger(file, record.eventIndex);
            file << "\n";
        }
    }

    void ExportElectronSpectrum(const std::string& filePath) const {
        std::ofstream file(filePath);

        if (!file.is_open()) {
            Geant4::G4Exception(
                "SimulationStatisticsService",
                "ElectronSpectrumExportFailed",
                Geant4::G4ExceptionSeverity::JustWarning,
                ("Cannot open file: " + filePath).c_str()
            );

            return;
        }

        file
            << "type,"
            << "bin_id,"
            << "energy_min_keV,"
            << "energy_max_keV,"
            << "count,"
            << "raw_entry_id,"
            << "electron_entry_energy_keV"
            << "\n";

        if (!electronSpectrumBins.empty() && spectrumMaxEnergy > spectrumMinEnergy) {
            const auto binWidth =
                (spectrumMaxEnergy - spectrumMinEnergy) /
                static_cast<double>(electronSpectrumBins.size());

            for (std::size_t i = 0; i < electronSpectrumBins.size(); ++i) {
                const auto e0 =
                    spectrumMinEnergy + static_cast<double>(i) * binWidth;

                const auto e1 = e0 + binWidth;

                file << "bin,";
                WriteCsvInteger(file, static_cast<std::uint64_t>(i));
                file << ",";

                WriteCsvNumber(file, e0 / Geant4::keV);
                file << ",";

                WriteCsvNumber(file, e1 / Geant4::keV);
                file << ",";

                WriteCsvInteger(file, electronSpectrumBins[i]);
                file << ",,";

                file << "\n";
            }
        }

        for (std::size_t i = 0; i < electronSpectrumRecords.size(); ++i) {
            file << "raw,,,,,";

            WriteCsvInteger(file, static_cast<std::uint64_t>(i));
            file << ",";

            WriteCsvNumber(
                file,
                electronSpectrumRecords[i].kineticEnergy / Geant4::keV
            );

            file << "\n";
        }
    }

#pragma endregion

#pragma region Variables

private:
    mutable std::mutex mutex;

    std::vector<DetectorStepRecord> detectorStepRecords;
    std::vector<ElectronSpectrumRecord> electronSpectrumRecords;

    // Для depth_step: отдельный счётчик шага по глубине для каждого event.
    std::map<std::uint64_t, std::uint64_t> eventDepthStepCounters;

    double spectrumMinEnergy = 0.0;
    double spectrumMaxEnergy = 100.0 * Geant4::keV;

    std::vector<std::uint64_t> electronSpectrumBins;

#pragma endregion
};

} // namespace BetaSimLib::Statistics