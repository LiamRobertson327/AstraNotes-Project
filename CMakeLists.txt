cmake_minimum_required(VERSION 3.19)
project(AstraNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Option to control whether OpenSSL modules (ossl-modules) are copied to the
# executable folder on Windows and test folders on Linux. Default ON to preserve behavior,
# but packagers and CI can disable with -DDEPLOY_OPENSSL_MODULES=OFF.
option(DEPLOY_OPENSSL_MODULES "Copy OpenSSL ossl-modules to build/test dirs" ON)

# 1. Setup the Qt Path
# Prefer an externally-provided Qt6_DIR or CMAKE_PREFIX_PATH; fall back to find_package
if(NOT DEFINED Qt6_DIR AND DEFINED ENV{QT_ROOT})
    set(Qt6_DIR $ENV{QT_ROOT})
endif()
find_package(Qt6 6.4 REQUIRED COMPONENTS Core Widgets Sql Test)
if(DEFINED Qt6_DIR)
    message(STATUS "Using Qt6 at: ${Qt6_DIR}")
endif()

# 2. Find OpenSSL
# Prefer user-provided OPENSSL_ROOT_DIR (or environment). Add common macOS Homebrew hints.
if(NOT DEFINED OPENSSL_ROOT_DIR)
    if(DEFINED ENV{OPENSSL_ROOT_DIR})
        set(OPENSSL_ROOT_DIR $ENV{OPENSSL_ROOT_DIR})
    elseif(APPLE)
        if(EXISTS "/opt/homebrew/opt/openssl@3")
            set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3")
        elseif(EXISTS "/usr/local/opt/openssl@3")
            set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl@3")
        endif()
    elseif(WIN32)
        if(MSVC)
            # Prefer native Windows OpenSSL layouts when building with MSVC.
            if(EXISTS "C:/Program Files/OpenSSL-Win64")
                set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL-Win64")
            elseif(EXISTS "C:/Program Files/OpenSSL")
                set(OPENSSL_ROOT_DIR "C:/Program Files/OpenSSL")
            endif()
        else()
            # Fall back to MSYS2/UCRT64 only when building with MinGW/GCC on Windows.
            if(EXISTS "C:/msys64/ucrt64")
                set(OPENSSL_ROOT_DIR "C:/msys64/ucrt64")
            endif()
        endif()
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

# Include UI sources in AstraCore so test executables can link UI classes like MainWindow
file(GLOB_RECURSE ASTRA_APP_UI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/ui/*.cpp"
)

# Build core library from shared sources so tests can link against implementation symbols.
add_library(AstraCore STATIC ${ASTRA_CORE_SOURCES})
target_include_directories(AstraCore PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
if(DEFINED OPENSSL_INCLUDE_DIR)
    target_include_directories(AstraCore PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()
target_link_libraries(AstraCore PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Widgets
    Qt6::Gui
)
if(OpenSSL_FOUND)
    target_link_libraries(AstraCore PRIVATE OpenSSL::Crypto)
endif()

# Build UI as a separate static library so tests can link MainWindow without
# duplicating MOC-generated symbols in AstraCore and the application target.
add_library(AstraAppUI STATIC ${ASTRA_APP_UI_SOURCES})
target_include_directories(AstraAppUI PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(AstraAppUI PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core Qt6::Sql AstraCore)

# Check if we are running in GitHub Actions (where we use MSVC which handles spaces fine)
if(DEFINED ENV{GITHUB_ACTIONS})
    set(RESOURCE_FILES resources.rc)
else()
    # Locally (MinGW), we skip the .rc file to avoid the "Space" crash
    set(RESOURCE_FILES "")
endif()

set(EXECUTABLE_OPTIONS)
if(WIN32)
    list(APPEND EXECUTABLE_OPTIONS WIN32)
endif()
if(APPLE)
    list(APPEND EXECUTABLE_OPTIONS MACOSX_BUNDLE)
endif()

qt_add_executable(AstraNotes
    ${EXECUTABLE_OPTIONS}
    src/app/main.cpp

    ${RESOURCE_FILES}
)
target_include_directories(AstraNotes PRIVATE
    src
)
if(DEFINED OPENSSL_INCLUDE_DIR)
    target_include_directories(AstraNotes PRIVATE ${OPENSSL_INCLUDE_DIR})
endif()

# 4. Tell CMake which folders to use
set_target_properties(AstraNotes PROPERTIES AUTOUIC_SEARCH_PATHS "src/ui")

# 5. Link libraries
target_link_libraries(AstraNotes PRIVATE
    Qt6::Widgets
    Qt6::Core
    Qt6::Sql
    AstraCore
)

if(OpenSSL_FOUND)
    target_link_libraries(AstraNotes PRIVATE OpenSSL::Crypto)
endif()

# Link the UI library into the application
target_link_libraries(AstraNotes PRIVATE AstraAppUI)


if(COMMAND qt_generate_deploy_app_script)
    # qt_generate_deploy_app_script changed signatures across Qt 6.x releases.
    # Newer Qt (>= 6.5) supports OUTPUT_SCRIPT; older Qt (6.2/6.4) use FILENAME.
    if(DEFINED Qt6Core_VERSION AND Qt6Core_VERSION VERSION_GREATER_EQUAL "6.5.0")
        qt_generate_deploy_app_script(
            TARGET AstraNotes
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
    else()
        # Fallback for older Qt 6.x where the argument is named FILENAME
        qt_generate_deploy_app_script(
            TARGET AstraNotes
            FILENAME deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
        )
    endif()

    install(SCRIPT ${deploy_script})
else()
    message(STATUS "qt_generate_deploy_app_script not available; skipping deploy script generation on this platform.")
endif()

enable_testing()

# Include project tests (smoke / unit tests)
add_subdirectory(tests)


# ============================================================================
# --- FIXED CRITICAL CROSS-PLATFORM OPENSSL MODULE HANDLING ---
# ============================================================================
if(OpenSSL_FOUND AND DEPLOY_OPENSSL_MODULES)
    if(WIN32)
        # Clean the list: Get the first actual path from the OpenSSL_CRYPTO_LIBRARIES list
        foreach(lib ${OPENSSL_CRYPTO_LIBRARIES})
            if(EXISTS "${lib}")
                set(REAL_CRYPTO_LIB "${lib}")
                break()
            endif()
        endforeach()

        get_filename_component(LIB_DIR "${REAL_CRYPTO_LIB}" DIRECTORY)
        
        # Check common locations for OpenSSL 3.x on Windows
        find_path(OSSL_MODULES_DIR_PATH
            NAMES legacy.dll default.dll
            PATHS 
                "${LIB_DIR}/ossl-modules"
                "${LIB_DIR}/../ossl-modules"
                "${LIB_DIR}/../../ossl-modules"
                "C:/Program Files/OpenSSL/lib/ossl-modules"
                "C:/Program Files/OpenSSL-Win64/lib/ossl-modules"
            NO_DEFAULT_PATH
        )

        if(OSSL_MODULES_DIR_PATH)
            message(STATUS "Found Windows OpenSSL modules at: ${OSSL_MODULES_DIR_PATH}")
            
            add_custom_command(TARGET AstraNotes POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${OSSL_MODULES_DIR_PATH}" "$<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
                COMMENT "Deploying OpenSSL modules to: $<TARGET_FILE_DIR:AstraNotes>/ossl-modules"
            )
        else()
            message(WARNING "Could not find ossl-modules directory for Windows deployment!")
        endif()

    elseif(UNIX AND NOT APPLE)
        # Standard Ubuntu runner layouts for OpenSSL 3 modules
        set(LINUX_PRIMARY_MODULE_PATH "/usr/lib/x86_64-linux-gnu/ossl-modules")
        set(LINUX_ALT_MODULE_PATH "/usr/lib/ossl-modules")
        
        if(EXISTS "${LINUX_PRIMARY_MODULE_PATH}")
            set(LINUX_MODULES_DIR "${LINUX_PRIMARY_MODULE_PATH}")
        elseif(EXISTS "${LINUX_ALT_MODULE_PATH}")
            set(LINUX_MODULES_DIR "${LINUX_ALT_MODULE_PATH}")
        endif()

        if(LINUX_MODULES_DIR)
            message(STATUS "Found Linux OpenSSL modules at: ${LINUX_MODULES_DIR}")
            
            # This automatically populates 'build/tests/ossl-modules' during compilation
            # allowing CTest / SmokeTest to bypass the Argon2id loading failure natively.
            add_custom_target(DeployLinuxTestModules ALL
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/tests/ossl-modules"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${LINUX_MODULES_DIR}" "${CMAKE_BINARY_DIR}/tests/ossl-modules"
                COMMENT "Staging Linux OpenSSL modules into test directory structure"
            )
        else()
            message(WARNING "Could not locate standard system ossl-modules folder on Linux!")
        endif()
    endif()
endif()