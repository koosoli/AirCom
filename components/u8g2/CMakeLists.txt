# CMakeLists.txt for the u8g2 core component

# This assumes that the u8g2 source files are located in the 'csrc' subdirectory.
# We use GLOB to automatically find all .c files.

file(GLOB U8G2_SRCS "csrc/*.c")

idf_component_register(SRCS ${U8G2_SRCS}
                    INCLUDE_DIRS "csrc")
