#set to minimum version that supports clean build
cmake_minimum_required(VERSION 3.14.0)

project(domoticz)

## required min. libBoost version
SET(DOMO_MIN_LIBBOOST_VERSION 1.66.0)
##

MACRO(History_GET_REVISION variable)
  IF(EXISTS ${CMAKE_SOURCE_DIR}/History.txt)
    MESSAGE(STATUS "Read ProjectRevision from History.txt")
    CMAKE_POLICY(SET CMP0007 NEW)
    FILE(STRINGS ${CMAKE_SOURCE_DIR}/History.txt AppVersionContent)
    LIST(GET AppVersionContent 0 AppVersionContent)
    STRING(REPLACE " " ";" AppVersionContent ${AppVersionContent})
    LIST(GET AppVersionContent 1 AppVersionContent)
    STRING(REPLACE "." ";" AppVersionContent ${AppVersionContent})
    LIST(GET AppVersionContent 1 ${variable})
  ELSE(EXISTS ${CMAKE_SOURCE_DIR}/History.txt)
    MESSAGE(STATUS "Failed to get ProjectRevision from History.txt, set it to 0")
    set (${variable} 0)
  ENDIF(EXISTS ${CMAKE_SOURCE_DIR}/History.txt)
ENDMACRO(History_GET_REVISION)

MACRO(Gitversion_GET_REVISION dir variable)
  EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} --git-dir ./.git rev-list HEAD --count
    WORKING_DIRECTORY ${dir}
    OUTPUT_VARIABLE ${variable}
    OUTPUT_STRIP_TRAILING_WHITESPACE)
ENDMACRO(Gitversion_GET_REVISION)

Gitversion_GET_REVISION("${CMAKE_SOURCE_DIR}" ProjectRevision)
IF(NOT ProjectRevision)
  MESSAGE(STATUS "Failed to get ProjectRevision from git")
  History_GET_REVISION(ProjectRevision)
ELSE(NOT ProjectRevision)
  MATH(EXPR ProjectRevision "${ProjectRevision}+2107")
ENDIF(NOT ProjectRevision)

MESSAGE(STATUS "###########################")
MESSAGE(STATUS "Compiling Revision #${ProjectRevision}")
MESSAGE(STATUS "###########################")

# The version number.
set (domoticz_VERSION_MAJOR 4)
set (domoticz_VERSION_MINOR 0)
set (domoticz_VERSION_PATCH ${ProjectRevision})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CXX_EXTENSIONS NO)
endif()

option(USE_OPENSSL_STATIC "Link OpenSSL static" YES)
IF(USE_OPENSSL_STATIC)
	set(OPENSSL_USE_STATIC_LIBS TRUE)
ENDIF(USE_OPENSSL_STATIC)

option(USE_BUILTIN_LUA "Use builtin lua library" YES)
IF(USE_BUILTIN_LUA)
  IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_definitions(-DLUA_USE_MACOSX)
  ENDIF()
  add_subdirectory (lua)
  get_directory_property (LUA_LIBRARIES DIRECTORY lua
    DEFINITION LUA_LIBRARIES)
ELSE(USE_BUILTIN_LUA)
  find_package(PkgConfig)
  pkg_search_module(LUA lua5.2>=5.2 lua>=5.2 lua-5.2)
  IF(LUA_FOUND)
    # Lua 5.2 pkg-config is broken.
    # Depending on whether the used distro fixed that to
    # provide backward compatibility we need to find the
    # include dir and library name ourselves.
    IF(NOT LUA_INCLUDE_DIRS)
      find_path(LUA_INC_DIRS NAMES "lua.h" PATH_SUFFIXES "lua5.2")
      set(LUA_INCLUDE_DIRS ${LUA_INC_DIRS})
    ENDIF(NOT LUA_INCLUDE_DIRS)
    message(STATUS "Found lua.h at: ${LUA_INCLUDE_DIRS}")
    INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIRS})
    add_definitions(-DWITH_EXTERNAL_LUA)
    IF(LUA_LIBRARY_DIRS)
      link_directories(${LUA_LIBRARY_DIRS})
    ELSE(LUA_LIBRARY_DIRS)
      find_library(LUA_LIB NAMES lua lua5.2)
      message(STATUS "Found lua library at: ${LUA_LIB}")
      set(LUA_LIBRARIES ${LUA_LIB})
    ENDIF(LUA_LIBRARY_DIRS)
  ELSE(LUA_FOUND)
    MESSAGE(FATAL_ERROR "LUA not found but USE_BUILTIN_LUA=NO")
  ENDIF(LUA_FOUND)
