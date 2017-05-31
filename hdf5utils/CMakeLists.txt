set(EXR2HDF5_NAME ${EXRTOOLS_PROG_PREFIX}-exr2hdf5)
set(HDF52EXR_NAME ${EXRTOOLS_PROG_PREFIX}-hdf52exr)

find_package(HDF5 QUIET COMPONENTS CXX)

if(HDF5_FOUND)

    INCLUDE_DIRECTORIES(${HDF5_INCLUDE_DIRS})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utils)

    add_executable(${EXR2HDF5_NAME} exr2hdf5.cpp)
    add_executable(${HDF52EXR_NAME} hdf52exr.cpp)

    target_link_libraries(${EXR2HDF5_NAME} ${HDF5_LIBRARIES} ${HDF5_CXX_LIBRARIES})
    target_link_libraries(${EXR2HDF5_NAME} ${UTILS_LIB-NAME})
    target_link_libraries(${EXR2HDF5_NAME} ${OpenEXR_LIBRARIES})

    target_link_libraries(${HDF52EXR_NAME} ${HDF5_LIBRARIES} ${HDF5_CXX_LIBRARIES})
    target_link_libraries(${HDF52EXR_NAME} ${UTILS_LIB-NAME})
    target_link_libraries(${HDF52EXR_NAME} ${OpenEXR_LIBRARIES})

endif(HDF5_FOUND)
