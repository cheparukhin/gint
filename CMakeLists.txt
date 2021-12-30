# Build system for the gint unikernel

cmake_minimum_required(VERSION 3.15)
project(Gint VERSION 2.6.0 LANGUAGES C ASM)

include(GitVersionNumber)
include(Fxconv)

option(GINT_USER_VRAM "Put all VRAMs into the user stack (fx-CG 50 only)")
option(GINT_STATIC_GRAY "Use static memory instead of malloc for gray buffers (fx-9860G only)")
option(GINT_KMALLOC_DEBUG "Enable debug functions for kmalloc")

set(CMAKE_INSTALL_MESSAGE LAZY)

# Generate <gint/config.h> with commit hash, version name and options
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  git_version_number(SHORT_HASH "GINT_GIT_HASH" TAG_RELATIVE "GINT_GIT_VERSION")
else()
  set(GINT_GIT_HASH "0000000")
  set(GINT_GIT_VERSION "${PROJECT_VERSION}")
endif()
configure_file(include/gint/config.h.in include/gint/config.h)

set(SOURCES_COMMON
  # Clock Pulse Generator driver
  src/cpg/cpg.c
  # CPU driver
  src/cpu/atomic.c
  src/cpu/cpu.c
  src/cpu/ics.s
  src/cpu/registers.s
  src/cpu/sleep.c
  # Direct Memory Access driver
  src/dma/dma.c
  src/dma/inth.s
  src/dma/memcpy.c
  src/dma/memset.c
  # Filesystem interface
  src/fs/close.c
  src/fs/closedir.c
  src/fs/creat.c
  src/fs/fdopendir.c
  src/fs/fs.c
  src/fs/lseek.c
  src/fs/mkdir.c
  src/fs/open.c
  src/fs/opendir.c
  src/fs/pread.c
  src/fs/pwrite.c
  src/fs/read.c
  src/fs/readdir.c
  src/fs/rewinddir.c
  src/fs/rmdir.c
  src/fs/seekdir.c
  src/fs/stat.c
  src/fs/telldir.c
  src/fs/unlink.c
  src/fs/write.c
  # Filesystem interface to Fugue
  src/fs/fugue/BFile_Ext_Stat.c
  src/fs/fugue/fugue.c
  src/fs/fugue/fugue_dir.c
  src/fs/fugue/fugue_open.c
  src/fs/fugue/fugue_mkdir.c
  src/fs/fugue/fugue_stat.c
  src/fs/fugue/fugue_rmdir.c
  src/fs/fugue/fugue_unlink.c
  src/fs/fugue/util.c
  # Interrupt Controller driver
  src/intc/intc.c
  src/intc/inth.s
  # Kernel
  src/kernel/exch.c
  src/kernel/exch.s
  src/kernel/hardware.c
  src/kernel/inth.S
  src/kernel/kernel.c
  src/kernel/osmenu.c
  src/kernel/start.c
  src/kernel/syscalls.S
  src/kernel/tlbh.S
  src/kernel/world.c
  # Key Scan Interface driver
  src/keysc/getkey.c
  src/keysc/iokbd.c
  src/keysc/keycodes.c
  src/keysc/keydev.c
  src/keysc/keysc.c
  # Memory allocator
  src/kmalloc/arena_gint.c
  src/kmalloc/arena_osheap.c
  src/kmalloc/kmalloc.c
  # MMU driver
  src/mmu/mmu.c
  # Rendering
  src/render/dhline.c
  src/render/dimage.c
  src/render/dline.c
  src/render/dprint.c
  src/render/drect_border.c
  src/render/dtext.c
  src/render/dupdate_hook.c
  src/render/dvline.c
  src/render/topti.c
  # RTC driver
  src/rtc/rtc.c
  src/rtc/rtc_ticks.c
  # Sound Processing Unit driver
  src/spu/spu.c
  # Timer Unit driver
  src/tmu/inth-etmu.s
  src/tmu/inth-tmu.s
  src/tmu/sleep.c
  src/tmu/tmu.c
  # USB driver
  src/usb/classes/ff-bulk.c
  src/usb/configure.c
  src/usb/pipes.c
  src/usb/setup.c
  src/usb/string.c
  src/usb/usb.c
)
set(SOURCES_FX
  # Gray engine
  src/gray/engine.c
  src/gray/gclear.c
  src/gray/gint_gline.c
  src/gray/gpixel.c
  src/gray/grect.c
  src/gray/gsubimage.c
  src/gray/gtext.c
  # Rendering
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
  # T6K11 driver
  src/t6k11/t6k11.c

  src/usb/classes/ff-bulk-gray.c
)
set(SOURCES_CG
  # R61524 driver
  src/r61524/r61524.c
  # Rendering
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

include_directories(
  "${PROJECT_SOURCE_DIR}/include"
  "${PROJECT_BINARY_DIR}/include"
  "${FXSDK_COMPILER_INSTALL}/include/openlibm")
add_compile_options(-Wall -Wextra -std=c11 -Os -fstrict-volatile-bitfields -mtas)

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
