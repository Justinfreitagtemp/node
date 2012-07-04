
#ifndef BOTAN_BUILD_CONFIG_H__
#define BOTAN_BUILD_CONFIG_H__

/*
* This file was automatically generated Thu Jun 28 16:36:40 2012 UTC by
* justin@ubuntu running './configure.py --no-autoload --enable-modules=rsa,eme1,pbkdf2,md5,auto_rng --disable-shared'
*
* Target
*  - Compiler: g++ -m64 -O3 -finline-functions 
*  - Arch: x86_64/x86_64
*  - OS: linux
*/

#define BOTAN_VERSION_MAJOR 1
#define BOTAN_VERSION_MINOR 10
#define BOTAN_VERSION_PATCH 2
#define BOTAN_VERSION_DATESTAMP 0

#define BOTAN_VERSION_VC_REVISION "mtn:2bf8ad2c501213efb4cf9b219330b87666988e91"

#define BOTAN_DISTRIBUTION_INFO "unspecified"

#ifndef BOTAN_DLL
  #define BOTAN_DLL 
#endif

/* Chunk sizes */
#define BOTAN_DEFAULT_BUFFER_SIZE 4096
#define BOTAN_MEM_POOL_CHUNK_SIZE 64*1024
#define BOTAN_BLOCK_CIPHER_PAR_MULT 4

/* BigInt toggles */
#define BOTAN_MP_WORD_BITS 64
#define BOTAN_KARAT_MUL_THRESHOLD 32
#define BOTAN_KARAT_SQR_THRESHOLD 32

/* PK key consistency checking toggles */
#define BOTAN_PUBLIC_KEY_STRONG_CHECKS_ON_LOAD 1
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_LOAD 0
#define BOTAN_PRIVATE_KEY_STRONG_CHECKS_ON_GENERATE 1

/* Should we use GCC-style inline assembler? */
#if !defined(BOTAN_USE_GCC_INLINE_ASM) && defined(__GNUG__)
  #define BOTAN_USE_GCC_INLINE_ASM 1
#endif

#if !defined(BOTAN_USE_GCC_INLINE_ASM)
  #define BOTAN_USE_GCC_INLINE_ASM 0
#endif

#ifdef __GNUC__
  #define BOTAN_GCC_VERSION \
     (__GNUC__ * 100 + __GNUC_MINOR__ * 10 + __GNUC_PATCHLEVEL__)
#else
  #define BOTAN_GCC_VERSION 0
#endif

/* Target identification and feature test macros */
#define BOTAN_TARGET_ARCH_IS_X86_64
#define BOTAN_TARGET_CPU_HAS_SSE2
#define BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN
#define BOTAN_TARGET_CPU_IS_X86_FAMILY
#define BOTAN_TARGET_UNALIGNED_MEMORY_ACCESS_OK 1

#if defined(BOTAN_TARGET_CPU_IS_LITTLE_ENDIAN) || \
    defined(BOTAN_TARGET_CPU_IS_BIG_ENDIAN)
  #define BOTAN_TARGET_CPU_HAS_KNOWN_ENDIANNESS
#endif

#define BOTAN_BUILD_COMPILER_IS_GCC
#define BOTAN_USE_STD_TR1

#if defined(_MSC_VER)
  // 4250: inherits via dominance (diamond inheritence issue)
  // 4251: needs DLL interface (STL DLL exports)
  #pragma warning(disable: 4250 4251)
#endif

/*
* Compile-time deprecatation warnings
*/
#if !defined(BOTAN_NO_DEPRECATED_WARNINGS)

  #if defined(__clang__)
    #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))

  #elif defined(_MSC_VER)
    #define BOTAN_DEPRECATED(msg) __declspec(deprecated(msg))

  #elif defined(__GNUG__)

    #if BOTAN_GCC_VERSION >= 450 && !defined(__INTEL_COMPILER)
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated(msg)))
    #else
      #define BOTAN_DEPRECATED(msg) __attribute__ ((deprecated))
    #endif

  #endif

#endif

#if !defined(BOTAN_DEPRECATED)
  #define BOTAN_DEPRECATED(msg)
#endif

/*
* Module availability definitions
*/
#define BOTAN_HAS_AES
#define BOTAN_HAS_ALGORITHM_FACTORY
#define BOTAN_HAS_ASN1
#define BOTAN_HAS_AUTO_SEEDING_RNG
#define BOTAN_HAS_BASE64_CODEC
#define BOTAN_HAS_BIGINT
#define BOTAN_HAS_BIGINT_MATH
#define BOTAN_HAS_BIGINT_MP
#define BOTAN_HAS_BLOCK_CIPHER
#define BOTAN_HAS_CBC
#define BOTAN_HAS_CIPHER_MODE_PADDING
#define BOTAN_HAS_CODEC_FILTERS
#define BOTAN_HAS_CORE_ENGINE
#define BOTAN_HAS_CTR_BE
#define BOTAN_HAS_EAX
#define BOTAN_HAS_ENTROPY_SRC_DEV_RANDOM
#define BOTAN_HAS_ENTROPY_SRC_EGD
#define BOTAN_HAS_ENTROPY_SRC_FTW
#define BOTAN_HAS_ENTROPY_SRC_HIGH_RESOLUTION_TIMER
#define BOTAN_HAS_ENTROPY_SRC_RDRAND
#define BOTAN_HAS_ENTROPY_SRC_UNIX
#define BOTAN_HAS_EME1
#define BOTAN_HAS_EMSA4
#define BOTAN_HAS_ENGINES
#define BOTAN_HAS_FILTERS
#define BOTAN_HAS_HEX_CODEC
#define BOTAN_HAS_HMAC
#define BOTAN_HAS_HMAC_RNG
#define BOTAN_HAS_IF_PUBLIC_KEY_FAMILY
#define BOTAN_HAS_KDF_BASE
#define BOTAN_HAS_KEYPAIR_TESTING
#define BOTAN_HAS_LIBSTATE_MODULE
#define BOTAN_HAS_MD5
#define BOTAN_HAS_MDX_HASH_FUNCTION
#define BOTAN_HAS_MGF1
#define BOTAN_HAS_MUTEX_NOOP
#define BOTAN_HAS_MUTEX_PTHREAD
#define BOTAN_HAS_MUTEX_WRAPPERS
#define BOTAN_HAS_OID_LOOKUP
#define BOTAN_HAS_PASSWORD_BASED_ENCRYPTION
#define BOTAN_HAS_PBKDF2
#define BOTAN_HAS_PEM_CODEC
#define BOTAN_HAS_PK_PADDING
#define BOTAN_HAS_PUBLIC_KEY_CRYPTO
#define BOTAN_HAS_RSA
#define BOTAN_HAS_SHA1
#define BOTAN_HAS_SHA1_SSE2
#define BOTAN_HAS_SHA1_X86_64
#define BOTAN_HAS_SHA2_32
#define BOTAN_HAS_SHA2_64
#define BOTAN_HAS_STREAM_CIPHER
#define BOTAN_HAS_UTIL_FUNCTIONS

/*
* Local configuration options (if any) follow
*/


#endif
