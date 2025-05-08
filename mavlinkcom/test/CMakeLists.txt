cmake_minimum_required(VERSION 3.2.2)
project(MavLinkTest)

include_directories(
  ../include
  ../common_utils
)

set(PROJECT_CPP ${PROJECT_NAME}_sources)
file(GLOB_RECURSE PROJECT_CPP "./*.cpp")
add_executable(${PROJECT_NAME} ${PROJECT_CPP})


target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
        ${JSON_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} mavlinkcom)
