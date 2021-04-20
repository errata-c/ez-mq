cmake_minimum_required(VERSION 3.15)

project(EZ_MQ
VERSION 0.2.1
DESCRIPTION "Provides easy interface to zeromq, specialized for inprocess messaging."
LANGUAGES C CXX)

include(GNUInstallDirs)
include("cmake/ez-cmake/install_package.cmake")

option(BUILD_TESTS "Build the test executables" ON)
option(SEPARATE_DIRECTORY "Install the package into its own directory" ON)
set(CONFIG_DIR "${CMAKE_INSTALL_DATAROOTDIR}/ez-mq" CACHE STRING "The relative directory to install package config files.")



find_package(ZeroMQ CONFIG REQUIRED)
find_package(ez-serialize CONFIG REQUIRED)


add_library(ez-mq STATIC
"src/MQCore.cpp"
)
target_link_libraries(ez-mq PUBLIC libzmq ez::serialize)
target_include_directories(ez-mq PUBLIC 
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>" 
	"$<INSTALL_INTERFACE:include>"
)

set_target_properties(ez-mq PROPERTIES EXPORT_NAME "mq")

add_library(ez::mq ALIAS ez-mq)



if(BUILD_TESTS)
	add_subdirectory("test")
endif()

if(SEPARATE_DIRECTORY)
	set(CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/ez-mq)
endif()



install(
	DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/"
	DESTINATION "include"
)

install(TARGETS ez-mq
	EXPORT ez-mq-targets
	LIBRARY DESTINATION "lib/$<CONFIG>"
	ARCHIVE DESTINATION "lib/$<CONFIG>"
	RUNTIME DESTINATION "bin/$<CONFIG>"
)

install_package(
	NAME "ez-mq"
	NAMESPACE "ez::"
	EXPORT "ez-mq-targets"
	DESTINATION "${CONFIG_DIR}"
	COMPATIBILITY SameMajorVersion
	PRECONFIG "cmake/preconfig.cmake"
	VERSION ${EZ_MQ_VERSION}
)