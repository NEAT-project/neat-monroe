cmake_minimum_required(VERSION 2.6)
project(neat-http-get)

set(SRC_LIST neat_http_get.c util.c)
set(LIB_LIST neat uv)
add_executable(neat_http_get ${SRC_LIST})
target_link_libraries(neat_http_get ${LIB_LIST})

install(TARGETS neat_http_get RUNTIME DESTINATION /usr/bin)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)

