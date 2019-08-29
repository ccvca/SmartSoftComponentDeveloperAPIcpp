CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(SmartSoftFramework)

# default installation prefix is set to the path given in the SMART_ROOT_ACE environment variable
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  # SET(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR} CACHE PATH "Set default SmartSoft install prefix" FORCE)
  SET(CMAKE_INSTALL_PREFIX $ENV{SMART_ROOT_ACE} CACHE PATH "Set default SmartSoft install prefix" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# create uninstall configuration file
CONFIGURE_FILE(
    "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    @ONLY
)

# create uninstall target
ADD_CUSTOM_TARGET(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

# build Doxygen documentation
ADD_SUBDIRECTORY(doc)

# build target for the SmartSoft Component-Developer API
ADD_SUBDIRECTORY(SmartSoft_CD_API)

