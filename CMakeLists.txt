cmake_minimum_required(VERSION 3.5)
project(ProjetoOpen)

set(OpenCV_DIR "C:/Users/pvc25/Downloads/opencv/build")

find_package(OpenCV REQUIRED)


if(OpenCV_FOUND)
    message(STATUS "OpenCV encontrado: ${OpenCV_VERSION}")

    add_executable(ProjetoOpen main.cpp)
    target_link_libraries(ProjetoOpen ${OpenCV_LIBS})
else()
    message(FATAL_ERROR "Não foi possível encontrar o OpenCV")
endif()