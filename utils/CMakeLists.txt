include_directories(${OpenEXR_INCLUDE_DIRS})

add_library(${UTILS_LIB-NAME} isexr.cpp getversionfield.cpp)

target_link_libraries(${UTILS_LIB-NAME} ${OpenEXR_LIBRARIES})