ENDIF(USE_BUILTIN_LUA)

option(USE_BUILTIN_SQLITE "Use builtin sqlite library" YES)
IF(USE_BUILTIN_SQLITE)
  add_subdirectory (sqlite)
  get_directory_property (SQLite_LIBRARIES DIRECTORY sqlite
      DEFINITION SQLite_LIBRARIES)
ELSE(USE_BUILTIN_SQLITE)
  find_package(PkgConfig)
  pkg_check_modules(SQLite QUIET sqlite3)
  IF(SQLite_FOUND)
    INCLUDE_DIRECTORIES(${SQLite_INCLUDE_DIRS})
    add_definitions(-DWITH_EXTERNAL_SQLITE)
  ELSE(SQLite_FOUND)
    MESSAGE(FATAL_ERROR "SQLite3 not found but USE_BUILTIN_SQLITE=NO")
  ENDIF(SQLite_FOUND)
ENDIF(USE_BUILTIN_SQLITE)

option(USE_PYTHON "Use Python for Plugins" YES)
IF(USE_PYTHON)
  find_package(PythonInterp 3.4)
  find_package(PythonLibs 3.4)
  IF(PYTHONLIBS_FOUND)
    MESSAGE(STATUS "Python3 includes found at: ${PYTHON_INCLUDE_PATH}")
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
    add_definitions(-DENABLE_PYTHON)
  ELSE(PYTHONLIBS_FOUND)
    MESSAGE(FATAL_ERROR "Python3 not found on your system, use USE_PYTHON=NO or sudo apt-get install python3-dev)")
  ENDIF(PYTHONLIBS_FOUND)
ENDIF(USE_PYTHON)

SET(EXECUTABLE_OUTPUT_PATH "" CACHE INTERNAL
  "Where to put the executables for Domoticz"
  )

INCLUDE(CheckIncludeFile)
CHECK_INCLUDE_FILE (execinfo.h HAVE_EXECINFO_H)

