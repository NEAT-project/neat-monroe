cmake_minimum_required(VERSION 2.6)
project(neat-metadata-exporter)

set(APP_VERSION "1.0.2")

add_definitions("-std=c++11 -Wall")
add_definitions("-DAPP_VERSION=\"${APP_VERSION}\"")

set(SRC_LIST main.cpp mqdaemon.cpp mqloop.cpp neat_writer.cpp)
set(LIB_LIST zmq jsoncpp)
add_executable(neat-metadata-exporter ${SRC_LIST})
target_link_libraries(neat-metadata-exporter ${LIB_LIST})

install(TARGETS neat-metadata-exporter RUNTIME DESTINATION /usr/sbin)
install(FILES ${PROJECT_SOURCE_DIR}/../files/systemd/neat-metadata-exporter.service DESTINATION
            /lib/systemd/system/ PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
            RENAME neat-metadata-exporter.service)

set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${APP_VERSION})
set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Tomasz Rozensztrauch <t.rozensztrauch@radytek.com>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CPACK_PROJECT_SOURCE_DIR}/../files/systemd/postinst;${CPACK_PROJECT_SOURCE_DIR}/../files/systemd/prerm")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack)

