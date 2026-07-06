module;

#include <cstddef> 

#define BETA_SIM_REFLECT_FIELD(StructType, FieldName) \
    BetaSimLib::Models::Stats::FieldMeta{ \
        #FieldName, \
        BetaSimLib::Models::Stats::get_type_name<decltype(StructType::FieldName)>(), \
        offsetof(StructType, FieldName) \
    }

export module BetaSimLib.Models.Stats.SimulationStatsInfo;
import std;

export namespace BetaSimLib::Models::Stats {

template <typename T> 
consteval std::string_view get_type_name() {
    std::string_view name = __PRETTY_FUNCTION__;
    
    // 1. Находим, где заканчивается имя типа (перед закрывающей скобкой ']')
    auto end = name.find_last_not_of("]");
    if (end != std::string_view::npos) {
        name = name.substr(0, end + 1);
    }
    
    // 2. Находим, где начинается сам тип (сразу после "T = ")
    auto start = name.find("T = ") + 4;
    
    // 3. Возвращаем чистый остаток строки
    return name.substr(start);
}

struct FieldMeta {
    std::string_view name;
    std::string_view type;
    std::size_t offset; 
};

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

template <typename T>
struct ModelTraits;

template <>
struct ModelTraits<DepthProfileModel> {
    static constexpr std::string_view name = "DepthProfileModel";
    
    // static constexpr гарантирует compile-time вычисление и бессмертие строк
    static constexpr std::array fields = {
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, stepCount),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, layerId),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, layerName),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, materialName),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, depthMin_nm),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, depthMax_nm),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, depthCenter_nm),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, totalEnergyDeposit_mev),
        BETA_SIM_REFLECT_FIELD(DepthProfileModel, totalElectronHolePairs)
    };
};

} // namespace BetaSimLib::Models::Stats