IF(HAVE_EXECINFO_H)
  # FreeBSD has to include libexecinfo
  IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    find_library(EXECINFO_LIBRARIES NAMES libexecinfo.so)
    IF(EXECINFO_LIBRARIES)
      ADD_DEFINITIONS(-DHAVE_EXECINFO_H)
    ENDIF()
  ELSEIF(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    find_library(EXECINFO_LIBRARIES NAMES libexecinfo.so)
    IF(EXECINFO_LIBRARIES)
      ADD_DEFINITIONS(-DHAVE_EXECINFO_H)
    ENDIF()
  ELSE()
    ADD_DEFINITIONS(-DHAVE_EXECINFO_H)
  ENDIF()
ENDIF(HAVE_EXECINFO_H)

INCLUDE(CheckIncludeFiles)

#set(CMAKE_EXE_LINKER_FLAGS "-static")

# Macro for setting up precompiled headers. Usage:
#
#   add_precompiled_header(target header.h [FORCEINCLUDE])
#
# MSVC: A source file with the same name as the header must exist and
# be included in the target (E.g. header.cpp).
#
# MSVC: Add FORCEINCLUDE to automatically include the precompiled
# header file from every source file.
#
# GCC: The precompiled header is always automatically included from
# every header file.
MACRO(ADD_PRECOMPILED_HEADER _targetName _input)
  GET_FILENAME_COMPONENT(_inputWe ${_input} NAME_WE)
  SET(pch_source ${_inputWe}.cpp)
  FOREACH(arg ${ARGN})
    IF(arg STREQUAL FORCEINCLUDE)
      SET(FORCEINCLUDE ON)
    ELSE(arg STREQUAL FORCEINCLUDE)
      SET(FORCEINCLUDE OFF)
    ENDIF(arg STREQUAL FORCEINCLUDE)
  ENDFOREACH(arg)

  IF(MSVC)
    GET_TARGET_PROPERTY(sources ${_targetName} SOURCES)
    SET(_sourceFound FALSE)
    FOREACH(_source ${sources})
      SET(PCH_COMPILE_FLAGS "")
      IF(_source MATCHES \\.\(cc|cxx|cpp\)$)
  GET_FILENAME_COMPONENT(_sourceWe ${_source} NAME_WE)
  IF(_sourceWe STREQUAL ${_inputWe})
    SET(PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /Yc${_input}")
    SET(_sourceFound TRUE)
  ELSE(_sourceWe STREQUAL ${_inputWe})
    SET(PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /Yu${_input}")
    IF(FORCEINCLUDE)
      SET(PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /FI${_input}")
    ENDIF(FORCEINCLUDE)
  ENDIF(_sourceWe STREQUAL ${_inputWe})
  SET_SOURCE_FILES_PROPERTIES(${_source} PROPERTIES COMPILE_FLAGS "${PCH_COMPILE_FLAGS}")
      ENDIF(_source MATCHES \\.\(cc|cxx|cpp\)$)
    ENDFOREACH()
    IF(NOT _sourceFound)
      MESSAGE(FATAL_ERROR "A source file for ${_input} was not found. Required for MSVC builds.")
    ENDIF(NOT _sourceFound)
  ENDIF(MSVC)

  IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch -Wno-parentheses-equality -Wno-deprecated-declarations -Wno-tautological-compare -Wno-unused-value -Wno-comment -Wno-unsequenced -Wno-logical-op-parentheses -Wno-literal-conversion")
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  ENDIF()
  IF(CMAKE_SYSTEM_NAME STREQUAL "NetBSD")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
  ENDIF()
  IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-psabi")
    #SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-switch")
    #ADD_DEFINITIONS( -Wall -O0 -ggdb )
    #ADD_DEFINITIONS( -Wfatal-errors -Wformat=2 -Werror=format-security )

    GET_FILENAME_COMPONENT(_name ${_input} NAME)
    SET(_source "${CMAKE_CURRENT_SOURCE_DIR}/${_input}")
    SET(_outdir "${CMAKE_CURRENT_BINARY_DIR}/${_name}.gch")
    MAKE_DIRECTORY(${_outdir})
    SET(_output "${_outdir}/.c++")

    STRING(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" _flags_var_name)
    # Strip out -Wl,… linker directives as they make GCC fail
    STRING(REGEX REPLACE " -Wl,[^ ]*" " " _compiler_FLAGS " ${CMAKE_CXX_FLAGS} ${${_flags_var_name}}")

    GET_DIRECTORY_PROPERTY(_directory_flags INCLUDE_DIRECTORIES)
    FOREACH(item ${_directory_flags})
      LIST(APPEND _compiler_FLAGS "-I${item}")
    ENDFOREACH(item)

    GET_DIRECTORY_PROPERTY(_directory_flags COMPILE_DEFINITIONS)
    FOREACH(item ${_directory_flags})
      LIST(APPEND _compiler_FLAGS "-D${item}")
    ENDFOREACH(item)

    SEPARATE_ARGUMENTS(_compiler_FLAGS)
    MESSAGE("${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_compiler_FLAGS} -x c++-header -o ${_output} ${_source}")
    ADD_CUSTOM_COMMAND(
      OUTPUT ${_output}
      COMMAND ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${_compiler_FLAGS} -x c++-header -o ${_output} ${_source}
      DEPENDS ${_source} )
    ADD_CUSTOM_TARGET(${_targetName}_gch DEPENDS ${_output})
    ADD_DEPENDENCIES(${_targetName} ${_targetName}_gch)
    SET_TARGET_PROPERTIES(${_targetName} PROPERTIES COMPILE_FLAGS "-include ${_name} -Winvalid-pch")
  ENDIF(CMAKE_COMPILER_IS_GNUCXX)
ENDMACRO()

FIND_PROGRAM(GIT_EXECUTABLE git
  DOC "git command line client")


include_directories(${CMAKE_SOURCE_DIR}/main)
include_directories(${CMAKE_SOURCE_DIR}/tinyxpath)

# a custom target that is always built
ADD_CUSTOM_TARGET(revisiontag ALL)

# creates appversion.h using cmake script
ADD_CUSTOM_COMMAND(TARGET revisiontag COMMAND ${CMAKE_COMMAND}
   -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
   -P ${CMAKE_CURRENT_SOURCE_DIR}/getgit.cmake)

## Target
set(
domoticz_SRCS
main/stdafx.cpp
main/BaroForecastCalculator.cpp
main/CmdLine.cpp
main/Camera.cpp
main/domoticz.cpp
main/EventSystem.cpp
main/EventsPythonModule.cpp
main/EventsPythonDevice.cpp
main/Helper.cpp
main/HTMLSanitizer.cpp
main/localtime_r.cpp
main/Logger.cpp
main/LuaCommon.cpp
main/LuaHandler.cpp
main/mainworker.cpp
main/RFXNames.cpp
main/Scheduler.cpp
main/SignalHandler.cpp
main/SQLHelper.cpp
main/SunRiseSet.cpp
main/TrendCalculator.cpp
main/WebServer.cpp
main/WebServerHelper.cpp
main/WindCalculation.cpp
push/BasePush.cpp
push/WebsocketPush.cpp
httpclient/HTTPClient.cpp
httpclient/UrlEncode.cpp
hardware/ASyncSerial.cpp
hardware/ASyncTCP.cpp
hardware/ColorSwitch.cpp
hardware/csocket.cpp
hardware/DomoticzHardware.cpp
hardware/DomoticzInternal.cpp
hardware/DomoticzTCP.cpp
hardware/OpenZWave.cpp
hardware/openzwave/control_panel/ozwcp.cpp
hardware/openzwave/control_panel/zwavelib.cpp
hardware/RFXBase.cpp
hardware/serial/serial.cpp
hardware/serial/impl/unix.cpp
hardware/ZWaveBase.cpp
hardware/plugins/DelayedLink.cpp
hardware/plugins/Plugins.cpp
hardware/plugins/PluginManager.cpp
hardware/plugins/PluginProtocols.cpp
hardware/plugins/PluginTransports.cpp
hardware/plugins/PythonObjects.cpp
notifications/NotificationBase.cpp
notifications/NotificationBrowser.cpp
notifications/NotificationHelper.cpp
tcpserver/TCPClient.cpp
tcpserver/TCPServer.cpp
webserver/Base64.cpp
webserver/connection.cpp
webserver/connection_manager.cpp
webserver/cWebem.cpp
webserver/fastcgi.cpp
webserver/mime_types.cpp
webserver/proxycereal.cpp
webserver/proxyclient.cpp
webserver/reply.cpp
webserver/request_handler.cpp
webserver/request_parser.cpp
webserver/server.cpp
webserver/Websockets.cpp
webserver/WebsocketHandler.cpp
json/json_reader.cpp
json/json_value.cpp
json/json_writer.cpp
tinyxpath/action_store.cpp
tinyxpath/htmlutil.cpp
tinyxpath/lex_util.cpp
tinyxpath/node_set.cpp
tinyxpath/tinystr.cpp
tinyxpath/tinyxml.cpp
tinyxpath/tinyxmlerror.cpp
tinyxpath/tinyxmlparser.cpp
tinyxpath/tokenlist.cpp
tinyxpath/xml_util.cpp
tinyxpath/xpath_expression.cpp
tinyxpath/xpath_processor.cpp
tinyxpath/xpath_stream.cpp
tinyxpath/xpath_stack.cpp
tinyxpath/xpath_static.cpp
tinyxpath/xpath_syntax.cpp
)
add_executable(domoticz ${domoticz_SRCS})

# explicitly say that the executable depends on the revisiontag
add_dependencies(domoticz revisiontag)

INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
IF(${BIGENDIAN})
     ADD_DEFINITIONS(-DIS_BIG_ENDIAN)
ENDIF(${BIGENDIAN})

## Link libraries

#
# Find MD5/RMD160/SHA library
#
find_package(OpenSSL REQUIRED)
if(NOT OPENSSL_INCLUDE_DIR)
	message(SEND_ERROR "Failed to find OpenSSL include files (ssl.h), no HTTPS support")
endif()
if(NOT OPENSSL_FOUND)
  message(SEND_ERROR "Failed to find the OpenSSL library, no HTTPS support")
  find_library(MD_LIBRARY NAMES md)
  if(MD_LIBRARY)
    target_link_libraries(domoticz ${MD_LIBRARY})
  endif(MD_LIBRARY)
else()
  message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
  add_definitions(-DWWW_ENABLE_SSL)
  add_definitions(-DWITH_TLS)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(domoticz ${OPENSSL_LIBRARIES})
endif()

#
# Boost
#
option(USE_STATIC_BOOST "Build with static BOOST libraries" YES)
set(Boost_USE_STATIC_LIBS ${USE_STATIC_BOOST})
set(Boost_USE_MULTITHREADED ON)
unset(Boost_INCLUDE_DIR CACHE)
unset(Boost_LIBRARY_DIRS CACHE)

if(USE_STATIC_BOOST)
   message(STATUS "Linking against boost static libraries")
else(USE_STATIC_BOOST)
   message(STATUS "Linking against boost dynamic libraries")
endif(USE_STATIC_BOOST)

find_package(Boost REQUIRED COMPONENTS thread system)
target_link_libraries(domoticz Boost::thread Boost::system)

# compare found vs required libBoost version
if(Boost_VERSION VERSION_LESS DOMO_MIN_LIBBOOST_VERSION)
   message(FATAL_ERROR "Found libBoost version ${Boost_VERSION}, ${DOMO_MIN_LIBBOOST_VERSION} or newer required")
endif(Boost_VERSION VERSION_LESS DOMO_MIN_LIBBOOST_VERSION)

include_directories(${Boost_INCLUDE_DIRS})

#
# ZLIB
#
option(USE_BUILTIN_ZLIB "Use builtin zlib library" NO)
if(USE_BUILTIN_ZLIB)
  add_subdirectory (zlib)
  include_directories(${ZLIB_INCLUDE_DIRS})
else(USE_BUILTIN_ZLIB)
  find_package(ZLIB REQUIRED)
  if(ZLIB_FOUND)
    MESSAGE(STATUS "ZLIB libraries found at: ${ZLIB_LIBRARIES}")
    MESSAGE(STATUS "ZLIB includes found at: ${ZLIB_INCLUDE_DIRS}")
    include_directories(${ZLIB_INCLUDE_DIRS})
  else()
    MESSAGE(FATAL_ERROR "ZLIB not found on your system, see install.txt how to get them installed. (for example 'sudo apt-get install zlib1g-dev')")
  endif(ZLIB_FOUND)
endif(USE_BUILTIN_ZLIB)

#
# MINIZIP
#
option(USE_BUILTIN_MINIZIP "Use builtin minizip library" YES)
if(USE_BUILTIN_MINIZIP)
  add_subdirectory(zip)
  get_directory_property(MINIZIP_LIBRARIES DIRECTORY zip DEFINITION MINIZIP_LIBRARIES)
  get_directory_property(MINIZIP_INCLUDE_DIRS DIRECTORY zip DEFINITION MINIZIP_INCLUDE_DIRS)
  include_directories(${MINIZIP_INCLUDE_DIRS})
else(USE_BUILTIN_MINIZIP)
  find_package(PkgConfig)
  pkg_check_modules(MINIZIP minizip)
  if(MINIZIP_FOUND)
    MESSAGE(STATUS "MINIZIP libraries found at: ${MINIZIP_LIBRARIES}")
    MESSAGE(STATUS "MINIZIP includes found at: ${MINIZIP_INCLUDE_DIRS}")
    include_directories(${MINIZIP_INCLUDE_DIRS})
  else()
    MESSAGE(FATAL_ERROR "MINIZIP not found on your system, see install.txt how to get them installed. (for example 'sudo apt-get install libminizip-dev')")
  endif(MINIZIP_FOUND)
endif(USE_BUILTIN_MINIZIP)

#
# CURL
#
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  MESSAGE(STATUS "Curl libraries found at: ${CURL_LIBRARIES}")
  MESSAGE(STATUS "Curl includes found at: ${CURL_INCLUDE_DIRS}")
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})
else()
  MESSAGE(FATAL_ERROR "cURL not found on your system, see install.txt how to get them installed. (for example 'sudo apt-get install curl libcurl4-gnutls-dev')")
ENDIF(CURL_FOUND)

#
# USB
#
option(WITH_LIBUSB "Enable libusb support" YES)
if(WITH_LIBUSB)
  find_path(LIBUSB_INCLUDE_DIR usb.h
     HINTS ${PC_LIBUSB_INCLUDEDIR} ${PC_LIBUSB_INCLUDE_DIRS})
  find_library(LIBUSB_LIBRARY NAMES usb
     HINTS ${PC_LIBUSB_LIBDIR} ${PC_LIBUSB_LIBRARY_DIRS})
  set(LIBUSB_LIBRARIES ${LIBUSB_LIBRARY})

  find_package_handle_standard_args(LIBUSB  DEFAULT_MSG  LIBUSB_LIBRARIES LIBUSB_INCLUDE_DIR)
  IF(LIBUSB_FOUND)
    MESSAGE(STATUS "LIBUSB found at: ${LIBUSB_LIBRARIES}")
    add_definitions(-DWITH_LIBUSB)
    target_link_libraries(domoticz ${LIBUSB_LIBRARIES})
  else()
    MESSAGE(STATUS "==== LibUSB not found, support for TE923/Voltcraft disabled!")
  ENDIF(LIBUSB_FOUND)
endif(WITH_LIBUSB)

## support lua popen on Linux platforms
#IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
# SET(OperatingSystem "Linux")
# add_definitions(-DLUA_USE_LINUX)
#ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

#
# OpenZWave
# try to find open-zwave, if found, include support
#
option(USE_STATIC_OPENZWAVE "Build with static OpenZwave libraries" YES)
if(USE_STATIC_OPENZWAVE)
  find_library(OpenZWave NAMES libopenzwave.a HINTS "../open-zwave-read-only" "../open-zwave-read-only/cpp/build")
  set(OPENZWAVE_LIB ${OpenZWave})
else()
  pkg_check_modules(OPENZWAVE libopenzwave)
  if(OPENZWAVE_FOUND)
    MESSAGE(STATUS "==== OpenZWave package found!")
    find_library(OpenZWave NAMES libopenzwave.so HINTS ${OPENZWAVE_LIBRARY_DIRS})
    message(STATUS ${OpenZWave})
  endif()
endif()
IF(OpenZWave)
  message(STATUS ${OpenZWave})
  target_link_libraries(domoticz ${OpenZWave})
  include_directories(${CMAKE_SOURCE_DIR}/hardware/openzwave)
  add_definitions(-DWITH_OPENZWAVE)
ELSE()
  MESSAGE(STATUS "==== OpenZWave not found, support disabled!")
ENDIF(OpenZWave)


target_link_libraries(domoticz ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${MINIZIP_LIBRARIES} ${CURL_LIBRARIES} pthread ${LUA_LIBRARIES} ${SQLite_LIBRARIES} ${CMAKE_DL_LIBS})

IF(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  target_link_libraries(domoticz -lresolv)
elseif(CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
  target_link_libraries(domoticz ${EXECINFO_LIBRARIES})
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  target_link_libraries(domoticz ${EXECINFO_LIBRARIES})
else()
  target_link_libraries(domoticz -lrt -lresolv ${EXECINFO_LIBRARIES})
ENDIF()

ADD_PRECOMPILED_HEADER(domoticz "main/stdafx.h")

IF(CMAKE_COMPILER_IS_GNUCXX)
  option(USE_STATIC_LIBSTDCXX "Build with static libgcc/libstdc++ libraries" YES)
  IF(USE_STATIC_LIBSTDCXX)
    message(STATUS "Using static libgcc/libstdc++")
    SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS} -static-libgcc -static-libstdc++")
    SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "${CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS} -static-libgcc -static-libstdc++")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -static-libgcc -static-libstdc++")
  ELSE(USE_STATIC_LIBSTDCXX)
    message(STATUS "Using dynamic libgcc_s/libstdc++")
  ENDIF(USE_STATIC_LIBSTDCXX)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)


# build a CPack driven installer package
INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_PACKAGE_NAME "domoticz")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${domoticz}-Home Automation System (Domotica).")
SET(CPACK_PACKAGE_VENDOR "Domoticz.com")
SET(CPACK_PACKAGE_CONTACT "info@domoticz.com")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")

SET(CPACK_PACKAGE_VERSION_MAJOR "${domoticz_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${domoticz_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${domoticz_VERSION_PATCH}")

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "domoticz")
#SET (DOMOTICZ_VERSION_SHORT "${domoticz_VERSION_MAJOR}.${domoticz_VERSION_MINOR}.${domoticz_VERSION_PATCH}")
#SET(CPACK_PACKAGE_FILE_NAME "domoticz-${DOMOTICZ_VERSION_SHORT}-${CMAKE_SYSTEM_NAME}")

SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${domoticz}-dev")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
SET(CPACK_PACKAGE_DEFAULT_LOCATION "/opt/${CPACK_PACKAGE_NAME}")
SET(CPACK_PACKAGE_EXECUTABLES "domoticz;Home Automation System (Domotica).")

#set(CPACK_DEB_COMPONENT_INSTALL TRUE)
SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

INCLUDE(CPack)

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/opt/${CPACK_PACKAGE_NAME}" CACHE PATH "Install path prefix, prepended onto install directories." FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

install(TARGETS domoticz DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/www DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/License.txt DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/History.txt DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/server_cert.pem DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/scripts DESTINATION ${CMAKE_INSTALL_PREFIX} USE_SOURCE_PERMISSIONS)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugins DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Config DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dzVents DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/updatedomo DESTINATION ${CMAKE_INSTALL_PREFIX} PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ)
