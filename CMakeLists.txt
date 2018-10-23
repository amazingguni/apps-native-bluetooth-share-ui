

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(bluetooth-share-ui C)

SET(SRCS ${CMAKE_SOURCE_DIR}/src/bt-share-ui-main.c
	${CMAKE_SOURCE_DIR}/src/bt-share-ui-ipc.c
	${CMAKE_SOURCE_DIR}/src/bt-share-ui-widget.c
	${CMAKE_SOURCE_DIR}/src/bt-share-ui-popup.c
	${CMAKE_SOURCE_DIR}/src/bt-share-ui-view.c )

SET(PREFIX ${CMAKE_INSTALL_PREFIX})

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	appcore-efl
	bundle
	dlog
	ecore
	edbus
	edje
	elementary
	efl-extension
	evas
	gobject-2.0
	aul
	capi-appfw-application
	capi-network-bluetooth
	bluetooth-share-api
	notification)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag} -g")
ENDFOREACH(flag)
#SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -finstrument-functions")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fPIC -Wall")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g ")
SET(CMAKE_C_FLAGS_RELEASE "-O2")
#This is flag for Gprof
#SET(CMAKE_C_FLAGS_RELEASE "-pg")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

ADD_DEFINITIONS("-DPREFIX=\"${CMAKE_INSTALL_PREFIX}/\"")
ADD_DEFINITIONS("-DSLP_DEBUG")

SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS})

INSTALL(TARGETS ${PROJECT_NAME} DESTINATION bin)

# install desktop file & icon
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
INSTALL(FILES ${CMAKE_BINARY_DIR}/data/org.tizen.bluetooth-share-ui.xml DESTINATION /usr/share/packages)
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/org.tizen.bluetooth-share-ui.png DESTINATION /usr/share/icons/default/small)

ADD_CUSTOM_TARGET(bt-share-layout.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/images
                ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/bt-share-layout.edc ${CMAKE_CURRENT_BINARY_DIR}/bt-share-layout.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/bt-share-layout.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} bt-share-layout.edj)

ADD_CUSTOM_TARGET(images.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/data/images
                ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/images.edc ${CMAKE_CURRENT_BINARY_DIR}/images.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/data/edc/images.edc
)
ADD_DEPENDENCIES(${PROJECT_NAME} images.edj)

# install edj file
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/bt-share-layout.edj DESTINATION ${PREFIX}/res/edje)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/images.edj DESTINATION ${PREFIX}/res/edje)

