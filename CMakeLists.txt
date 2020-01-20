CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

project("exr tools")

#options

option(DTM2EXR-UTILS "A set of tools helping to convert standard digital terrain model to exr an vice versa" OFF)
option(HDF2EXR-UTILS "A set of tools helping to convert hdf file format to exr and vice versa" OFF)
option(HYPERSPECTR2EXR-UTILS "A set of tools helping to convert standard hyperspectral image bil file format to exr" OFF)
option(EXR-LISTUTIL "A tool to inspect exr images" ON)
option(EXRTOOLS-PYTHON "A non official biding for python, is still incomplete thus" ON)
option(RASTERMAP2EXR "This is a old tool develloped for a special application, you shouldn't need it" OFF)

option(USE-PYTHON-2 "force the system to look for python2 on systems where it is installed along python3" OFF)

#config

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
add_subdirectory(convertDtmFromExrtoRaster)
add_subdirectory(pyreadExrChannels)
add_subdirectory(hdf5utils)
add_subdirectory(rasterLabelsToExrMaps)
add_subdirectory(hyperspectralutils)
