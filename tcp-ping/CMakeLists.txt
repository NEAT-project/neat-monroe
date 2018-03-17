cmake_minimum_required(VERSION 3.5)
project(tcp-ping)

set(APP_VERSION "1.0.4")

set(SRC_LIST tcp-ping.cpp arg_parser.cpp)
add_executable(tcp-ping ${SRC_LIST})
target_compile_definitions(tcp-ping PRIVATE APP_NAME="tcp-ping" APP_VERSION="${APP_VERSION}")

set(SRC_LIST_NEAT neat-tcp-ping.cpp arg_parser.cpp)
set(LIB_LIST_NEAT neat)
add_executable(neat-tcp-ping ${SRC_LIST_NEAT})
target_compile_definitions(neat-tcp-ping PRIVATE APP_NAME="neat-tcp-ping" APP_VERSION="${APP_VERSION}")
target_link_libraries(neat-tcp-ping ${LIB_LIST_NEAT})

install(TARGETS tcp-ping RUNTIME DESTINATION /usr/local/bin)
install(TARGETS neat-tcp-ping RUNTIME DESTINATION /usr/local/bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.3")
set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
#set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)

