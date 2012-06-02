cmake_minimum_required ( VERSION 2.8 )
project ( libQtLogger )
set ( TARGET_NAME   "qtLogger" )                        # actual executable name

find_package ( Qt4 REQUIRED )
INCLUDE(${QT_USE_FILE})

# define project sources and includes directories
set ( SOURCES_DIR   "${CMAKE_CURRENT_SOURCE_DIR}/src/" )
set ( INCLUDES_DIR  "${CMAKE_CURRENT_SOURCE_DIR}/inc/" )

# define includes search path
include_directories ( ${INCLUDES_DIR}
                      ${QT_INCLUDES}
                    )
# define sources search path
aux_source_directory ( ${SOURCES_DIR} SOURCES )
# define libraries search path
link_directories ( ${QT_LIBRARY_DIR} )

# set default build type
if ( NOT CMAKE_BUILD_TYPE )
    message ( STATUS "${PROJECT_NAME}: set default build type" )

    # set ( CMAKE_BUILD_TYPE Release )
    # while in intensive development use debug build type
    set ( CMAKE_BUILD_TYPE Debug )
endif ()

# set common compiler flags
set ( CFLAGS    "-Wall -Werror" )
set ( CXXFLAGS  "-Wall -Werror" )
set ( DEFINES   "-DLIBQTLOGGER_LIBRARY ${QT_DEFINITIONS} -DQT_SHARED" )

# set compiler flags for build type
if ( CMAKE_BUILD_TYPE STREQUAL "Release" )              # Release
    message ( STATUS "${PROJECT_NAME}: build release" )

    set ( DEFINES   "${DEFINES} -D_RELEASE -DQT_NO_DEBUG" )
endif()
if ( CMAKE_BUILD_TYPE STREQUAL "Debug" )                # Debug
    message ( STATUS "${PROJECT_NAME}: build debug" )

    set ( CFLAGS    "${CFLAGS} -O0" )
    set ( CXXFLAGS  "${CXXFLAGS} -O0" )
    set ( DEFINES   "${DEFINES} -D_DEBUG" )
endif ()

# apply flags
set ( CMAKE_C_FLAGS     "${CMAKE_C_FLAGS} ${CFLAGS}" )
set ( CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS} ${CXXFLAGS}" )
add_definitions ( ${DEFINES} )

# show flags
message ( STATUS "${PROJECT_NAME}: c flags: ${CMAKE_C_FLAGS}" )
message ( STATUS "${PROJECT_NAME}: cxx flags: ${CMAKE_CXX_FLAGS}" )
message ( STATUS "${PROJECT_NAME}: defines: ${DEFINES}" )

add_library ( ${TARGET_NAME} SHARED ${SOURCES} )
target_link_libraries ( ${TARGET_NAME} ${QT_LIBRARIES} )
