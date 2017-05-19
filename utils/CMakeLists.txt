include_directories(${OpenEXR_INCLUDE_DIRS})

add_library(${UTILS_LIB-NAME} SHARED isexr.cpp getversionfield.cpp)

target_link_libraries(${UTILS_LIB-NAME} ${OpenEXR_LIBRARIES})
