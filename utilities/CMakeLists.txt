# Specify the version used
cmake_minimum_required(VERSION 3.0.2)

# Add source files
add_sources(
    astroConstants.h    astroFunctions.c    astroFunctions.h
    enumConversions.h    linearAlgebra.c    linearAlgebra.h    orbitalMotion.c    orbitalMotion.h    projectMacros.h    rigidBodyKinematics.c    rigidBodyKinematics.h    shuntingYard.c    shuntingYard.h    stack.c    stack.h    strMap.c    strMap.h
)
