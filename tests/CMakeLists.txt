cmake_minimum_required(VERSION 3.19)

# Smoke tests for AstraNotes
project(AstraNotesTests)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/smoke/*.cpp"
)

add_executable(smoke_tests ${TEST_SOURCES})

target_include_directories(smoke_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(smoke_tests PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Sql
    OpenSSL::Crypto
    AstraCore
)

add_test(NAME smoke_tests COMMAND smoke_tests)
