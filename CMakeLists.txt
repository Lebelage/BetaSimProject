cmake_minimum_required(VERSION 4.3)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

# --- 2. НАСТРОЙКА КОМПИЛЯТОРА И ПУТЕЙ ДО ВЫЗОВА PROJECT() ---
if(APPLE)
    set(LLVM_PREFIX "/opt/homebrew/opt/llvm")
    
    # Жестко форсируем использование Clang от Homebrew
    set(CMAKE_C_COMPILER "${LLVM_PREFIX}/bin/clang" CACHE PATH "" FORCE)
    set(CMAKE_CXX_COMPILER "${LLVM_PREFIX}/bin/clang++" CACHE PATH "" FORCE)
    
    # Явно указываем CMake, где лежит правильный JSON-манифест для import std
    set(CMAKE_CXX_STDLIB_MODULES_JSON "${LLVM_PREFIX}/lib/c++/libc++.modules.json" CACHE PATH "" FORCE)
endif()

# --- 3. ГЛОБАЛЬНЫЕ НАСТРОЙКИ СТАНДАРТА ---
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_MODULE_STD ON) 

project(BetaSimProject LANGUAGES CXX)


if(APPLE)
    # Заставляем компилятор использовать libc++ от LLVM, а не от Apple
    add_compile_options("-stdlib=libc++" "-I${LLVM_PREFIX}/include")
    add_link_options("-stdlib=libc++" "-L${LLVM_PREFIX}/lib" "-Wl,-rpath,${LLVM_PREFIX}/lib")
    
    execute_process(COMMAND xcrun --show-sdk-path
                    OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(CMAKE_OSX_SYSROOT ${CMAKE_OSX_SYSROOT} CACHE PATH "" FORCE)
endif()


find_package(Geant4 REQUIRED)


add_subdirectory(BetaSimLib)
add_subdirectory(Application)