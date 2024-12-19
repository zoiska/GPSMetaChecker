cmake_minimum_required(VERSION 3.28)
project(GPSMetaChecker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.6.1/mingw_64")
#set(exiv2_DIR "C:/exiv2extract/exiv2-0.28.3-2019msvc64/lib/cmake/exiv2")
#set(EXPAT_INCLUDE_DIR "C:/user/zoisk/.vcpkg-clion")

#include_directories("C:/exiv2extract/exiv2-0.28.3-2019msvc64/include")


#find_package(exiv2 CONFIG REQUIRED)
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

#link_directories("C:/exiv2extract/exiv2-0.28.3-2019msvc64/lib")

add_executable(GPSMetaChecker main.cpp
        TinyEXIF.h
        TinyEXIF.cpp
        tinyxml2.cpp
        tinyxml2.h
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

target_link_libraries(GPSMetaChecker PRIVATE
        Qt::Core
        Qt::Gui
        Qt::Widgets
#        C:/exiv2extract/exiv2-0.28.3-2019msvc64/lib/exiv2.lib
)


if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEBUG_SUFFIX)
    if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
        set(DEBUG_SUFFIX "d")
    endif ()
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt6${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
