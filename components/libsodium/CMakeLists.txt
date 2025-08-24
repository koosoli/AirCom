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
# Corrected paths to point inside the cloned repository
set(SODIUM_SRCS
    "src/libsodium/src/libsodium/sodium/core.c"
    "src/libsodium/src/libsodium/sodium/utils.c"
    "src/libsodium/src/libsodium/sodium/version.c"
    "src/libsodium/src/libsodium/randombytes/sysrandom/randombytes_sysrandom.c"
    "src/libsodium/src/libsodium/crypto_secretbox/xsalsa20poly1305/secretbox_xsalsa20poly1305.c"
    "src/libsodium/src/libsodium/crypto_stream/xsalsa20/stream_xsalsa20.c"
    "src/libsodium/src/libsodium/crypto_stream/salsa20/ref/salsa20_ref.c"
    "src/libsodium/src/libsodium/crypto_onetimeauth/poly1305/donna/poly1305_donna.c"
    "src/libsodium/src/libsodium/crypto_verify/verify.c"
)

# Register the component with the build system
idf_component_register(SRCS ${SODIUM_SRCS}
                        # The directory containing the public header files
                        INCLUDE_DIRS "src/libsodium/src/libsodium/include"
                        REQUIRES main)
