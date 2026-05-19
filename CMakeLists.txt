#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.16...3.27)
project(Radpharm)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# See the documentation for a guide on how to enable/disable specific components
#
find_package(Geant4 REQUIRED ui_all vis_all)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, use our local headers, and link it to the Geant4 libraries
#
add_executable(Radpharm Radpharm.cc ${sources} ${headers})
target_include_directories(Radpharm PRIVATE include)
target_link_libraries(Radpharm PRIVATE ${Geant4_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build B1. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(Radpharm_SCRIPTS
  init_vis.mac
  run.mac
  vis.mac
  )

foreach(_script ${Radpharm_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

# Collect all .csv and .C files dynamically using GLOB
file(GLOB CSV_FILES "${PROJECT_SOURCE_DIR}/*.csv")
file(GLOB PY_FILES "${PROJECT_SOURCE_DIR}/*.py")
file(GLOB C_FILES "${PROJECT_SOURCE_DIR}/*.C")
# Copy all discovered .csv and .C files to the build directory
foreach(_file ${CSV_FILES} ${C_FILES} ${PY_FILES})
  # Get only the filename component (e.g., "data.csv" instead of the full path)
  get_filename_component(_filename_only ${_file} NAME)
  
  configure_file(
    ${_file}
    ${PROJECT_BINARY_DIR}/${_filename_only}
    COPYONLY
  )
endforeach()
