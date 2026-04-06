cmake_minimum_required(VERSION 3.19)
project(AstraNotes VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 1. Setup the Qt Path
set(Qt6_DIR "C:/Qt/6.11.0/mingw_64/lib/cmake/Qt6")
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

# 2. Modern Qt Setup
qt_standard_project_setup()

# 3. Create the executable
qt_add_executable(AstraNotes
    WIN32 MACOSX_BUNDLE
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    src/main.cpp 
    src/Note.cpp
)

# 4. Tell CMake which folders to use
target_include_directories(AstraNotes PUBLIC include src)

# 5. Link libraries
target_link_libraries(AstraNotes PRIVATE Qt6::Widgets Qt6::Core)

qt_generate_deploy_app_script(
    TARGET AstraNotes
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})



# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTORCC ON)

# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(Qt6_DIR "C:/Qt/6.11.0/mingw_64/lib/cmake/Qt6")
# find_package(Qt6 REQUIRED COMPONENTS Core Widgets)


# add_executable(AstraNotes 
#     src/main.cpp 
#     src/Note.cpp
# )

# target_include_directories(AstraNotes PUBLIC include)
# target_link_libraries(AstraNotes PRIVATE Qt6::Widgets Qt6::Core)