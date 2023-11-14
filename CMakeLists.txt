cmake_minimum_required(VERSION 3.10)

project(GAME)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)



file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
      ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(TBB REQUIRED)
target_link_libraries(${PROJECT_NAME} TBB::tbb)

target_link_libraries(${PROJECT_NAME}
  ${SDL2_LIBRARIES}
  SDL2_image
  SDL2_mixer
  box2d
)
