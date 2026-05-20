cmake_minimum_required(VERSION 3.19)
project(AstraNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/std:c++23)
endif()

# 1. Setup the Qt Path
set(Qt6_DIR "C:/Qt/6.11.0/mingw_64/lib/cmake/Qt6")
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql Test)

# 2. Find OpenSSL
if(NOT DEFINED ENV{GITHUB_ACTIONS})
    if(EXISTS "C:/msys64/ucrt64")
        set(OPENSSL_ROOT_DIR "C:/msys64/ucrt64")
    endif()
endif()

find_package(OpenSSL REQUIRED)

# 2. Modern Qt Setup
qt_standard_project_setup()

# 3. Split shared core code from app UI code
file(GLOB_RECURSE ASTRA_CORE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/model/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/repository/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/service/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/crypto/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/plugins/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/logging/*.cpp"
)

file(GLOB_RECURSE ASTRA_APP_UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/ui/*.cpp"
)

# Build core library from shared sources so tests can link against implementation symbols.
add_library(AstraCore STATIC ${ASTRA_CORE_SOURCES})
target_include_directories(AstraCore PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${OPENSSL_INCLUDE_DIR}
)
target_link_libraries(AstraCore PRIVATE
    OpenSSL::Crypto
    Qt6::Core
    Qt6::Sql
    Qt6::Widgets
    Qt6::Gui
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
    src/app/main.cpp
    ${ASTRA_APP_UI_SOURCES}
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
    AstraCore
    OpenSSL::Crypto
)


qt_generate_deploy_app_script(
    TARGET AstraNotes
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})

enable_testing()

# Include project tests (smoke / unit tests)
add_subdirectory(tests)

# --- OpenSSL Module Deployment Fix ---
# This detects your MSYS2 OpenSSL modules and copies them to your build directory 
# so the "Brain" can find its "Tools" even after a restart.
# --- Portable OpenSSL Module Handling ---
if(OpenSSL_FOUND AND WIN32)
    # 1. Clean the list: Get the first actual path from the OpenSSL_CRYPTO_LIBRARIES list
    # This removes the "optimized" and "debug" keywords
    foreach(lib ${OPENSSL_CRYPTO_LIBRARIES})
        if(EXISTS "${lib}")
            set(REAL_CRYPTO_LIB "${lib}")
            break()
        endif()
    endforeach()

    # 2. Derive the directory. 
    # Usually: C:/Program Files/OpenSSL/lib/VC/x64/MD/libcrypto.lib
    get_filename_component(LIB_DIR "${REAL_CRYPTO_LIB}" DIRECTORY)
    
    # 3. Find the modules folder by looking relative to the lib path
    # We check common locations for OpenSSL 3.x on Windows
    find_path(OSSL_MODULES_DIR_PATH
        NAMES legacy.dll default.dll
        PATHS 
            "${LIB_DIR}/ossl-modules"
            "${LIB_DIR}/../ossl-modules"
            "${LIB_DIR}/../../ossl-modules"
            "C:/Program Files/OpenSSL/lib/ossl-modules"
        NO_DEFAULT_PATH
    )

    if(OSSL_MODULES_DIR_PATH)
        message(STATUS "Found OpenSSL modules at: ${OSSL_MODULES_DIR_PATH}")
        
        add_custom_command(TARGET AstraNotes POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${OSSL_MODULES_DIR_PATH}" "$<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
            COMMENT "Deploying OpenSSL modules to: $<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
        )
    else()
        message(WARNING "Could not find ossl-modules directory for deployment!")
    endif()
endif()