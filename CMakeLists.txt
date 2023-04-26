cmake_minimum_required(VERSION 3.5)

project(qtchat VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTING OFF) # crypto++, causes issues with Ninja, not required for QtChat

set(DISABLE_SSSE3 ON CACHE BOOL "Disable SSSE3 intrinsic instructions") # crypto++

include_directories(lib/cryptopp)
add_subdirectory(lib/cryptopp)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network REQUIRED)

set(PROJECT_SOURCES
        src/main.cpp
        src/mainwindow.cpp
        include/mainwindow.h
        src/mainwindow.ui
        include/network.h
        src/network.cpp
        include/messaging.h
        src/messaging.cpp
        include/encryption.h
        src/encryption.cpp
        include/configuration.h
        src/configuration.cpp
        include/session.h
        src/session.cpp
        include/utils.h
        src/utils.cpp
        include/chatwindow.h
        src/chatwindow.cpp
        src/chatwindow.ui
        include/connectiondialog.h
        src/connectiondialog.cpp
        src/connectiondialog.ui
        include/settingswindow.h
        src/settingswindow.cpp
        src/settingswindow.ui
        include/chatmessagehistory.h
        src/chatmessagehistory.ui
        src/chatmessagehistory.cpp
        include/chatmessage.h
        src/chatmessage.ui
        src/chatmessage.cpp
        include/chatmessageeditdialog.h
        src/chatmessageeditdialog.cpp
        src/chatmessageeditdialog.ui
    )

qt_add_executable(qtchat
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_include_directories(qtchat PUBLIC include/)
target_link_libraries(qtchat PRIVATE Qt6::Widgets Qt6::Network cryptopp-static)
set_target_properties(qtchat PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER mff.cuni.cz
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)


