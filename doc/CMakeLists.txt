CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(SmartSoft_CD_API_Doxygen)

FIND_PACKAGE(Doxygen)

IF(DOXYGEN_FOUND)
# copy doxygen files to build/doc folder
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/SmartSoft-API-Reference.doxyfile.in ${PROJECT_BINARY_DIR}/SmartSoft-API-Reference.doxyfile @ONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/DoxygenLayout.xml ${PROJECT_BINARY_DIR}/DoxygenLayout.xml COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/mainpage.dox ${PROJECT_BINARY_DIR}/mainpage.dox COPYONLY)
# run doxygen in build/doc folder
ADD_CUSTOM_TARGET(doc ALL
	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/SmartSoft-API-Reference.doxyfile
	WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
	COMMENT "Generating SmartSoft Component-Developer API documentation with Doxygen" VERBATIM
)
ENDIF(DOXYGEN_FOUND)
