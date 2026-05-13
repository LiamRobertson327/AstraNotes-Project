cmake_minimum_required(VERSION 3.19)
project(AstraNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Setup the Qt Path
set(Qt6_DIR "C:/Qt/6.11.0/mingw_64/lib/cmake/Qt6")
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

# 2. Find OpenSSL
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    if(EXISTS "C:/msys64/ucrt64")
        set(OPENSSL_ROOT_DIR "C:/msys64/ucrt64")
    endif()
endif()

find_package(OpenSSL REQUIRED)

# 2. Modern Qt Setup
qt_standard_project_setup()

# 3. Gather sources and create the executable
file(GLOB_RECURSE ASTRA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.c"
)

# Check if we are running in GitHub Actions (where we use MSVC which handles spaces fine)
if(DEFINED ENV{GITHUB_ACTIONS})
    set(RESOURCE_FILES resources.rc)
else()
    # Locally (MinGW), we skip the .rc file to avoid the "Space" crash
    set(RESOURCE_FILES "")
endif()

qt_add_executable(AstraNotes
    WIN32 MACOSX_BUNDLE
    ${ASTRA_SOURCES}
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    ${RESOURCE_FILES}
)
target_include_directories(AstraNotes PRIVATE
    src 
    ${OPENSSL_INCLUDE_DIR}
    )

# 4. Tell CMake which folders to use
set_target_properties(AstraNotes PROPERTIES AUTOUIC_SEARCH_PATHS "src/ui")

# 5. Link libraries
target_link_libraries(AstraNotes PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Sql
    OpenSSL::Crypto
)


qt_generate_deploy_app_script(
    TARGET AstraNotes
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

# --- OpenSSL Module Deployment Fix ---
# This detects your MSYS2 OpenSSL modules and copies them to your build directory 
# so the "Brain" can find its "Tools" even after a restart.
# --- Portable OpenSSL Module Handling ---
if(WIN32 AND OPENSSL_FOUND)
    # Use the 'OPENSSL_PROGRAM' or 'OPENSSL_INCLUDE_DIR' to find the library root 
    # instead of hardcoding C:/msys64
    get_filename_component(OPENSSL_BIN_DIR "${OPENSSL_CRYPTO_LIBRARY}" DIRECTORY)
    
    # Usually modules are in ../lib/ossl-modules relative to the bin
    set(OSSL_SRC "${OPENSSL_BIN_DIR}/../lib/ossl-modules")
    set(OSSL_DEST "${CMAKE_BINARY_DIR}/ossl-modules")

    add_custom_command(TARGET AstraNotes POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${OSSL_DEST}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${OSSL_SRC}" "${OSSL_DEST}"
        COMMENT "Deploying OpenSSL modules for portable encryption"
    )
endif()