#-------------------------------------------------------------------------------
# Copyright (c) 2018-2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

# This file gathers all Musca-B1 specific files in the application.

# Musca-B1 has a Cortex-M33 CPU.
include("Common/CpuM33")

set(PLATFORM_DIR ${CMAKE_CURRENT_LIST_DIR})

# Specify the location of platform specific build dependencies.
if (COMPILER STREQUAL "ARMCLANG")
    set(S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/armclang/tfm_common_s.sct")
    set(BL2_SCATTER_FILE_NAME "${PLATFORM_DIR}/target/musca_b1/Device/Source/armclang/musca_bl2.sct")
    set(NS_SCATTER_FILE_NAME  "${PLATFORM_DIR}/target/musca_b1/Device/Source/armclang/musca_ns.sct")
    if (DEFINED CMSIS_5_DIR)
        # Not all projects define CMSIS_5_DIR, only the ones that use it.
        set(RTX_LIB_PATH "${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Library/ARM/RTX_V8MMN.lib")
    endif()
elseif (COMPILER STREQUAL "GNUARM")
    set(S_SCATTER_FILE_NAME   "${PLATFORM_DIR}/common/gcc/tfm_common_s.ld")
    set(BL2_SCATTER_FILE_NAME "${PLATFORM_DIR}/target/musca_b1/Device/Source/gcc/musca_bl2.ld")
    set(NS_SCATTER_FILE_NAME  "${PLATFORM_DIR}/target/musca_b1/Device/Source/gcc/musca_ns.ld")
    if (DEFINED CMSIS_5_DIR)
        # Not all projects define CMSIS_5_DIR, only the ones that use it.
        set(RTX_LIB_PATH "${CMSIS_5_DIR}/CMSIS/RTOS2/RTX/Library/GCC/libRTX_V8MMN.a")
    endif()
else()
    message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
endif()
set(FLASH_LAYOUT           "${PLATFORM_DIR}/target/musca_b1/partition/flash_layout.h")
set(PLATFORM_LINK_INCLUDES "${PLATFORM_DIR}/target/musca_b1/partition")

if (BL2)
    set(BL2_LINKER_CONFIG ${BL2_SCATTER_FILE_NAME})
    if (NOT ${MCUBOOT_UPGRADE_STRATEGY} STREQUAL "NO_SWAP")
        message(WARNING "NO_SWAP upgrade strategy is mandatory on target '${TARGET_PLATFORM}'. Your choice was overriden.")
        set(MCUBOOT_UPGRADE_STRATEGY "NO_SWAP")
    endif()
endif()

embedded_include_directories(PATH "${PLATFORM_DIR}/cmsis" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver/Config" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/Device/Config" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/Device/Include" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/Native_Driver" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/partition" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/services/include" ABSOLUTE)
embedded_include_directories(PATH "${PLATFORM_DIR}/../include" ABSOLUTE)

# Gather all source files we need.
if (TFM_PARTITION_PLATFORM)
    list(APPEND ALL_SRC_C_NS "${PLATFORM_DIR}/target/musca_b1/services/src/tfm_ioctl_ns_api.c")
endif()

if (NOT DEFINED BUILD_CMSIS_CORE)
    message(FATAL_ERROR "Configuration variable BUILD_CMSIS_CORE (true|false) is undefined!")
elseif (BUILD_CMSIS_CORE)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/Device/Source/system_core_init.c")
endif()

if (NOT DEFINED BUILD_RETARGET)
    message(FATAL_ERROR "Configuration variable BUILD_RETARGET (true|false) is undefined!")
elseif (BUILD_RETARGET)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/Device/Source/device_definition.c")
endif()

if (NOT DEFINED BUILD_UART_STDOUT)
    message(FATAL_ERROR "Configuration variable BUILD_UART_STDOUT (true|false) is undefined!")
elseif (BUILD_UART_STDOUT)
    if (NOT DEFINED SECURE_UART1)
        message(FATAL_ERROR "Configuration variable SECURE_UART1 (true|false) is undefined!")
    elseif (SECURE_UART1)
        message(FATAL_ERROR "Configuration SECURE_UART1 TRUE is invalid for this target!")
    endif()
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/uart_stdout.c")
    embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
    set(BUILD_NATIVE_DRIVERS True)
    set(BUILD_CMSIS_DRIVERS True)
endif()

if (NOT DEFINED BUILD_NATIVE_DRIVERS)
    message(FATAL_ERROR "Configuration variable BUILD_NATIVE_DRIVERS (true|false) is undefined!")
elseif (BUILD_NATIVE_DRIVERS)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/Native_Driver/uart_pl011_drv.c"
                          "${PLATFORM_DIR}/target/musca_b1/Native_Driver/musca_b1_scc_drv.c")

    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/Native_Driver/mpc_sie200_drv.c"
                            "${PLATFORM_DIR}/target/musca_b1/Native_Driver/ppc_sse200_drv.c")
endif()

if (NOT DEFINED BUILD_TIME)
    message(FATAL_ERROR "Configuration variable BUILD_TIME (true|false) is undefined!")
elseif (BUILD_TIME)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/Native_Driver/timer_cmsdk_drv.c")
    embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/Native_Driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_STARTUP)
    message(FATAL_ERROR "Configuration variable BUILD_STARTUP (true|false) is undefined!")
