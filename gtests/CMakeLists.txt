CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

# we keep the SmartSoft API Version and the Tests Version in sync
PROJECT(SmartSoftTests VERSION 1.1)

FIND_PACKAGE(GTest QUIET NO_CMAKE_SYSTEM_PACKAGE_REGISTRY)

IF(GTest_FOUND)
  # the SmartSoft_CD_API's generic tests are implemented as header-only library and can be easily reused
  # to test different middleware mappings
  ADD_LIBRARY(${PROJECT_NAME} INTERFACE)

  TARGET_LINK_LIBRARIES(${PROJECT_NAME} INTERFACE SmartSoft_CD_API GTest::gtest pthread)
  # define include paths for the target
  TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>  # <prefix>/include
  )

  # set the export-name used in the ${PROJECT_NAME}Config.cmake.in and for exporting and installing the target
  SET(EXPORT_NAME ${PROJECT_NAME}Targets)

  # generate CMake package-config file
  CONFIGURE_FILE(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    @ONLY
  )
  
  # generate ConfigVersion file
  INCLUDE(CMakePackageConfigHelpers)
  write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    COMPATIBILITY AnyNewerVersion
  )
  
  FILE(GLOB HDRS ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/*.h)

  # install library and target-exports
  INSTALL(FILES ${HDRS} DESTINATION include/${PROJECT_NAME})
  INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/CommTestObjects DESTINATION include/${PROJECT_NAME})
  # install target and package-config files
  INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${EXPORT_NAME})
  INSTALL(EXPORT ${EXPORT_NAME} DESTINATION modules)
  INSTALL(FILES ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake DESTINATION modules)
  INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake" DESTINATION modules)

  ADD_SUBDIRECTORY(FakeObjectsImpl)
  ADD_SUBDIRECTORY(FakeObjectsTests)
  
ENDIF(GTest_FOUND)