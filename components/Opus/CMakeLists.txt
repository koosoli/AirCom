# CMakeLists.txt for the pre-compiled Opus library component

# Note to the user:
# This component is a placeholder for the pre-compiled Opus codec library.
# To make this work, you must:
#
# 1. Obtain a pre-compiled version of the Opus library for the ESP32
#    (a file named something like 'libopus.a').
# 2. Place the 'libopus.a' file inside the 'components/Opus/lib/' directory.
# 3. Place the corresponding Opus header files (opus.h, opus_defines.h, etc.)
#    inside the 'components/Opus/include/' directory, replacing the placeholder
#    header.

# Register the component with the build system
idf_component_register(
    # Source files for this component (wrapper implementation)
    SRCS "src/audio_codec.c"
    # The directory containing the public header files for this component
    INCLUDE_DIRS "include"
    # This component requires the 'main' component (or others if needed)
    REQUIRES main
)

# Specify the path to the pre-compiled library
set(OPUS_LIB_PATH ${CMAKE_CURRENT_LIST_DIR}/lib)

# Add the pre-compiled library to the build
target_link_libraries(${COMPONENT_TARGET} INTERFACE
    -L${OPUS_LIB_PATH}
    -lopus
)
