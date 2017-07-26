set(BIL2EXR_NAME ${EXRTOOLS_PROG_PREFIX}-bil2exr)

find_package(GDAL QUIET)

message("Has GDAL been found ? ${GDAL_FOUND}")
if(GDAL_FOUND)

    INCLUDE_DIRECTORIES(${GDAL_INCLUDE_DIR})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utils)

    add_executable(${BIL2EXR_NAME} bil2exr.cpp)

    target_link_libraries(${BIL2EXR_NAME} ${GDAL_LIBRARY})
    target_link_libraries(${BIL2EXR_NAME} ${UTILS_LIB-NAME})
    target_link_libraries(${BIL2EXR_NAME} ${OpenEXR_LIBRARIES})

endif(GDAL_FOUND)
