cmake_minimum_required(VERSION 3.16)
project(TextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(APP_REPO_URL "https://github.com/kawsaramin101/text-editor")

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TextEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_compile_definitions(TextEditor PRIVATE APP_REPO_URL="${APP_REPO_URL}")


qt_add_resources(TextEditor "icons"
    PREFIX "/"
    FILES texteditor.svg
)

target_link_libraries(TextEditor PRIVATE Qt6::Widgets)

set_target_properties(TextEditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)

# ── Install + CPack / .deb ───────────────────────────────────────────────────
install(TARGETS TextEditor DESTINATION bin)
install(FILES texteditor.desktop DESTINATION share/applications)
install(FILES texteditor.svg    DESTINATION share/icons/hicolor/256x256/apps)

execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
string(REGEX REPLACE "^v" "" APP_VERSION "${GIT_TAG}")
if(NOT APP_VERSION)
    set(APP_VERSION "1.3.0")
endif()

set(CPACK_GENERATOR                     "DEB")
set(CPACK_PACKAGE_NAME                  "texteditor")
set(CPACK_PACKAGE_VERSION               "${APP_VERSION}")
set(CPACK_PACKAGE_CONTACT               "https://github.com/kawsaramin101/text-editor")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY   "Simple text editor")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/kawsaramin101/text-editor")
set(CPACK_DEBIAN_PACKAGE_DEPENDS        "libqt6widgets6 (>= 6.0), libqt6gui6 (>= 6.0), libqt6core6 (>= 6.0)")
set(CPACK_DEBIAN_PACKAGE_SECTION        "editors")
set(CPACK_PACKAGING_INSTALL_PREFIX      "/usr")
include(CPack)
