cmake_minimum_required(VERSION 3.10)
project(WFCGenerator)

# ---- Standard ----
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Build type ----
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ---- OpenCV ----
find_package(OpenCV REQUIRED)

# ---- OpenMP ----
find_package(OpenMP REQUIRED)

# ---- Executable ----
add_executable(WFCGenerator
    main.cpp
    tile.cpp
    tileset.cpp
    tilesetconfig.cpp
    wfc_generator.cpp
    grid.cpp
    cell.cpp
    sides.cpp
    image_generator.cpp
    tileinfo.cpp
)

# ---- Includes ----
target_include_directories(WFCGenerator PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ---- Linking ----
target_link_libraries(WFCGenerator PRIVATE
    ${OpenCV_LIBS}
    OpenMP::OpenMP_CXX
)

# ---- Debug flags ----
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_options(WFCGenerator PRIVATE -fsanitize=address)
    target_link_options(WFCGenerator PRIVATE -fsanitize=address)
endif()