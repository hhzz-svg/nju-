cmake_minimum_required(VERSION 3.16)
project(Synera_Starter VERSION 1.0)

# CXX configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QT configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# QT6 dependency
find_package(Qt6 COMPONENTS Core Widgets Gui REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hxx"
)

add_executable(Synera_Starter ${SOURCES} ${HEADERS})

target_include_directories(Synera_Starter PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Synera_Starter PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

if(WIN32)
    set_target_properties(Synera_Starter PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

if(UNIX)
    target_link_libraries(Synera_Starter PRIVATE pthread)
endif()

if(APPLE)
    set_target_properties(Synera_Starter PROPERTIES MACOSX_BUNDLE TRUE)
endif()

enable_testing()

add_executable(Synera_Starter_CombatTests
    tests/combat_tests.cpp
    src/core/board.cpp
    src/core/combat.cpp
    src/entity/unit.cpp
)

target_include_directories(Synera_Starter_CombatTests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Synera_Starter_CombatTests PRIVATE
    Qt6::Core
)

add_test(NAME combat_tests COMMAND Synera_Starter_CombatTests)

add_executable(Synera_Starter_Week4Tests
    tests/week4_tests.cpp
    src/core/board.cpp
    src/core/combat.cpp
    src/core/game.cpp
    src/core/player.cpp
    src/core/shop.cpp
    src/core/savemanager.cpp
    src/core/synergy.cpp
    src/entity/unit.cpp
    src/gui/griditem.cpp
    src/gui/unititem.cpp
)

target_include_directories(Synera_Starter_Week4Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(Synera_Starter_Week4Tests PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
)

add_test(NAME week4_tests COMMAND Synera_Starter_Week4Tests)
