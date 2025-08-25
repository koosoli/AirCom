# AirCom Protobuf Component CMakeLists.txt

idf_component_register(
    SRCS
        "AirCom.pb-c.cpp"

    INCLUDE_DIRS
        "."

    REQUIRES
        protobuf-c
)

# Custom target to generate protobuf files
add_custom_target(generate_protobuf
    COMMAND protoc --c_out=${CMAKE_CURRENT_SOURCE_DIR} --proto_path=${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/AirCom.proto
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/AirCom.pb-c.c ${CMAKE_CURRENT_SOURCE_DIR}/AirCom.pb-c.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/AirCom.proto
    COMMENT "Generating protobuf C files"
)

# Make sure protobuf files are generated before building
add_dependencies(${COMPONENT_LIB} generate_protobuf)