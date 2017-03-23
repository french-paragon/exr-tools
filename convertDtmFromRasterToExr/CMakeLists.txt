set(PROG_NAME ${EXRTOOLS_PROG_PREFIX}-dtm_convert)

add_executable(${PROG_NAME} main.cpp)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../utils)
include_directories(${OpenEXR_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../submodules/CImg)

target_link_libraries(${PROG_NAME} ${UTILS_LIB-NAME})
target_link_libraries(${PROG_NAME} ${OpenEXR_LIBRARIES})

find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
  target_compile_options(PUBLIC my_app "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
  target_link_libraries(${PROG_NAME} "${CMAKE_THREAD_LIBS_INIT}")
endif()

FIND_PACKAGE(JPEG)
FIND_PACKAGE(TIFF)
FIND_PACKAGE(PNG)
FIND_PACKAGE(ZLIB)


set(CIMG_JPEG_CCFLAGS  -Dcimg_use_jpeg)

set(CIMG_TIFF_CCFLAGS  -Dcimg_use_tiff)

set(CIMG_PNG_CCFLAGS -Dcimg_use_png)

if(TIFF_FOUND)
  get_filename_component(TIFF_LIB_DIRS ${TIFF_LIBRARIES} PATH)
  set(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_TIFF_CCFLAGS}")
  link_directories(${TIFF_LIB_DIRS})
  include_directories(${TIFF_INCLUDE_DIR})
  target_link_libraries(${PROG_NAME} ${TIFF_LIBRARIES})
endif(TIFF_FOUND)

if(JPEG_FOUND)
  get_filename_component(JPEG_LIB_DIRS ${JPEG_LIBRARIES} PATH)
  set(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_JPEG_CCFLAGS}")
  link_directories(${JPEG_LIB_DIRS})
  include_directories(${JPEG_INCLUDE_DIR})
  target_link_libraries(${PROG_NAME} ${JPEG_LIBRARIES})
endif()

if (ZLIB_FOUND)
  set(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_ZLIB_CCFLAGS}")
  link_directories(${ZLIB_LIB_DIRS})
  include_directories(${ZLIB_INCLUDE_DIR})
  target_link_libraries(${PROG_NAME} ${ZLIB_LIBRARIES})

  # PNG requires ZLIB
  if(PNG_FOUND)
    set(CIMG_CFLAGS "${CIMG_CFLAGS} ${CIMG_PNG_CCFLAGS}")
    link_directories(${PNG_LIB_DIRS})
    include_directories(${PNG_INCLUDE_DIR} )
    target_link_libraries(${PROG_NAME} ${PNG_LIBRARIES} )
  endif(PNG_FOUND)
endif(ZLIB_FOUND)

set_target_properties(${PROG_NAME} PROPERTIES COMPILE_FLAGS ${CIMG_CFLAGS})
