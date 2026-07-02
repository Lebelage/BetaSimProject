module;
#include <expected>
export module BetaSimLib.Statistics.FileWorkerService;
import std;
import BetaSimLib.Concepts.FileWorkers.FileWorkerConcept;
export namespace BetaSimLib::Statistics {

using BetaSimLib::Concepts::FileWorkers::FileWorkerConcept;

template <FileWorkerConcept T> class FileWorkerService {
#pragma region Singleton
public:
  static FileWorkerService &Instance() {

    static FileWorkerService instance;
    return instance;
  }
#pragma endregion

#pragma region Constructors/Destructor
private:
  FileWorkerService() = default;
  ~FileWorkerService() = default;

  FileWorkerService(const FileWorkerService &) = delete;
  FileWorkerService &operator=(const FileWorkerService &) = delete;

  FileWorkerService(FileWorkerService &&) = delete;
  FileWorkerService &operator=(FileWorkerService &&) = delete;
#pragma endregion

#pragma region Methods
public:
  std::expected<bool, std::string> Initialize() {

    fileWorker = std::make_unique<T>();

    const auto result = fileWorker->Initialize();

    if (!result.has_value())
      return std::unexpected(result.error());
  }
#pragma endregion

#pragma region Variables
private:
  std::unique_ptr<T> fileWorker;
#pragma endregion
};
} // namespace BetaSimLib::Statistics