elseif (BUILD_STARTUP)
    if (CMAKE_C_COMPILER_ID STREQUAL "ARMCLANG")
        list(APPEND ALL_SRC_ASM_S "${PLATFORM_DIR}/target/musca_b1/Device/Source/armclang/startup_cmsdk_musca_s.s")
        list(APPEND ALL_SRC_ASM_NS "${PLATFORM_DIR}/target/musca_b1/Device/Source/armclang/startup_cmsdk_musca_ns.s")
        list(APPEND ALL_SRC_ASM_BL2 "${PLATFORM_DIR}/target/musca_b1/Device/Source/armclang/startup_cmsdk_musca_bl2.s")
    elseif (CMAKE_C_COMPILER_ID STREQUAL "GNUARM")
        list(APPEND ALL_SRC_ASM_S "${PLATFORM_DIR}/target/musca_b1/Device/Source/gcc/startup_cmsdk_musca_s.S")
        list(APPEND ALL_SRC_ASM_NS "${PLATFORM_DIR}/target/musca_b1/Device/Source/gcc/startup_cmsdk_musca_ns.S")
        list(APPEND ALL_SRC_ASM_BL2 "${PLATFORM_DIR}/target/musca_b1/Device/Source/gcc/startup_cmsdk_musca_bl2.S")
        set_property(SOURCE "${ALL_SRC_ASM_S}" "${ALL_SRC_ASM_NS}" "${ALL_SRC_ASM_BL2}"
                     APPEND PROPERTY COMPILE_DEFINITIONS "__STARTUP_CLEAR_BSS_MULTIPLE" "__STARTUP_COPY_MULTIPLE")
    else()
        message(FATAL_ERROR "No startup file is available for compiler '${CMAKE_C_COMPILER_ID}'.")
    endif()
endif()

if (NOT DEFINED BUILD_TARGET_CFG)
    message(FATAL_ERROR "Configuration variable BUILD_TARGET_CFG (true|false) is undefined!")
elseif (BUILD_TARGET_CFG)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/target_cfg.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/spm_hal.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/attest_hal.c")
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/Native_Driver/mpu_armv8m_drv.c")
    if (TFM_PARTITION_PLATFORM)
        list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/services/src/tfm_platform_system.c")
        list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/services/src/tfm_ioctl_s_api.c")
    endif()
    embedded_include_directories(PATH "${PLATFORM_DIR}/common" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_PLAT_TEST)
    message(FATAL_ERROR "Configuration variable BUILD_PLAT_TEST (true|false) is undefined!")
elseif(BUILD_PLAT_TEST)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/plat_test.c")
endif()

if (NOT DEFINED BUILD_TARGET_HARDWARE_KEYS)
    message(FATAL_ERROR "Configuration variable BUILD_TARGET_HARDWARE_KEYS (true|false) is undefined!")
elseif (BUILD_TARGET_HARDWARE_KEYS)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/common/tfm_initial_attestation_key_material.c")
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/dummy_crypto_keys.c")
endif()

if (NOT DEFINED BUILD_TARGET_NV_COUNTERS)
    message(FATAL_ERROR "Configuration variable BUILD_TARGET_NV_COUNTERS (true|false) is undefined!")
elseif (BUILD_TARGET_NV_COUNTERS)
    # NOTE: This non-volatile counters implementation is a dummy
    #       implementation. Platform vendors have to implement the
    #       API ONLY if the target has non-volatile counters.
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/dummy_nv_counters.c")
    set(TARGET_NV_COUNTERS_ENABLE ON)
    # Sets SST_ROLLBACK_PROTECTION flag to compile in the SST services
    # rollback protection code as the target supports nv counters.
    set(SST_ROLLBACK_PROTECTION ON)
endif()

if (NOT DEFINED BUILD_CMSIS_DRIVERS)
    message(FATAL_ERROR "Configuration variable BUILD_CMSIS_DRIVERS (true|false) is undefined!")
elseif (BUILD_CMSIS_DRIVERS)
    list(APPEND ALL_SRC_C_S "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver/Driver_MPC.c"
                            "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver/Driver_PPC.c")
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver/Driver_USART.c")
    embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver" ABSOLUTE)
    embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_FLASH)
    message(FATAL_ERROR "Configuration variable BUILD_FLASH (true|false) is undefined!")
elseif (BUILD_FLASH)
    list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver/Driver_Flash.c")
    # As the SST area is going to be in RAM, it is required to set
    # SST_CREATE_FLASH_LAYOUT to be sure the SST service knows that when it
    # starts the SST area does not contain any valid SST flash layout and it
    # needs to create one.
    set(SST_CREATE_FLASH_LAYOUT ON)
    embedded_include_directories(PATH "${PLATFORM_DIR}/target/musca_b1/CMSIS_Driver" ABSOLUTE)
    embedded_include_directories(PATH "${PLATFORM_DIR}/driver" ABSOLUTE)
endif()

if (NOT DEFINED BUILD_BOOT_SEED)
  message(FATAL_ERROR "Configuration variable BUILD_BOOT_SEED (true|false) is undefined!")
elseif(BUILD_BOOT_SEED)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/dummy_boot_seed.c")
endif()

if (NOT DEFINED BUILD_DEVICE_ID)
  message(FATAL_ERROR "Configuration variable BUILD_DEVICE_ID (true|false) is undefined!")
elseif(BUILD_DEVICE_ID)
  list(APPEND ALL_SRC_C "${PLATFORM_DIR}/target/musca_b1/dummy_device_id.c")
endif()
