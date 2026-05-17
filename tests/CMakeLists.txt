cmake_minimum_required(VERSION 3.19)

# Smoke tests for AstraNotes
project(AstraNotesTests)

file(GLOB_RECURSE SMOKE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/smoke/*.cpp"
)

add_executable(smoke_tests ${SMOKE_SOURCES})
target_include_directories(smoke_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(smoke_tests PRIVATE Qt6::Test Qt6::Core Qt6::Sql OpenSSL::Crypto AstraCore)
add_test(NAME smoke_tests COMMAND smoke_tests)

# Build each unit test as a separate executable to avoid multiple main() definitions
file(GLOB UNIT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")
foreach(unit_src IN LISTS UNIT_SOURCES)
    get_filename_component(unit_name ${unit_src} NAME_WE)
    add_executable(${unit_name} ${unit_src})
    target_include_directories(${unit_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${unit_name} PRIVATE Qt6::Test Qt6::Core Qt6::Sql OpenSSL::Crypto AstraCore)
    add_test(NAME ${unit_name} COMMAND ${unit_name})
endforeach()
