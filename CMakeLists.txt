cmake_minimum_required(VERSION 3.16.3)
project(csa_mdp_experiments)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++17")

# Directories containing sources files
set(DIRECTORIES
  src/kick_model
  src/learning_machine
  src/odometry
  src/policies
  src/problems
  )

# Build ALL_SOURCES
set (SOURCES)
set (ALL_SOURCES)
set (PREFIXED_SOURCES)
foreach (DIRECTORY ${DIRECTORIES})
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}")
    include (${DIRECTORY}/sources.cmake)

    foreach (SOURCE ${SOURCES})
        set (PREFIXED_SOURCES ${PREFIXED_SOURCES} ${DIRECTORY}/${SOURCE})
    endforeach (SOURCE)

    set (ALL_SOURCES ${ALL_SOURCES} ${PREFIXED_SOURCES})
endforeach (DIRECTORY)

# Declare a C++ library
add_library(csa_mdp_experiments ${ALL_SOURCES})
target_link_libraries(csa_mdp_experiments PUBLIC rhoban_csa_mdp)
target_include_directories(csa_mdp_experiments PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

## Common generic program

add_executable(learn_from_logs src/learn_from_logs.cpp)
target_link_libraries(learn_from_logs csa_mdp_experiments)

# Generic machine to execute experiment, parameters are given through xml file
add_executable(learning_machine src/learning_machine.cpp)
target_link_libraries(learning_machine csa_mdp_experiments)

add_executable(black_box_learning src/black_box_learning.cpp)
target_link_libraries(black_box_learning csa_mdp_experiments)

add_executable(extract_fa_tree src/extract_fa_tree.cpp)
target_link_libraries(extract_fa_tree csa_mdp_experiments)

add_executable(blackbox_value_estimator src/blackbox_value_estimator.cpp)
target_link_libraries(blackbox_value_estimator csa_mdp_experiments)

add_executable(robot_kick_comparator src/robot_kick_comparator.cpp)
target_link_libraries(robot_kick_comparator csa_mdp_experiments)

enable_testing()

set(TESTS
  problems/ssl_dynamic_ball_approach
  )

if (CATKIN_ENABLE_TESTING)
  # create an executable for each test
  foreach(test ${TESTS})
    string(REPLACE "/" "_" id ${test})
    catkin_add_gtest(${id} tests/${test}.cpp)
    target_link_libraries(${id} ${catkin_LIBRARIES} csa_mdp_experiments)
  endforeach(test)
endif()
