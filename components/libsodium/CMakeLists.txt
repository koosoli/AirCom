# CMakeLists.txt for the libsodium component
#
# This is a minimal libsodium-compatible implementation specifically designed
# for the AirCom project's cryptographic needs. It provides the essential
# functions required by main/crypto.cpp:
# - sodium_init()
# - randombytes_buf()
# - crypto_secretbox_easy()
# - crypto_secretbox_open_easy()
# - Associated constants (KEYBYTES, NONCEBYTES, MACBYTES)

# Source files for the minimal libsodium implementation
set(SODIUM_SRCS
    "src/libsodium/sodium/core.c"
)

# Register the component with the build system
idf_component_register(SRCS ${SODIUM_SRCS}
                        # The directory containing the public header files
                        INCLUDE_DIRS "src/libsodium/include"
                        REQUIRES main)
