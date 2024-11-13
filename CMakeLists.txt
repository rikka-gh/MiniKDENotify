cmake_minimum_required(VERSION 3.29)

set(MINIKDENOTIFY_VERSION_MAJOR 0)
set(MINIKDENOTIFY_VERSION_MINOR 1)
set(MINIKDENOTIFY_VERSION_PATCH 0)
set(MINIKDENOTIFY_VERSION "${MINIKDENOTIFY_VERSION_MAJOR}.${MINIKDENOTIFY_VERSION_MINOR}.${MINIKDENOTIFY_VERSION_PATCH}")

project(MiniKDENotify
        VERSION ${MINIKDENOTIFY_VERSION}
        DESCRIPTION "Minimal KDE Notification Library"
        HOMEPAGE_URL "https://github.com/rikka-gh/MiniKDENotify"
        LANGUAGES CXX
)

option(MINIKDENOTIFY_BUILD_EXAMPLES "Build example programs" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)

configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in
        ${CMAKE_CURRENT_SOURCE_DIR}/include/MiniKDENotify/version.hpp
        @ONLY
)


add_library(MiniKDENotify INTERFACE)
add_library(MiniKDENotify::MiniKDENotify ALIAS MiniKDENotify)


target_include_directories(MiniKDENotify
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        ${DBUS_INCLUDE_DIRS}
)


target_link_libraries(MiniKDENotify
        INTERFACE
        ${DBUS_LIBRARIES}
)

install(DIRECTORY include/MiniKDENotify
        DESTINATION include
        FILES_MATCHING PATTERN "*.hpp"
)

install(TARGETS MiniKDENotify
        EXPORT MiniKDENotifyTargets
        INCLUDES DESTINATION include
)

install(EXPORT MiniKDENotifyTargets
        FILE MiniKDENotifyTargets.cmake
        NAMESPACE MiniKDENotify::
        DESTINATION lib/cmake/MiniKDENotify
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MiniKDENotifyConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/MiniKDENotifyConfig.cmake
        INSTALL_DESTINATION lib/cmake/MiniKDENotify
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/MiniKDENotifyConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/MiniKDENotifyConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/MiniKDENotifyConfigVersion.cmake
        DESTINATION lib/cmake/MiniKDENotify
)

if (MINIKDENOTIFY_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif ()

export(EXPORT MiniKDENotifyTargets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/MiniKDENotifyTargets.cmake
        NAMESPACE MiniKDENotify::
)