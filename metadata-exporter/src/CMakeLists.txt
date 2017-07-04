cmake_minimum_required(VERSION 2.6)
project(neat-metadata-exporter)

add_definitions("-std=c++11 -Wall")

set(SRC_LIST main.cpp mqdaemon.cpp mqloop.cpp neat_writer.cpp)
set(LIB_LIST zmq jsoncpp)
add_executable(neat-metadata-exporter ${SRC_LIST})
target_link_libraries(neat-metadata-exporter ${LIB_LIST})

add_executable(clock-info clock_info.cpp)

#install(TARGETS nne-lxc-network-manager RUNTIME DESTINATION /usr/sbin)
#install(FILES ${PROJECT_SOURCE_DIR}/../files/systemd/nne-lxc-network-manager.service DESTINATION
#            /lib/systemd/system/ PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
#            RENAME nne-lxc-network-manager.service)

#set(CPACK_GENERATOR "DEB")
#set(CPACK_PROJECT_SOURCE_DIR "${PROJECT_SOURCE_DIR}")
#set(CPACK_PROJECT_CONFIG_FILE ${PROJECT_SOURCE_DIR}/DebConf.cmake)
#include(CPack)

