cmake_minimum_required(VERSION 3.19)

# Smoke tests for AstraNotes
project(AstraNotesTests)

file(GLOB SMOKE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/smoke/*.cpp")
foreach(smoke_src IN LISTS SMOKE_SOURCES)
    get_filename_component(smoke_name ${smoke_src} NAME_WE)
    add_executable(${smoke_name} ${smoke_src})
    target_include_directories(${smoke_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${smoke_name} PRIVATE Qt6::Test Qt6::Core Qt6::Sql OpenSSL::Crypto AstraCore)
    add_test(NAME ${smoke_name} COMMAND ${smoke_name})
endforeach()

# Build each unit test as a separate executable to avoid multiple main() definitions
file(GLOB UNIT_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/unit/*.cpp")
foreach(unit_src IN LISTS UNIT_SOURCES)
    get_filename_component(unit_name ${unit_src} NAME_WE)
    add_executable(${unit_name} ${unit_src})
    target_include_directories(${unit_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${unit_name} PRIVATE Qt6::Test Qt6::Core Qt6::Sql OpenSSL::Crypto AstraCore)
    add_test(NAME ${unit_name} COMMAND ${unit_name})
endforeach()

# Integration tests that exercise multiple layers together.
file(GLOB INTEGRATION_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/integration/*.cpp")
foreach(integration_src IN LISTS INTEGRATION_SOURCES)
    get_filename_component(integration_name ${integration_src} NAME_WE)
    add_executable(${integration_name} ${integration_src})
    target_include_directories(${integration_name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${integration_name} PRIVATE Qt6::Test Qt6::Core Qt6::Sql OpenSSL::Crypto AstraCore)
    add_test(NAME ${integration_name} COMMAND ${integration_name})
endforeach()
