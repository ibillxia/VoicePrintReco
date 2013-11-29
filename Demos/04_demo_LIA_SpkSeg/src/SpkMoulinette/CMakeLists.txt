cmake_minimum_required(VERSION 2.6)
project(SpkMoulinette)

set(CMAKE_MODULE_PATH .)
find_package(Alize REQUIRED)
find_package(LIATools REQUIRED)

# The version number.
set(SpkMoulinette_VERSION_MAJOR 0)
set(SpkMoulinette_VERSION_MINOR 7)

# The final name of the application
set(SpkMoulinette_APP_NAME "SpkMoulinette")

configure_file("include/SpkMoulinetteMain.h.in" "include/SpkMoulinetteMain.h")

# List of source files
file(
        GLOB_RECURSE
        source_files
        src/*
)

include_directories(include)

add_executable(${SpkMoulinette_APP_NAME} ${source_files})
target_link_libraries(${SpkMoulinette_APP_NAME} ${LIATools_LIBS}  ${Alize_LIBS})

if(EXISTS "${INSTALL_DIR}")
	install(TARGETS ${SpkMoulinette_APP_NAME}
		RUNTIME DESTINATION ${INSTALL_DIR})
endif(EXISTS "${INSTALL_DIR}")
