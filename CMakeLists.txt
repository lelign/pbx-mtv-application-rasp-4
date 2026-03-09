cmake_minimum_required(VERSION 2.8.11)
project(PBXMTV508)
message(STATUS "ON START  CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)

message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

#IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
#        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O3")
##       SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3") 20_12 changed Ignat
#        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon ")
#        message(STATUS "Debug  CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
#        add_definitions(-DQT_NO_EXCEPTIONS)
#ENDIF(CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon ")

# 0 - PBX-MTV-508 CMAKE_CXX_FLAGS=-mfpu=neon -Wall -O3
# 1 - PN-MTV-581
set(BOARD_REV "0" CACHE STRING "Board rev")
add_definitions(-DBOARD_REV=${BOARD_REV})
message(STATUS "Board rev: ${BOARD_REV}")

#set(MY_VAR "8" CACHE STRING "my variable")
#message( "my variable: ${MY_VAR}")

#find_library(Qt5_Network NAMES Qt5Network PATHS /usr/lib/arm-linux-gnueabihf)
#if(NOT Qt5_Network)
#  message(ERROR "Qt5_Network has not been found")
#else()
#   message(STATUS "Qt5_Network found ")
#ENDIF(NOT Qt5_Network)

#find_library(LIB_usb NAMES libusb-1.0 PATHS /usr/lib/arm-linux-gnueabihf/libusb-1.0.so)
#if(NOT LIB_usb)
#  message(ERROR "LIB_usb has not been found")
#else()
#   message(STATUS "LIB_usb found")
#ENDIF(NOT LIB_usb)


#find_package(Qt5Network)
#message(STATUS "Qt5Network ${Qt5Network_CONFIG}")
find_package(Qt5WebSockets)
#message(STATUS "Qt5WebSockets ${Qt5WebSockets_CONFIG}")
#find_package(Qt5 COMPONENTS Widgets REQUIRED)
#message(STATUS "Widgets ${Widgets_CONFIG}")
find_package(Qt5SerialPort)
#message(STATUS "Qt5SerialPort ${Qt5SerialPort_CONFIG}")
#find_package(Threads)
#message(STATUS "Threads ${Threads_CONFIG}")
FIND_PACKAGE(libusb-1.0 REQUIRED)
#find_package(bitstream REQUIRED)
#message(  " # ")
#message(STATUS "bitstream ${bitstream_CONFIG}")
#find_package(Zvbi REQUIRED)

find_package(Qt5Svg REQUIRED)
#include_directories(${Qt5Svg_INCLUDE_DIRS})

message(STATUS "include_directories Qt5Svg  ${Qt5Svg_INCLUDE_DIRS}")

#include_directories(${Qt5Core_INCLUDE_DIRS})

message(STATUS "include_directories Qt5Core  ${Qt5Svg_INCLUDE_DIRS}")

#include_directories(${Qt5Network_INCLUDE_DIRS})
#include_directories(${Qt5WebSockets_INCLUDE_DIRS})
include_directories(${LIBUSB_1_INCLUDE_DIRS})
#include_directories(${LIBBITSTREAM_INCLUDE_DIR})
#message(STATUS "include_directories LIBBITSTREAM_INCLUDE_DIR  ${LIBBITSTREAM_INCLUDE_DIR}")
include_directories(${ZVBI_INCLUDE_DIRS})
#include_directories(tsl)
SET (CMAKE_POSITION_INDEPENDENT_CODE ON)
SET (CMAKE_AUTORCC ON)

add_subdirectory(zvbi-test)
#add_subdirectory(profnext)
add_subdirectory(tsl)

add_executable(pbx-mtv-508
        pbx-mtv-508-main.cpp
        pbx-mtv-508.cpp
        mtv-system/mtv-system.cpp
	hdmi_adv7513/hdmi_adv7513.cpp
	i2c/i2c.cpp	
	web_socket_server/websocketserver.cpp
	mtv-web/mtv_web.cpp
	hardware_diagnostics/hardware_diagnostics.cpp
        layout/layout.cpp
        cascade_server/cascade_server.cpp
        cascade_ctrl/cascade_ctrl.cpp
        m26/mb86m26.cpp
        m26/mb86m26_control.cpp
        m26/mb86m26_lib.cpp
        m26/mpeg-dump.cpp
        HlsServer/hlsserver.cpp
	led/led_class.cpp
	factory_defaults/factory_defaults.cpp
	ip-utils/ip-utils.c
        uptime-utils/uptime_utils.cpp
	dns-utils/dns-utils.c
        watchdog.cpp
	qpps/qpps.cpp
        time_counter/time_counter.cpp
	utils/utils.c
	ntpdate-utils/ntpdate-utils.c
	gpio/gpio.cpp
	eventlog-api/eventlog.cpp
        resource.qrc
        mtv-system/anc-reader.cpp
        ipout/ts-in-reader.cpp
        ipout/ts-reader.cpp
	file_handle_leaks/file_handle_leaks.cpp
	scte_104/scte_104.cpp
	snmp_agent/snmp_agent.cpp
        mtv-snmp/mtv_snmp.cpp
        teletext-decoder/teletext-decoder.cpp
        profitt-security/profitt-security.c
        profnext/5566.c
        profnext/profnext-frontpanel.cpp
        ctrl_panel_PERP4116/ctrl_panel_perp4116.cpp
        vgpi_server/vgpi_server.cpp
        vgpi_api/vgpi_api.cpp
        remote_ctrl_preset/remote_ctrl_preset.cpp
)
target_link_libraries(pbx-mtv-508 Qt5::Widgets Qt5::Core Qt5::WebSockets Qt5::Svg Qt5::SerialPort ${LIBUSB_1_LIBRARIES} ${ZVBI_LIBRARY} tsl-server)
#target_link_libraries(pbx-mtv-508 ${LIBUSB_1_LIBRARIES} ${ZVBI_LIBRARY} tsl-server)
#message(STATUS "CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}")
INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/pbx-mtv-508 DESTINATION bin)

#add_executable(m26-eeprom
#        m26/m26-eeprom.cpp
#)
#target_link_libraries(m26-eeprom ${LIBUSB_1_LIBRARIES})

#INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/m26-eeprom DESTINATION bin)

#add_executable(rgbyuv-test
#        rgbyuv-test/rgbyuv-test.cpp
#)
#target_link_libraries(rgbyuv-test Qt5::Widgets)

#INSTALL(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/rgbyuv-test DESTINATION bin)
