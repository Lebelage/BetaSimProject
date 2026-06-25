project(BetaSimApp LANGUAGES CXX)

# --- Исходники приложения ---
file(GLOB_RECURSE APP_MODULES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cppm"
)

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# --- Исполняемый файл ---
add_executable(Application)

# --- C++23 + modules + import std ---
target_compile_features(Application PRIVATE cxx_std_23)

set_target_properties(Application PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF

    CXX_SCAN_FOR_MODULES ON
    CXX_MODULE_STD ON
)

# --- Подключение модулей и обычных .cpp ---
if(APP_MODULES)
    target_sources(Application
        PUBLIC
            FILE_SET CXX_MODULES FILES
                ${APP_MODULES}
    )
endif()

if(APP_SOURCES)
    target_sources(Application
        PRIVATE
            ${APP_SOURCES}
    )
endif()

# --- Линковка библиотеки ---
target_link_libraries(Application
    PRIVATE
        BetaSimLib
)