# First, this checks for Hydrogen. If it fails to find Hydrogen, it
# searches for Elemental. If it finds Elemental, it sets
# HYDROGEN_LIBRARIES to point to Elemental. If neither is found, an
# error is thrown.
#
# FIXME: This file exists to hide the implementation detail of
# Hydrogen vs. Elemental. Once we decide to move fully over to
# Hydrogen, this file is no longer necessary as it's just one
# find_package() line.

find_package(Hydrogen NO_MODULE
  HINTS ${Hydrogen_DIR} ${HYDROGEN_DIR} $ENV{Hydrogen_DIR} $ENV{HYDROGEN_DIR}
  PATH_SUFFIXES lib/cmake/hydrogen
  NO_DEFUALT_PATH)
find_package(Hydrogen NO_MODULE)

if (Hydrogen_FOUND)
  message(STATUS "Found Hydrogen: ${Hydrogen_DIR}")
  set(LBANN_HAS_HYDROGEN TRUE)
else ()
  set(LBANN_HAS_HYDROGEN FALSE)

  find_package(Elemental NO_MODULE
    PATH_SUFFIXES lib/cmake/elemental)

  if (Elemental_FOUND)
    set(HYDROGEN_LIBRARIES "${Elemental_LIBRARIES}")
    message(STATUS "Found Elemental: ${Elemental_DIR}")

    if (TARGET El)
      set_property(TARGET El PROPERTY
        INTERFACE_INCLUDE_DIRECTORIES ${Elemental_INCLUDE_DIRS})
    endif ()

    set(LBANN_HAS_ELEMENTAL TRUE)
  else ()
    message(FATAL_ERROR "Neither Hydrogen nor Elemental was found! "
      "Try setting Hydrogen_DIR or Elemental_DIR and try again!")

    set(LBANN_HAS_ELEMENTAL FALSE)
  endif (Elemental_FOUND)
endif (Hydrogen_FOUND)

if (NOT LBANN_HAS_HYDROGEN AND NOT LBANN_HAS_ELEMENTAL)
  message(FATAL_ERROR "LBANN requires Hydrogen or Elemental.")
endif ()
