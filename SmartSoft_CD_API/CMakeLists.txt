CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# this is the SmartSoft Component-Developer (short CD) C++ API
PROJECT(SmartSoft_CD_API VERSION 0.8)

# create the API as an Interface Library (headers only)
ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

# define include paths for the target
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>  # <prefix>/include/SmartSoft_CD_API
)

# install library and target-exports
INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Target DESTINATION lib)
EXPORT(EXPORT ${PROJECT_NAME}Target)
INSTALL(EXPORT ${PROJECT_NAME}Target DESTINATION modules)
