cmake_minimum_required(VERSION 3.5)
project(zfp_data_gen)

find_package(zfp REQUIRED)

add_executable(zfp_make_test_data 
    zfp_make_test_data.cpp)

set_target_properties(zfp_make_test_data PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON)

target_link_libraries(zfp_make_test_data PUBLIC
    zfp::zfp
    glm::glm)

