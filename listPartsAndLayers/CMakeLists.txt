set(PROG_NAME ${EXRTOOLS_PROG_PREFIX}-list)

add_executable(${PROG_NAME} main.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utils)
include_directories(${OpenEXR_INCLUDE_DIRS})

message(STATUS ${UTILS_LIB-NAME})
target_link_libraries(${PROG_NAME} ${UTILS_LIB-NAME})
target_link_libraries(${PROG_NAME} ${OpenEXR_LIBRARIES})
