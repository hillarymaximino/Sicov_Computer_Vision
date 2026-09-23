cmake_minimum_required(VERSION 3.10)
project(VisionTest VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Encontra o OpenCV com CUDA que acabámos de instalar
find_package(OpenCV REQUIRED)

include_directories(
    include
    ${OpenCV_INCLUDE_DIRS}
)

set(SOURCES
    src/main.cpp
    src/Bsp.cpp
    src/Orchestrator.cpp
    src/VisionModule.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Vincula o projeto ao OpenCV e à biblioteca de threads
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OpenCV_LIBS}
    pthread
)