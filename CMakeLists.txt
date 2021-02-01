# Build system for the gint unikernel

cmake_minimum_required(VERSION 3.16)
project(Gint VERSION 2.3.0 LANGUAGES C ASM)

include(GitVersionNumber)
include(Fxconv)

option(GINT_USER_VRAM "Put all VRAMs into the user stack (fx-CG 50 only)")
option(GINT_STATIC_GRAY "Use static memory instead of malloc for gray buffers (fx-9860G only)")

# Generate <gint/config.h> with commit hash, version name and options
git_version_number(SHORT_HASH "GINT_GIT_HASH" TAG_RELATIVE "GINT_GIT_VERSION")
configure_file(include/gint/config.h.in include/gint/config.h)

set(SOURCES_COMMON
  src/cpg/cpg.c
  src/intc/intc.c
  src/kernel/cpu.s
  src/kernel/exch.c
  src/kernel/exch.s
  src/kernel/hardware.c
  src/kernel/inth.S
  src/kernel/kernel.c
  src/kernel/osmenu.c
  src/kernel/start.c
  src/kernel/syscalls.S
  src/kernel/tlbh.S
  src/keysc/getkey.c
  src/keysc/iokbd.c
  src/keysc/keycodes.c
  src/keysc/keysc.c
  src/kprint/kprint.c
  src/mmu/mmu.c
  src/render/dhline.c
  src/render/dimage.c
  src/render/dline.c
  src/render/dprint.c
  src/render/drect_border.c
  src/render/dtext.c
  src/render/dvline.c
  src/render/topti.c
  src/rtc/inth.s
  src/rtc/rtc.c
  src/rtc/rtc_ticks.c
  src/spu/spu.c
  src/std/memcmp.s
  src/std/memcpy.s
  src/std/memmove.s
  src/std/memset.s
  src/std/print.c
  src/std/string.c
  src/tmu/inth-etmu.s
  src/tmu/inth-tmu.s
  src/tmu/sleep.c
  src/tmu/tmu.c
  src/3rdparty/tinymt32/rand.c
  src/3rdparty/tinymt32/tinymt32.c
)
set(SOURCES_FX
  src/gray/engine.c
  src/gray/gclear.c
  src/gray/gint_gline.c
  src/gray/gpixel.c
  src/gray/grect.c
  src/gray/gsubimage.c
  src/gray/gtext.c
  src/render-fx/bopti-asm-gray-scsp.s
  src/render-fx/bopti-asm-gray.s
  src/render-fx/bopti-asm-mono-scsp.s
  src/render-fx/bopti-asm.s
  src/render-fx/bopti.c
  src/render-fx/dclear.c
  src/render-fx/dpixel.c
  src/render-fx/drect.c
  src/render-fx/dsubimage.c
  src/render-fx/dupdate.c
  src/render-fx/gint_dline.c
  src/render-fx/masks.c
  src/render-fx/topti-asm.s
  src/render-fx/topti.c
  src/t6k11/t6k11.c
)
set(SOURCES_CG
  src/dma/dma.c
  src/dma/inth.s
  src/dma/memcpy.c
  src/dma/memset.c
  src/r61524/r61524.c
  src/render-cg/bopti-asm.s
  src/render-cg/bopti.c
  src/render-cg/dclear.c
  src/render-cg/dpixel.c
  src/render-cg/drect.c
  src/render-cg/dsubimage.c
  src/render-cg/dupdate.c
  src/render-cg/dvram.c
  src/render-cg/gint_dline.c
  src/render-cg/topti-asm.s
  src/render-cg/topti.c
)

set(ASSETS_FX src/font5x7.png)
set(ASSETS_CG src/font8x9.png)
fxconv_declare_assets(${ASSETS_FX} ${ASSETS_CG})

include_directories("${PROJECT_SOURCE_DIR}/include" "${PROJECT_BINARY_DIR}/include")
add_compile_options(-Wall -Wextra -std=c11 -Os -fstrict-volatile-bitfields)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fx9860G)
  add_compile_definitions(FX9860G)
  add_library(gint-fx STATIC ${SOURCES_COMMON} ${SOURCES_FX} ${ASSETS_FX})
  set(NAME "gint-fx")
  set(LINKER_SCRIPT "fx9860g.ld")
endif()

if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  add_compile_definitions(FXCG50)
  add_library(gint-cg STATIC ${SOURCES_COMMON} ${SOURCES_CG} ${ASSETS_CG})
  set(NAME "gint-cg")
  set(LINKER_SCRIPT "fxcg50.ld")
endif()

set_target_properties("${NAME}" PROPERTIES OUTPUT_NAME "${NAME}")

# Library file
install(TARGETS "${NAME}" DESTINATION "${FXSDK_COMPILER_INSTALL}")
# Linker script
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}"
  DESTINATION "${FXSDK_COMPILER_INSTALL}")
# Headers
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include"
  DESTINATION "${FXSDK_COMPILER_INSTALL}"
  FILES_MATCHING PATTERN "*.h")
# Auto-generated config header
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/gint/config.h"
  DESTINATION "${FXSDK_COMPILER_INSTALL}/include/gint")
# CMake module to find gint
install(FILES cmake/FindGint.cmake DESTINATION "${FXSDK_CMAKE_MODULE_PATH}")
