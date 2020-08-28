##
##    Copyright: O-Marius Chincisan 2016-2017 Zirexix Inc
##
##    This program is distributed
##    WITHOUT ANY WARRANTY; without even the implied warranty of
##    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##
##    The library or code from is restricted to be used in commercial products
##    without bunget.cert file. Please obtain a custom cert file from admin@meeiot.org
##
##
cmake_minimum_required (VERSION 2.6)

include_directories ("./include")

add_definitions(-DDEBUG)
add_definitions(-DACL_MTU_FRAG)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ggdb")
set(LIBRARY_OUTPUT_PATH ../link)

add_library(bungetLib STATIC ascon.cpp
    bt_incinpl.cpp
    bt_socket.cpp
    bu_gap.cpp
    bu_gatt.cpp
    bu_hci.cpp
    bybuff.cpp
    gattdefs.cpp
    hci_socket.cpp
    l2cap_socket.cpp
    libbungetpriv.cpp
    rfcomm_socket.cpp
    sco_socket.cpp
    secmanp.cpp
    uguid.cpp)
