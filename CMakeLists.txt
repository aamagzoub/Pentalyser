# Minimum CMake version required
cmake_minimum_required(VERSION 3.16)

# Ensure Universal Binary (x86_64 for Intel, arm64 for Apple Silicon)
set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64")

# Set macOS Deployment Target (to support older macOS versions)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")  # Change as needed

# Project name and settings (MUST come AFTER architecture setting)
project(Pentalyser VERSION 0.1 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "/Users/emagabu/Qt/6.8.1/macos/lib/cmake")

# Enable Qt-specific automatic processing
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt (either Qt6 or Qt5)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Source files
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# Define executable with proper icon setup
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Pentalyser
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(Pentalyser SHARED ${PROJECT_SOURCES})
    else()
        add_executable(Pentalyser ${PROJECT_SOURCES})
    endif()
endif()

# Link against Qt libraries
target_link_libraries(Pentalyser PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

# macOS-specific settings: bundle and icon
set(APP_ICON "${CMAKE_SOURCE_DIR}/MyApp.icns")

# Check if the icon file exists
if(NOT EXISTS "${APP_ICON}")
    message(FATAL_ERROR "Icon file '${APP_ICON}' not found!")
endif()

# Set macOS bundle properties
set_target_properties(Pentalyser PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Pentalyser"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.Pentalyser"
    MACOSX_BUNDLE_ICON_FILE "MyApp.icns"
    WIN32_EXECUTABLE TRUE
)

# Copy the icon to the correct bundle resources directory after build
add_custom_command(TARGET Pentalyser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            "${APP_ICON}"
            "$<TARGET_BUNDLE_DIR:Pentalyser>/Contents/Resources/MyApp.icns"
)

# Install instructions
include(GNUInstallDirs)
install(TARGETS Pentalyser
    BUNDLE DESTINATION /Applications
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize executable for Qt 6
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Pentalyser)
endif()
