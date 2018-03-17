cmake_minimum_required(VERSION 2.6)
project(dwnl-test)

set(SRC_LIST dwnl-test.cpp arg_parser.cpp)
add_executable(dwnl-test ${SRC_LIST})


set(SRC_LIST_NEAT neat-dwnl-test.cpp arg_parser.cpp)
set(LIB_LIST_NEAT neat)
add_executable(neat-dwnl-test ${SRC_LIST_NEAT})
target_link_libraries(neat-dwnl-test ${LIB_LIST_NEAT})

install(TARGETS dwnl-test RUNTIME DESTINATION /usr/local/bin)
install(TARGETS neat-dwnl-test RUNTIME DESTINATION /usr/local/bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.3")
set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
#set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)

