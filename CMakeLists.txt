CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

PROJECT(SmartSoftFramework)

# build Doxygen documentation
ADD_SUBDIRECTORY(doc)

# build target for the SmartSoft Component-Developer API
ADD_SUBDIRECTORY(SmartSoft_CD_API)
