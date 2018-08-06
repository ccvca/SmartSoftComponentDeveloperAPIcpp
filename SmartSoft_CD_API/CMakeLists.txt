CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# this is the SmartSoft Component-Developer (short CD) C++ API
PROJECT(SmartSoft_CD_API VERSION 1.0)

# collect header files
FILE(GLOB HDRS *.h)

# create the API as an Interface Library (headers only)
ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

# define include paths for the target
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>  # <prefix>/include/SmartSoft_CD_API
)

# use the auto-type compile feature (available since C++11)
TARGET_COMPILE_FEATURES(${PROJECT_NAME} INTERFACE cxx_auto_type)

# generate ConfigVersion file
INCLUDE(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY AnyNewerVersion
)

# install library and target-exports
INSTALL(FILES ${HDRS} DESTINATION include/${PROJECT_NAME})
# install target and package-config files
INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)
INSTALL(EXPORT ${PROJECT_NAME}Targets DESTINATION modules)
INSTALL(FILES ${PROJECT_NAME}Config.cmake DESTINATION modules)
INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION modules)
