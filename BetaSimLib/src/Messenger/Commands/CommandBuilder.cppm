//
// Created by Sonora on 17.06.2026.
//
export module BetaSimLib.Commands.CommandBuilder;
import std;
import Geant4.Externals;
export namespace BetaSimLib::Commands {
    struct Param {
  std::string name;
  char type;
  bool omittable = false;

  Param(std::string n, char t, bool om = false)
      : name(std::move(n)), type(t), omittable(om) {}
};

class G4CommandBuilder {
public:
  explicit G4CommandBuilder(Geant4::G4UImessenger *owner) : owner(owner) {}

  // Директория
  Geant4::G4UIdirectory *Directory(std::string_view path, std::string_view guidance) {
    auto *dir = new Geant4::G4UIdirectory(std::string(path).c_str());
    dir->SetGuidance(std::string(guidance).c_str());
    return dir;
  }

  // Простая команда без параметров
  Geant4::G4UIcommand *Command(std::string_view path) {
    return new Geant4::G4UIcommand(std::string(path).c_str(), owner);
  }

  // Строковая команда
  Geant4::G4UIcmdWithAString *String(std::string_view path) {
    return new Geant4::G4UIcmdWithAString(std::string(path).c_str(), owner);
  }

  // Число + единицы
  Geant4::G4UIcmdWithADoubleAndUnit *DoubleUnit(std::string_view path,
                                        std::string_view unitCategory) {
    auto *cmd = new Geant4::G4UIcmdWithADoubleAndUnit(std::string(path).c_str(), owner);
    cmd->SetUnitCategory(std::string(unitCategory).c_str());
    return cmd;
  }

  // Вектор + единицы
  Geant4::G4UIcmdWith3VectorAndUnit *Vec3Unit(std::string_view path,
                                      std::string_view unitCategory) {
    auto *cmd = new Geant4::G4UIcmdWith3VectorAndUnit(std::string(path).c_str(), owner);
    cmd->SetUnitCategory(std::string(unitCategory).c_str());
    return cmd;
  }

  // Команда со списком параметров
  template <typename... Args>
  Geant4::G4UIcommand *Params(std::string_view path, Args &&...params) {
    auto *cmd = new Geant4::G4UIcommand(std::string(path).c_str(), owner);

    (AddParam(cmd, std::forward<Args>(params)), ...);

    return cmd;
  }

  // Установка кандидатов для строковой команды
  Geant4::G4UIcmdWithAString *Candidates(std::string_view path,
                                 std::string_view options) {
    auto *cmd = new Geant4::G4UIcmdWithAString(std::string(path).c_str(), owner);
    cmd->SetCandidates(std::string(options).c_str());
    return cmd;
  }

private:
  Geant4::G4UImessenger *owner;

  // Добавление параметра
  void AddParam(Geant4::G4UIcommand *cmd, const Param &p) {
    auto *prm = new Geant4::G4UIparameter(p.name.c_str(), p.type, p.omittable);
    cmd->SetParameter(prm);
  }
};
};