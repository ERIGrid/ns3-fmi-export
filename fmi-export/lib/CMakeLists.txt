# -------------------------------------------------------------------
# Copyright (c) 2013-2017, AIT Austrian Institute of Technology GmbH.
# All rights reserved. See file FMIPP_LICENSE for details.
# -------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)


# Variable to be defined by the user.
set( FMIPP_ROOT_DIR "./fmipp-code" CACHE PATH "directory containing the FMI++ source code" )

if ( NOT EXISTS ${FMIPP_ROOT_DIR} )
   message( FATAL_ERROR "FMI++ source directory not found." )
endif ()


project( fmippex_ns3 )


set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR} )


# OS-specific compiler settings
if ( WIN32 ) # windows-specific
   set( PATH_SEPARATOR ";" )
   set( CMAKE_SHARED_LIBRARY_PREFIX "" )
   set( CMAKE_SHARED_MODULE_PREFIX "" )
   add_definitions( -DWIN32 )
   if ( MINGW )
      add_definitions( -DMINGW -Wall ) # -O1
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=gnu++11" )
      endif ()
   endif ()
   if ( MSVC )
      # disable auto-linking and use cmake's dependency handling
      add_definitions( -DBOOST_ALL_NO_LIB /W3 )
   endif ()
elseif ( CYGWIN ) # Cygwin
   set( PATH_SEPARATOR ":" )
   add_definitions( -Wall -O2 )
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x -D_BSD_SOURCE" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=c++11 -D_BSD_SOURCE" )
      endif ()
else () # linux-specific
   set( PATH_SEPARATOR ":" )
   add_definitions( -Wall -O2 )
      if ( CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.7.0" )
         set( CMAKE_CXX_FLAGS "-std=c++0x" )
      else ()
         set( CMAKE_CXX_FLAGS "-std=c++11" )
      endif ()
endif ()


# Boost dependencies here
find_package( Boost COMPONENTS date_time system filesystem REQUIRED )
if ( Boost_FOUND )
   include_directories( ${Boost_INCLUDE_DIRS} )
   set( BOOST_LIBRARYDIR ${Boost_LIBRARY_DIRS} )
   set( Boost_USE_STATIC_LIBS OFF ) # OFF when using shared Boost libs.
   set( Boost_USE_MULTITHREADED ON )
   set( Boost_USE_STATIC_RUNTIME OFF )
endif ()


# common include directories
include_directories( ${FMIPP_ROOT_DIR} )
include_directories( ${FMIPP_ROOT_DIR}/common )
include_directories( ${FMIPP_ROOT_DIR}/common/fmi_v1.0 )
include_directories( ${FMIPP_ROOT_DIR}/common/fmi_v1.0 )
include_directories( ${FMIPP_ROOT_DIR}/export/functions/fmi_v2.0 )
include_directories( ${FMIPP_ROOT_DIR}/export/include )


# Define shared library: FMI back-end component for ns-3 FMUs
add_library( fmipp_backend SHARED
  ${FMIPP_ROOT_DIR}/export/src/BackEndApplicationBase.cpp
  ${FMIPP_ROOT_DIR}/export/src/FMIComponentBackEnd.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCSlaveLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMSlave.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMManager.cpp
  ${FMIPP_ROOT_DIR}/export/src/HelperFunctions.cpp
  ${FMIPP_ROOT_DIR}/export/src/ScalarVariable.cpp )


# OS-specific dependencies here
if ( WIN32 )
   set_target_properties( fmipp_backend PROPERTIES COMPILE_FLAGS "-DBUILD_FMI_DLL" "-DBACKEND_USE_PARENT_PID" )
   target_link_libraries( fmipp_backend ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} shlwapi )
elseif ( APPLE )
   set_target_properties( fmipp_backend PROPERTIES COMPILE_FLAGS "-DBACKEND_USE_PARENT_PID" )
   target_link_libraries( fmipp_backend ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} )
else ()
   set_target_properties( fmipp_backend PROPERTIES COMPILE_FLAGS "-DBACKEND_USE_PARENT_PID" )
   target_link_libraries( fmipp_backend ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} rt )
endif ()


# FMI 1.0: front end component library (pre-stage for compiling an FMU DLL according to FMI 1.0).
add_library( fmipp_frontend STATIC
  ${FMIPP_ROOT_DIR}/export/src/FMIComponentFrontEndBase.cpp
  ${FMIPP_ROOT_DIR}/export/src/FMIComponentFrontEnd.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCMasterLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMMaster.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMManager.cpp
  ${FMIPP_ROOT_DIR}/export/src/HelperFunctions.cpp
  ${FMIPP_ROOT_DIR}/export/src/ScalarVariable.cpp
  ${FMIPP_ROOT_DIR}/import/base/src/ModelDescription.cpp
  ${FMIPP_ROOT_DIR}/import/base/src/PathFromUrl.cpp )

# OS-specific dependencies here
if ( WIN32 )
   set_target_properties( fmipp_frontend PROPERTIES COMPILE_FLAGS "-DBUILD_FMI_DLL" )
elseif ( APPLE )
   set_target_properties( fmipp_frontend PROPERTIES COMPILE_FLAGS "-fPIC" )
else ()
   set_target_properties( fmipp_frontend PROPERTIES COMPILE_FLAGS "-fPIC" )
endif ()


# FMI 2.0: complete FMU DLL.
add_library( fmi2 SHARED
  ${FMIPP_ROOT_DIR}/export/functions/fmi_v2.0/fmi2Functions.cpp
  ${FMIPP_ROOT_DIR}/export/src/FMIComponentFrontEndBase.cpp
  ${FMIPP_ROOT_DIR}/export/src/FMIComponentFrontEnd.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/IPCMasterLogger.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMMaster.cpp
  ${FMIPP_ROOT_DIR}/export/src/SHMManager.cpp
  ${FMIPP_ROOT_DIR}/export/src/HelperFunctions.cpp
  ${FMIPP_ROOT_DIR}/export/src/ScalarVariable.cpp
  ${FMIPP_ROOT_DIR}/import/base/src/ModelDescription.cpp
  ${FMIPP_ROOT_DIR}/import/base/src/PathFromUrl.cpp )

# Define compiler flags.
target_compile_definitions( fmi2 PRIVATE
  FRONT_END_TYPE=FMIComponentFrontEnd
  FRONT_END_TYPE_INCLUDE="export/include/FMIComponentFrontEnd.h" )

# OS-specific dependencies here
if ( WIN32 )
  target_link_libraries( fmi2 Shlwapi ${Boost_LIBRARIES} )
elseif ( APPLE )
  set_target_properties( fmi2 PROPERTIES COMPILE_FLAGS "-fPIC" )
  target_link_libraries( fmi2 ${Boost_LIBRARIES} rt )
else ()
  set_target_properties( fmi2 PROPERTIES COMPILE_FLAGS "-fPIC" )
  target_link_libraries( fmi2 ${Boost_LIBRARIES} rt )
endif ()
