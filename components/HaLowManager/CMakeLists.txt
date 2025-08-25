# CMakeLists.txt for the HaLowManager component

# Register the component with the build system
idf_component_register(SRCS "HaLowMeshManager.cpp"
                           "src/mm_iot_sdk.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES main)
