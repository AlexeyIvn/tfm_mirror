#-------------------------------------------------------------------------------
# Copyright (c) 2018-2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

#This file gathers all MPS2/AN519 specific files in the application.

#MPS2/AN519 has a Cortex M23 CPU.
include("Common/CpuM23")

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

#Specify the location of platform specific build dependencies.
if(COMPILER STREQUAL "ARMCLANG")
    set (S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/armclang/tfm_common_s.sct")
    set (BL2_SCATTER_FILE_NAME "${PLATFORM_DIR}/target/mps2/an519/armclang/mps2_an519_bl2.sct")
    set (NS_SCATTER_FILE_NAME  "${PLATFORM_DIR}/target/mps2/an519/armclang/mps2_an519_ns.sct")
    if (DEFINED CMSIS_5_DIR)
      # not all project defines CMSIS_5_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Library/ARM/RTX_V8MBN.lib")
    endif()
elseif(COMPILER STREQUAL "GNUARM")
    set (S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/gcc/tfm_common_s.ld")
    set (BL2_SCATTER_FILE_NAME "${PLATFORM_DIR}/target/mps2/an519/gcc/mps2_an519_bl2.ld")
    set (NS_SCATTER_FILE_NAME  "${PLATFORM_DIR}/target/mps2/an519/gcc/mps2_an519_ns.ld")
    if (DEFINED CMSIS_5_DIR)
      # not all project defines CMSIS_5_DIR, only the ones that use it.
      set (RTX_LIB_PATH "${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Library/GCC/libRTX_V8MBN.a")
    endif()
else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
endif()
set (FLASH_LAYOUT          "${PLATFORM_DIR}/target/mps2/an519/partition/flash_layout.h")
set (PLATFORM_LINK_INCLUDES "${PLATFORM_DIR}/target/mps2/an519/partition/")

if (BL2)
  set (BL2_LINKER_CONFIG ${BL2_SCATTER_FILE_NAME})
  if (${MCUBOOT_UPGRADE_STRATEGY} STREQUAL "RAM_LOADING")
      message(FATAL_ERROR "ERROR: RAM_LOADING upgrade strategy is not supported on target '${TARGET_PLATFORM}'.")
  endif()
endif()

embedded_include_directories(PATH "${PLATFORM_DIR}/cmsis" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/cmsis_core" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/retarget" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/native_drivers" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/partition" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/../include" ABSOLUTE)

#Gather all source files we need.
if (NOT DEFINED BUILD_CMSIS_CORE)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_CORE (true|false) is undefined!")
elseif(BUILD_CMSIS_CORE)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/cmsis_core/system_cmsdk_mps2_an519.c")
endif()

if (NOT DEFINED BUILD_RETARGET)
  message(FATAL_ERROR "Configuration variable BUILD_RETARGET (true|false) is undefined!")
elseif(BUILD_RETARGET)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/retarget/platform_retarget_dev.c")
endif()

if (NOT DEFINED BUILD_UART_STDOUT)
  message(FATAL_ERROR "Configuration variable BUILD_UART_STDOUT (true|false) is undefined!")
elseif(BUILD_UART_STDOUT)
  if (NOT DEFINED SECURE_UART1)
    message(FATAL_ERROR "Configuration variable SECURE_UART1 (true|false) is undefined!")
  elseif(SECURE_UART1)
    add_definitions(-DSECURE_UART1)
  endif()
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/uart_stdout.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
  set(BUILD_NATIVE_DRIVERS true)
  set(BUILD_CMSIS_DRIVERS true)
endif()

if (NOT DEFINED BUILD_NATIVE_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_NATIVE_DRIVERS (true|false) is undefined!")
elseif(BUILD_NATIVE_DRIVERS)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/native_drivers/arm_uart_drv.c")

  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/native_drivers/mpc_sie200_drv.c"
              "${PLATFORM_DIR}/target/mps2/an519/native_drivers/ppc_sse200_drv.c"
              )
endif()

if (NOT DEFINED BUILD_TIME)
  message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is undefined!")
elseif(BUILD_TIME)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/native_drivers/timer_cmsdk/timer_cmsdk.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/native_drivers/timer_cmsdk" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_STARTUP)
  message(FATAL_ERROR "Configuration variable BUILD_STARTUP (true|false) is undefined!")
elseif(BUILD_STARTUP)
  if(CMAKE_C_COMPILER_ID STREQUAL "ARMCLANG")
    list(APPEND ALL_SRC_ASM_S "${PLATFORM_DIR}/target/mps2/an519/armclang/startup_cmsdk_mps2_an519_s.s")
    list(APPEND ALL_SRC_ASM_NS "${PLATFORM_DIR}/target/mps2/an519/armclang/startup_cmsdk_mps2_an519_ns.s")
    list(APPEND ALL_SRC_ASM_BL2 "${PLATFORM_DIR}/target/mps2/an519/armclang/startup_cmsdk_mps2_an519_bl2.s")
  elseif(CMAKE_C_COMPILER_ID STREQUAL "GNUARM")
    list(APPEND ALL_SRC_ASM_S "${PLATFORM_DIR}/target/mps2/an519/gcc/startup_cmsdk_mps2_an519_s.S")
    list(APPEND ALL_SRC_ASM_NS "${PLATFORM_DIR}/target/mps2/an519/gcc/startup_cmsdk_mps2_an519_ns.S")
    list(APPEND ALL_SRC_ASM_BL2 "${PLATFORM_DIR}/target/mps2/an519/gcc/startup_cmsdk_mps2_an519_bl2.S")
    set_property(SOURCE "${ALL_SRC_ASM_S}" "${ALL_SRC_ASM_NS}" "${ALL_SRC_ASM_BL2}" APPEND
      PROPERTY COMPILE_DEFINITIONS "__STARTUP_CLEAR_BSS_MULTIPLE" "__STARTUP_COPY_MULTIPLE")
  else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
  endif()
endif()

if (NOT DEFINED BUILD_TARGET_CFG)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_CFG (true|false) is undefined!")
elseif(BUILD_TARGET_CFG)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/target_cfg.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/spm_hal.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/attest_hal.c")
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/native_drivers/mpu_armv8m_drv.c")
  if (TFM_PARTITION_PLATFORM)
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/services/src/tfm_platform_system.c")
  endif()
  embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_PLAT_TEST)
  message(FATAL_ERROR "Configuration variable BUILD_PLAT_TEST (true|false) is undefined!")
elseif(BUILD_PLAT_TEST)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/plat_test.c")
endif()

if (NOT DEFINED BUILD_TARGET_HARDWARE_KEYS)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_HARDWARE_KEYS (true|false) is undefined!")
elseif(BUILD_TARGET_HARDWARE_KEYS)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/tfm_initial_attestation_key_material.c")
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/dummy_crypto_keys.c")
endif()

if (NOT DEFINED BUILD_TARGET_NV_COUNTERS)
  message(FATAL_ERROR "Configuration variable BUILD_TARGET_NV_COUNTERS (true|false) is undefined!")
elseif(BUILD_TARGET_NV_COUNTERS)
  # NOTE: This non-volatile counters implementation is a dummy
  #       implementation. Platform vendors have to implement the
  #       API ONLY if the target has non-volatile counters.
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/dummy_nv_counters.c")
  set(TARGET_NV_COUNTERS_ENABLE ON)
  # Sets SST_ROLLBACK_PROTECTION flag to compile in the SST services
  # rollback protection code as the target supports nv counters.
  set (SST_ROLLBACK_PROTECTION ON)
endif()

if (NOT DEFINED BUILD_CMSIS_DRIVERS)
  message(FATAL_ERROR "Configuration variable BUILD_CMSIS_DRIVERS (true|false) is undefined!")
elseif(BUILD_CMSIS_DRIVERS)
  list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers/Driver_MPC.c"
    "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers/Driver_PPC.c")
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers/Driver_USART.c")
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers" ABSOLUTE)
  embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_FLASH)
  message(FATAL_ERROR "Configuration variable BUILD_FLASH (true|false) is undefined!")
elseif(BUILD_FLASH)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers/Driver_Flash.c")
  # There is no real flash memory for code on MPS2 board. Instead a code SRAM is
  # used for code storage: ZBT SSRAM1. The Driver_Flash driver just emulates a flash
  # interface and behaviour on top of the SRAM memory.
  # As the SST area is going to be in RAM, it is required to set SST_CREATE_FLASH_LAYOUT
  # to be sure the SST service knows that when it starts the SST area does not contain any
  # valid SST flash layout and it needs to create one.
  set(SST_CREATE_FLASH_LAYOUT ON)
  set(SST_RAM_FS OFF)
  embedded_include_directories(PATH "${PLATFORM_DIR}/target/mps2/an519/cmsis_drivers" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_BOOT_SEED)
  message(FATAL_ERROR "Configuration variable BUILD_BOOT_SEED (true|false) is undefined!")
elseif(BUILD_BOOT_SEED)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/dummy_boot_seed.c")
endif()

if (NOT DEFINED BUILD_DEVICE_ID)
  message(FATAL_ERROR "Configuration variable BUILD_DEVICE_ID (true|false) is undefined!")
elseif(BUILD_DEVICE_ID)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/mps2/an519/dummy_device_id.c")
endif()
