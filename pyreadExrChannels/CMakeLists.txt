if (EXRTOOLS-PYTHON)

set(LIB_NAME ${EXRTOOLS_LIB_PREFIX}_pyreadExrChannels)

if (USE-PYTHON-2)

find_package(Python 2. COMPONENTS Interpreter Development)

else (USE-PYTHON-2)

find_package(Python COMPONENTS Interpreter Development)

endif (USE-PYTHON-2)

add_definitions(-DLIB_NAME=${LIB_NAME})

add_library(${LIB_NAME} SHARED pyreadexrchannels.cpp)
SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utils)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/pybind11/include)
include_directories(${OpenEXR_INCLUDE_DIRS})
include_directories(${Python_INCLUDE_DIRS})

target_link_libraries(${LIB_NAME} ${UTILS_LIB-NAME})
target_link_libraries(${LIB_NAME} ${OpenEXR_LIBRARIES})
target_link_libraries(${LIB_NAME} ${Python_LIBRARIES})

install(TARGETS ${LIB_NAME} DESTINATION lib/python${Python_VERSION_MAJOR}.${Python_VERSION_MINOR})

endif (EXRTOOLS-PYTHON)
