cmake_minimum_required(VERSION 3.16)
project(TextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TextEditor
    main.cpp
    mainwindow.cpp
    mainwindow.h
    resources.qrc
)

target_link_libraries(TextEditor PRIVATE Qt6::Widgets)

set_target_properties(TextEditor PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)


install(TARGETS TextEditor DESTINATION bin)

# CPack / .deb config
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "texteditor")
set(CPACK_PACKAGE_VERSION "1.1.0")
set(CPACK_PACKAGE_CONTACT "you@example.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A minimal Qt6 text editor with line numbers")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt6widgets6 (>= 6.0)")
set(CPACK_DEBIAN_PACKAGE_SECTION "editors")
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

install(FILES texteditor.desktop
    DESTINATION share/applications)

install(FILES texteditor.svg
    DESTINATION share/icons/hicolor/256x256/apps)

include(CPack)
