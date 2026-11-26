cmake_minimum_required(VERSION 3.16)
project(PokeBattle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# ---- Module 1 + 2: engine library (no SFML dependency) ----
add_library(pokecore STATIC
  src/game/Types.cpp          src/game/MoveDatabase.cpp  src/game/StatusQueue.cpp
  src/game/Pokemon.cpp        src/game/DamageCalculator.cpp
  src/game/Battle.cpp)
target_include_directories(pokecore PUBLIC include)
if(MSVC)
  target_compile_options(pokecore PRIVATE /W4)
else()
  target_compile_options(pokecore PRIVATE -Wall -Wextra)
endif()

# ---- headless tests (DSA + engine) ----
add_executable(test_core tests/test_core.cpp)
target_link_libraries(test_core PRIVATE pokecore)
enable_testing()
add_test(NAME core COMMAND test_core)

# ---- Module 3: SFML 2.6 game ----
find_package(SFML 2.5 COMPONENTS graphics window system)
if(SFML_FOUND)
  add_executable(pokebattle src/main.cpp src/gfx/BattleScene.cpp)
  target_link_libraries(pokebattle PRIVATE pokecore sfml-graphics sfml-window sfml-system)
  add_custom_command(TARGET pokebattle POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:pokebattle>/assets)
else()
  message(WARNING "SFML 2.5/2.6 not found - only the engine library and tests will be built.")
endif()
