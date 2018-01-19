cmake_minimum_required(VERSION 3.8)
project(ANNESE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS_RELEASE -O2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMY_FILENAME='\"$(subst ${CMAKE_SOURCE_DIR}/,,$(abspath $<))\"'")

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D DEBUG=1")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D DEBUG=0")
endif()

set(SOURCE_FILES src/main.cpp
        include/CPUOpcodes.h
        src/CPU.cpp include/CPU.h
        src/MainBus.cpp include/MainBus.h
        src/PPU.cpp include/PPU.h include/Utility.h
        src/Mapper.cpp include/Mapper.h
        src/Cartridge.cpp include/Cartridge.h
        src/CartridgeLoader.cpp include/CartridgeLoader.h
        include/TeeLog.hpp
        src/MapperNROM.cpp include/MapperNROM.h src/Screen.cpp include/Screen.h src/PictureBus.cpp include/PictureBus.h include/PaletteColors.h src/Emulator.cpp include/Emulator.h src/Joypad.cpp include/Joypad.h src/ConfigManager.cpp include/ConfigManager.h src/MapperSxROM.cpp include/MapperSxROM.h src/MapperCNROM.cpp include/MapperCNROM.h src/MapperUxROM.cpp include/MapperUxROM.h)

add_executable(ANNESE ${SOURCE_FILES})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")
find_package(SFML REQUIRED system window graphics audio)
if (SFML_FOUND)
    include_directories(${SFML_INCLUDE_DIR})
    target_link_libraries(ANNESE ${SFML_LIBRARIES})
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR}/lib/cpptoml/include)