CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project("exr tools")

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

#ecm contain a module to configure openexr libs.
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

#will the the variables we'll need to use the library
include(FindOpenEXR)

if (NOT OpenEXR_FOUND)
    message(FATAL_ERROR "Open exr not found !")
endif (NOT OpenEXR_FOUND)

set(EXRTOOLS_PROG_PREFIX "exrtools")
set(EXRTOOLS_LIB_PREFIX "lexrtools")

#libraries
set(UTILS_LIB-NAME ${EXRTOOLS_LIB_PREFIX}-utils)
add_subdirectory(utils)

#programs
add_subdirectory(listPartsAndLayers)
add_subdirectory(convertDtmFromRasterToExr)
