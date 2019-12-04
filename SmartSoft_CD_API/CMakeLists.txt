CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# this is the SmartSoft Component-Developer (short CD) C++ API
PROJECT(SmartSoft_CD_API VERSION 1.1)

# collect header files
FILE(GLOB HDRS *.h)

# create the API as an Interface Library (headers only)
ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

# define include paths for the target
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>  # <prefix>/include/SmartSoft_CD_API
  $<INSTALL_INTERFACE:include>
)

# use the auto-type compile feature (available since C++11)
TARGET_COMPILE_FEATURES(${PROJECT_NAME} INTERFACE cxx_auto_type)

# set the export-name used in the ${PROJECT_NAME}Config.cmake.in and for exporting and installing the target
SET(EXPORT_NAME ${PROJECT_NAME}Targets)

# generate CMake package-config file
CONFIGURE_FILE(
  ${CMAKE_SOURCE_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  @ONLY
)

# generate ConfigVersion file
INCLUDE(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)

include(GNUInstallDirs)
set(INSTALL_CMAKE_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
# install library and target-exports
INSTALL(FILES ${HDRS} DESTINATION include/${PROJECT_NAME})
# install target and package-config files
INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${EXPORT_NAME})
INSTALL(EXPORT ${EXPORT_NAME} DESTINATION ${INSTALL_CMAKE_DESTINATION})
INSTALL(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake DESTINATION ${INSTALL_CMAKE_DESTINATION})
INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION ${INSTALL_CMAKE_DESTINATION})
