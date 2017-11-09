cmake_minimum_required(VERSION 2.4.7)

##################  Paths  ################################
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/share/gsc/")
set(BIN_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/bin")

if(COMMAND cmake_policy)
 cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_SOURCE_DIR})

set(util_SRCS prtgsc.c dispos.c decode_c.c get_head.c find_reg.c to_d.c dtos.c)

######################################
################ GSC #################
######################################

add_executable(gsc gsc.c ${util_SRCS})

target_link_libraries(gsc m)

install(TARGETS gsc RUNTIME DESTINATION bin )

################################################
#################### Decode ####################
################################################
add_executable(decode decode.c decode_c.c get_head.c prtgsc.c dtos.c)

target_link_libraries(decode m)

install(TARGETS decode RUNTIME DESTINATION bin )

###################################
## Install Files and Directories ##
###################################

install(FILES regions.bin regions.ind ${DATA_INSTALL_DIR}/bin)
install(DIRECTORY N0000 N1500 N3000 N4500 N6000 N7500 N0730 N2230 N3730 N5230 N6730 N8230 S0000 S1500 S3000 S4500 S6000 S7500 S0730 S2230 S3730 S5230 S6730 S8230 DESTINATION ${DATA_INSTALL_DIR})

