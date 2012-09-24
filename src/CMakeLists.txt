# ---------------------------------------------------
# Definicion
# ---------------------------------------------------
cmake_minimum_required(VERSION 2.6)
project (TP-SO)

#MESSAGE("CMAKE_CXX_LINK_EXECUTABLE = [${CMAKE_CXX_LINK_EXECUTABLE}]")
#MESSAGE("CMAKE_C_LINK_EXECUTABLE = [${CMAKE_C_LINK_EXECUTABLE}]")

SET(CMAKE_C_COMPILER "${CMAKE_CXX_COMPILER}") 

#MESSAGE("CMAKE_CXX_COMPILER = [${CMAKE_CXX_COMPILER}]")
#MESSAGE("CMAKE_C_COMPILER = [${CMAKE_C_COMPILER}]")


#MESSAGE("CMAKE_CXX_CREATE_SHARED_LIBRARY = [${CMAKE_CXX_CREATE_SHARED_LIBRARY}]")
#MESSAGE("CMAKE_C_CREATE_SHARED_LIBRARY = [${CMAKE_C_CREATE_SHARED_LIBRARY}]")
#SET(CMAKE_CXX_CREATE_SHARED_LIBRARY "<CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
#SET(CMAKE_C_CREATE_SHARED_LIBRARY "<CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
#MESSAGE("CMAKE_CXX_CREATE_SHARED_LIBRARY = [${CMAKE_CXX_CREATE_SHARED_LIBRARY}]")
#MESSAGE("CMAKE_C_CREATE_SHARED_LIBRARY = [${CMAKE_C_CREATE_SHARED_LIBRARY}]")
# ---------------------------------------------------
# Configuracion
# ---------------------------------------------------
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -fPIC -pedantic -Wall ")
set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -ansi -fPIC -pedantic -Wall -Werror-implicit-function-declaration -Wno-write-strings")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fPIC  ")
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC  -Werror-implicit-function-declaration")
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -o0 -g -pg -finstrument-functions")
#include_directories("/opt/local/include")

enable_testing ()
include_directories(${CMAKE_SOURCE_DIR})
link_directories(${CMAKE_SOURCE_DIR}/libs/macos)
link_directories(${CMAKE_SOURCE_DIR}/libs/windows)
link_directories(${CMAKE_SOURCE_DIR}/libs/linux)
link_directories(${CMAKE_SOURCE_DIR}/libs/sunos)

# ---------------------------------------------------
# Funciones / Macros
# ---------------------------------------------------
macro (crear_ejecutable file)
  add_executable (${file}.bin ${file})
  foreach(f ${ARGN})
    target_link_libraries (${file}.bin ${f})
  endforeach(f)
  target_link_libraries (${file}.bin Log)
  target_link_libraries (${file}.bin pthread)
  IF("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
    target_link_libraries (${file}.bin socket )
    target_link_libraries (${file}.bin nsl)
    target_link_libraries (${file}.bin resolv)
    target_link_libraries (${file}.bin rt)
  ENDIF("${CMAKE_SYSTEM_NAME}" STREQUAL "SunOS")
endmacro (crear_ejecutable)

# ---------------------------------------------------
# Modulos
# ---------------------------------------------------

add_subdirectory (model)
add_subdirectory (xml)
add_subdirectory (ipc)
add_subdirectory (node)
add_subdirectory (md5)
add_subdirectory (util)

add_subdirectory (webserver)

add_subdirectory (tests)


