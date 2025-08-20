# CMakeLists.txt for the libsodium component
#
# NOTE: This is a template file. To use it, you must first add the
# full source code of the libsodium library into this directory.
# A common way to do this is with `git submodule add`.
#
# This file assumes the libsodium source code is in a subdirectory
# named 'src/libsodium'. You may need to adjust the paths based on
# the actual structure of the library.

# This command finds all the .c source files within the libsodium source directory.
# It excludes files related to testing, specific architectures not relevant to ESP32, etc.
# This list would need to be carefully curated for a real build.
file(GLOB_RECURSE SODIUM_SRCS
    "src/libsodium/crypto_aead/aes256gcm/*.c"
    "src/libsodium/crypto_aead/chacha20poly1305/*.c"
    "src/libsodium/crypto_auth/*.c"
    "src/libsodium/crypto_box/*.c"
    "src/libsodium/crypto_core/hchacha20/*.c"
    "src/libsodium/crypto_core/hsalsa20/*.c"
    "src/libsodium/crypto_generichash/blake2b/*.c"
    "src/libsodium/crypto_hash/sha256/*.c"
    "src/libsodium/crypto_hash/sha512/*.c"
    "src/libsodium/crypto_kx/*.c"
    "src/libsodium/crypto_onetimeauth/*.c"
    "src/libsodium/crypto_pwhash/*.c"
    "src/libsodium/crypto_scalarmult/curve25519/*.c"
    "src/libsodium/crypto_secretbox/*.c"
    "src/libsodium/crypto_shorthash/siphash24/*.c"
    "src/libsodium/crypto_sign/ed25519/*.c"
    "src/libsodium/crypto_stream/chacha20/*.c"
    "src/libsodium/crypto_stream/salsa20/*.c"
    "src/libsodium/crypto_verify/*.c"
    "src/libsodium/randombytes/*.c"
    "src/libsodium/sodium/*.c"
)

# Register the component with the build system
idf_component_register(SRCS ${SODIUM_SRCS}
                       # The directory containing the public header files
                       INCLUDE_DIRS "src/libsodium/include"
                       REQUIRES main)
