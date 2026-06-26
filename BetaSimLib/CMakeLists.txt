project(BetaSimLib LANGUAGES CXX)

file(GLOB_RECURSE ALL_MODULE_INTERFACES
        "src/*.cppm"
        "externals/*.cppm"
        "utils/*.cppm"
)

set(GEANT_EXT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/externals/geant_externals.cppm")

if (EXISTS "${GEANT_EXT_FILE}")
    list(REMOVE_ITEM ALL_MODULE_INTERFACES "${GEANT_EXT_FILE}")
else ()
    file(GLOB_RECURSE GEANT_EXT_FILE_SEARCH "*/geant_externals.cppm")
    if (GEANT_EXT_FILE_SEARCH)
        list(GET GEANT_EXT_FILE_SEARCH 0 GEANT_EXT_FILE)
        list(REMOVE_ITEM ALL_MODULE_INTERFACES "${GEANT_EXT_FILE}")
    endif ()
endif ()

file(GLOB_RECURSE ALL_SOURCES "src/*.cpp")

add_library(BetaSimLib STATIC)

target_compile_features(BetaSimLib PUBLIC cxx_std_23)
set_target_properties(BetaSimLib PROPERTIES
        CXX_MODULE_STD ON
        CXX_SCAN_FOR_MODULES ON
)

target_sources(BetaSimLib
        PUBLIC
        FILE_SET CXX_MODULES FILES
        ${GEANT_EXT_FILE}
        ${ALL_MODULE_INTERFACES}
        PRIVATE
        ${ALL_SOURCES}
)

target_link_libraries(BetaSimLib PUBLIC ${Geant4_LIBRARIES})
target_include_directories(BetaSimLib PUBLIC ${Geant4_INCLUDE_DIRS})
target_compile_definitions(BetaSimLib PUBLIC ${Geant4_DEFINITIONS})