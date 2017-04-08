INCLUDE(CMakeForceCompiler)

# TOOLCHAIN EXTENSION
IF(WIN32)
    SET(TOOLCHAIN_EXT ".exe")
ELSE()
    SET(TOOLCHAIN_EXT "")
ENDIF()

# EXECUTABLE EXTENSION
SET (CMAKE_EXECUTABLE_SUFFIX ".elf")

# CMAKE_BUILD_TYPE
IF(NOT CMAKE_BUILD_TYPE MATCHES Debug)
    SET (CMAKE_BUILD_TYPE Release)
ENDIF()

SET(TOOLCHAIN_DIR /usr/local/resources/compilers)

STRING(REGEX REPLACE "\\\\" "/" TOOLCHAIN_DIR "${TOOLCHAIN_DIR}")
IF(NOT TOOLCHAIN_DIR)
    MESSAGE(FATAL_ERROR "***Please set ARMGCC_DIR in envionment variables or use -g***")
ENDIF()

MESSAGE(STATUS "TOOLCHAIN_DIR: " ${TOOLCHAIN_DIR})

SET(LLVM_VERSION  clang-llvm-3.9.0)

SET(CMAKE_C_COMPILER   ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/clang)
MESSAGE(STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER})
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/clang++)
MESSAGE(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})

SET(CMAKE_AR      ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/llvm-ar)
SET(CMAKE_LINKER  ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/llvm-link)
SET(CMAKE_NM      ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/llvm-nm)
SET(CMAKE_OBJDUMP ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/llvm-objdump)
SET(CMAKE_RANLIB  ${TOOLCHAIN_DIR}/${LLVM_VERSION}/bin/llvm-ranlib)

SET(ARMGCC_DIR "$ENV{ARMGCC_DIR}")
set(CMAKE_CLANG_FLAGS "-v -target arm-none-eabi --sysroot=${ARMGCC_DIR}/arm-none-eabi -isystem ${ARMGCC_DIR}/arm-none-eabi/include/")
MESSAGE(STATUS "CMAKE Clang Flags: " ${CMAKE_CLANG_FLAGS})

# ------------- DEBUG BUILD FLAGS (no optimizations) ------------- #

SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_CLANG_FLAGS} ${CMAKE_C_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CLANG_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -O0 -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "${CMAKE_CLANG_FLAGS} ${CMAKE_ASM_FLAGS_DEBUG} -g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} " CACHE INTERNAL "linker flags debug")


# ------------- RELEASE BUILD FLAGS (no optimizations) ------------- #

SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_CLANG_FLAGS} ${CMAKE_C_FLAGS_RELEASE} -O2 " CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CLANG_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2 " CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "${CMAKE_CLANG_FLAGS} ${CMAKE_ASM_FLAGS_RELEASE}" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} " CACHE INTERNAL "linker flags release")

SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR} ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

IF(CMAKE_BUILD_TYPE MATCHES Release)
    SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/build/release)
    SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/build/release)
ELSEIF(CMAKE_BUILD_TYPE MATCHES Debug)
    SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/build/debug)
    SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/build/debug)
ENDIF()

MESSAGE(STATUS "BUILD_TYPE: " ${CMAKE_BUILD_TYPE})