CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

PROJECT( "RapaDirectInput" )

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/cmake" )

IF( CMAKE_SYSTEM_NAME MATCHES "Windows" )
	
	IF( MSVC )
		INCLUDE( RapaConfigureVisualStudio )
	ENDIF()

	INCLUDE( RapaFindDirectInput )
	IF( DIRECTINPUT_FOUND )
		
		INCLUDE_DIRECTORIES( ${DirectInput_INCLUDE_DIR} )
		INCLUDE_DIRECTORIES( include )
						
		SET	( 	HEADERS
				include/RDICommon.h
				include/RDITime.h
				include/RDIObjectInstance.h
				include/RDIObject.h
				include/RDIButton.h
				include/RDIAxis.h
				include/RDIPOV.h
				include/RDIDeviceInstance.h
				include/RDIDevice.h
				include/RDIDeviceEnumerationTrigger.h
				include/RDIDeviceManager.h
			)
		SET	(	SOURCES
				src/RDICommon.cpp
				src/RDITime.cpp
				src/RDIObjectInstance.cpp
				src/RDIObject.cpp
				src/RDIButton.cpp
				src/RDIAxis.cpp
				src/RDIPOV.cpp
				src/RDIDeviceInstance.cpp
				src/RDIDevice.cpp
				src/RDIDeviceEnumerationTrigger.cpp
				src/RDIDeviceManager.cpp
			)
		SOURCE_GROUP("" FILES ${HEADERS} ${SOURCES} )		# Avoid "Header Files" and "Source Files" virtual folders in VisualStudio

		SET(CMAKE_DEBUG_POSTFIX "d")
		ADD_LIBRARY( ${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} )
		TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${DirectInput_LIBRARIES} ) 
		
		#
		# Install
		#
		SET( EXTRA_SYSTEM_INCLUDE_DIRS ${DirectInput_INCLUDE_DIR} )
		INSTALL(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets
				LIBRARY DESTINATION lib
				ARCHIVE DESTINATION lib
				RUNTIME DESTINATION bin )
				#INCLUDES DESTINATION include )		# If uncommented, the ${PROJECT_NAME} target contains INCLUDE_DIRECTORIES information. Importing the target automatically adds this directory to the INCLUDE_DIRECTORIES.
		SET( TARGET_NAMESPACE Rapa:: )
		INSTALL( FILES ${HEADERS} DESTINATION include COMPONENT Devel )		
		EXPORT( EXPORT ${PROJECT_NAME}Targets FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Targets.cmake" NAMESPACE ${TARGET_NAMESPACE} )
		CONFIGURE_FILE( cmake/${PROJECT_NAME}Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" @ONLY )
		SET( ConfigPackageLocation lib/cmake/${PROJECT_NAME} )
		INSTALL(EXPORT ${PROJECT_NAME}Targets
				FILE ${PROJECT_NAME}Targets.cmake
				NAMESPACE ${TARGET_NAMESPACE}
				DESTINATION ${ConfigPackageLocation} )
		INSTALL( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake" DESTINATION ${ConfigPackageLocation} COMPONENT Devel )
	
		
		ADD_SUBDIRECTORY( samples )
		
	ELSE()
		MESSAGE("DirectInput not found")
	ENDIF()
ELSE()
	MESSAGE("${PROJECT_NAME} is Windows only")
ENDIF()
