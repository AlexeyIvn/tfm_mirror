/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stdio.h>
#include "cmsis.h"
#include "platform/include/tfm_spm_hal.h"
#include "spm_api.h"
#include "spm_db.h"
#include "tfm_platform_core_api.h"
#include "target_cfg.h"
#include "Driver_MPC.h"
#include "mpu_armv8m_drv.h"
#include "region_defs.h"
#include "secure_utilities.h"

/* Import MPC driver */
extern ARM_DRIVER_MPC Driver_SRAM1_MPC;

/* Get address of memory regions to configure MPU */
extern const struct memory_region_limits memory_regions;

struct mpu_armv8m_dev_t dev_mpu_s = { MPU_BASE };

enum tfm_plat_err_t tfm_spm_hal_init_isolation_hw(void)
{
    int32_t ret = ARM_DRIVER_OK;
    /* Configures non-secure memory spaces in the target */
    sau_and_idau_cfg();
    ret = mpc_init_cfg();
    if (ret != ARM_DRIVER_OK) {
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
    ppc_init_cfg();
    return TFM_PLAT_ERR_SUCCESS;
}

void tfm_spm_hal_configure_default_isolation(
        const struct tfm_spm_partition_platform_data_t *platform_data)
{
    if (platform_data) {
        if (platform_data->periph_ppc_bank != PPC_SP_DO_NOT_CONFIGURE) {
            ppc_configure_to_secure(platform_data->periph_ppc_bank,
                                    platform_data->periph_ppc_loc);
        }
    }
}

#if TFM_LVL != 1

#define MPU_REGION_VENEERS           0
#define MPU_REGION_TFM_UNPRIV_CODE   1
#define MPU_REGION_TFM_UNPRIV_DATA   2
#define PARTITION_REGION_RO          4
#define PARTITION_REGION_RW_STACK    5
#define PARTITION_REGION_PERIPH      6
#define PARTITION_REGION_SHARE       7

#if TFM_LVL == 2
#define MPU_REGION_NS_STACK          3
#elif TFM_LVL == 3
#define MPU_REGION_NS_DATA           3
#endif

REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Limit);
REGION_DECLARE(Image$$, TFM_UNPRIV_DATA, $$RW$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_DATA, $$ZI$$Limit);
#ifndef TFM_PSA_API
REGION_DECLARE(Image$$, TFM_UNPRIV_SCRATCH, $$ZI$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_SCRATCH, $$ZI$$Limit);
#endif
#if TFM_LVL == 2
REGION_DECLARE(Image$$, TFM_APP_CODE_START, $$Base);
REGION_DECLARE(Image$$, TFM_APP_CODE_END, $$Base);
REGION_DECLARE(Image$$, TFM_APP_RW_STACK_START, $$Base);
REGION_DECLARE(Image$$, TFM_APP_RW_STACK_END, $$Base);
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Base);
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Limit);
#endif

static enum spm_err_t tfm_spm_mpu_init(void)
{
    struct mpu_armv8m_region_cfg_t region_cfg;

    mpu_armv8m_clean(&dev_mpu_s);

    /* Veneer region */
    region_cfg.region_nr = MPU_REGION_VENEERS;
    region_cfg.region_base = memory_regions.veneer_base;
    region_cfg.region_limit = memory_regions.veneer_limit;
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_CODE_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RO_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_OK;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

    /* TFM Core unprivileged code region */
    region_cfg.region_nr = MPU_REGION_TFM_UNPRIV_CODE;
    region_cfg.region_base =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Base);
    region_cfg.region_limit =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Limit);
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_CODE_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RO_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_OK;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

    /* TFM Core unprivileged data region */
    region_cfg.region_nr = MPU_REGION_TFM_UNPRIV_DATA;
    region_cfg.region_base =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_DATA, $$RW$$Base);
    region_cfg.region_limit =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_DATA, $$ZI$$Limit);
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

#if TFM_LVL == 3
    /* TFM Core unprivileged non-secure data region */
    region_cfg.region_nr = MPU_REGION_NS_DATA;
    region_cfg.region_base = NS_DATA_START;
    region_cfg.region_limit = NS_DATA_LIMIT;
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }
#endif

#if TFM_LVL == 2
    /* NSPM PSP */
    region_cfg.region_nr = MPU_REGION_NS_STACK;
    region_cfg.region_base =
        (uint32_t)&REGION_NAME(Image$$, ARM_LIB_STACK, $$ZI$$Base);
    region_cfg.region_limit =
        (uint32_t)&REGION_NAME(Image$$, ARM_LIB_STACK, $$ZI$$Limit);
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

    /* RO region */
    region_cfg.region_nr = PARTITION_REGION_RO;
    region_cfg.region_base =
        (uint32_t)&REGION_NAME(Image$$, TFM_APP_CODE_START, $$Base);
    region_cfg.region_limit =
        (uint32_t)&REGION_NAME(Image$$, TFM_APP_CODE_END, $$Base);
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_CODE_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RO_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_OK;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

    /* RW, ZI and stack as one region */
    region_cfg.region_nr = PARTITION_REGION_RW_STACK;
    region_cfg.region_base =
        (uint32_t)&REGION_NAME(Image$$, TFM_APP_RW_STACK_START, $$Base);
    region_cfg.region_limit =
        (uint32_t)&REGION_NAME(Image$$, TFM_APP_RW_STACK_END, $$Base);
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }
#endif

    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE,
                      HARDFAULT_NMI_ENABLE);

    return SPM_ERR_OK;
}

enum spm_err_t tfm_spm_hal_partition_sandbox_config(
        const struct tfm_spm_partition_memory_data_t *memory_data,
        const struct tfm_spm_partition_platform_data_t *platform_data)
{
    /* This function takes a partition id and enables the
     * SPM partition for that partition
     */

    struct mpu_armv8m_region_cfg_t region_cfg;

    mpu_armv8m_disable(&dev_mpu_s);

    /* Configure Regions */
    if (memory_data->ro_start) {
        /* RO region */
        region_cfg.region_nr = PARTITION_REGION_RO;
        region_cfg.region_base = memory_data->ro_start;
        region_cfg.region_limit = memory_data->ro_limit;
        region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_CODE_IDX;
        region_cfg.attr_access = MPU_ARMV8M_AP_RO_PRIV_UNPRIV;
        region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
        region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_OK;

        if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg)
            != MPU_ARMV8M_OK) {
            return SPM_ERR_INVALID_CONFIG;
        }
    }

    /* RW, ZI and stack as one region */
    region_cfg.region_nr = PARTITION_REGION_RW_STACK;
    region_cfg.region_base = memory_data->rw_start;
    region_cfg.region_limit = memory_data->stack_top;
    region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;

    if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg) != MPU_ARMV8M_OK) {
        return SPM_ERR_INVALID_CONFIG;
    }

    if (platform_data) {
        /* Peripheral */
        region_cfg.region_nr = PARTITION_REGION_PERIPH;
        region_cfg.region_base = platform_data->periph_start;
        region_cfg.region_limit = platform_data->periph_limit;
        region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DEVICE_IDX;
        region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
        region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
        region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
        if (mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg)
            != MPU_ARMV8M_OK) {
            return SPM_ERR_INVALID_CONFIG;
        }

        ppc_en_secure_unpriv(platform_data->periph_ppc_bank,
                             platform_data->periph_ppc_loc);
    }

    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE,
                      HARDFAULT_NMI_ENABLE);

    return SPM_ERR_OK;
}

enum spm_err_t tfm_spm_hal_partition_sandbox_deconfig(
        const struct tfm_spm_partition_memory_data_t *memory_data,
        const struct tfm_spm_partition_platform_data_t *platform_data)
{
    /* This function takes a partition id and disables the
     * SPM partition for that partition
     */

    if (platform_data) {
        /* Peripheral */
        ppc_clr_secure_unpriv(platform_data->periph_ppc_bank,
                              platform_data->periph_ppc_loc);
    }

    mpu_armv8m_disable(&dev_mpu_s);
    mpu_armv8m_region_disable(&dev_mpu_s, PARTITION_REGION_RO);
    mpu_armv8m_region_disable(&dev_mpu_s, PARTITION_REGION_RW_STACK);
    mpu_armv8m_region_disable(&dev_mpu_s, PARTITION_REGION_PERIPH);
    mpu_armv8m_region_disable(&dev_mpu_s, PARTITION_REGION_SHARE);
    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE,
                      HARDFAULT_NMI_ENABLE);

    return SPM_ERR_OK;
}

#if !defined(TFM_PSA_API)
/**
 * Set share region to which the partition needs access
 */
enum spm_err_t tfm_spm_hal_set_share_region(uint32_t share)
{
    struct mpu_armv8m_region_cfg_t region_cfg;
    enum spm_err_t res = SPM_ERR_INVALID_CONFIG;
    uint32_t scratch_base =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_SCRATCH, $$ZI$$Base);
    uint32_t scratch_limit =
        (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_SCRATCH, $$ZI$$Limit);

    mpu_armv8m_disable(&dev_mpu_s);

    if (share == TFM_BUFFER_SHARE_DISABLE) {
        mpu_armv8m_region_disable(&dev_mpu_s, PARTITION_REGION_SHARE);
    } else {

        region_cfg.region_nr = PARTITION_REGION_SHARE;
        region_cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
        region_cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
        region_cfg.attr_sh = MPU_ARMV8M_SH_NONE;
        region_cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
        switch (share) {
        case TFM_BUFFER_SHARE_SCRATCH:
            /* Use scratch area for SP-to-SP data sharing */
            region_cfg.region_base = scratch_base;
            region_cfg.region_limit = scratch_limit;
            res = SPM_ERR_OK;
            break;
        case TFM_BUFFER_SHARE_NS_CODE:
            region_cfg.region_base = memory_regions.non_secure_partition_base;
            region_cfg.region_limit = memory_regions.non_secure_partition_limit;
            /* Only allow read access to NS code region and keep
             * exec.never attribute
             */
            region_cfg.attr_access = MPU_ARMV8M_AP_RO_PRIV_UNPRIV;
            res = SPM_ERR_OK;
            break;
        default:
            /* Leave res to be set to SPM_ERR_INVALID_CONFIG */
            break;
        }
        if (res == SPM_ERR_OK) {
            mpu_armv8m_region_enable(&dev_mpu_s, &region_cfg);
        }
    }
    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE,
                      HARDFAULT_NMI_ENABLE);

    return res;
}
#endif /* !defined(TFM_PSA_API) */
#endif /* TFM_LVL != 1 */

enum tfm_plat_err_t tfm_spm_hal_setup_isolation_hw(void)
{
#if TFM_LVL != 1
    if (tfm_spm_mpu_init() != SPM_ERR_OK) {
        ERROR_MSG("Failed to set up initial MPU configuration! Halting.");
        return TFM_PLAT_ERR_SYSTEM_ERR;
    }
#endif
    return TFM_PLAT_ERR_SUCCESS;
}

void MPC_Handler(void)
{
    /* Clear MPC interrupt flag and pending MPC IRQ */
    Driver_SRAM1_MPC.ClearInterrupt();
    NVIC_ClearPendingIRQ(MPC_IRQn);

    /* Print fault message and block execution */
    LOG_MSG("Oops... MPC fault!!!");

    /* Inform TF-M core that isolation boundary has been violated */
    tfm_access_violation_handler();
}

void PPC_Handler(void)
{
    /*
     * Due to an issue on the FVP, the PPC fault doesn't trigger a
     * PPC IRQ which is handled by the PPC_handler.
     * In the FVP execution, this code is not execute.
     */

    /* Clear PPC interrupt flag and pending PPC IRQ */
    ppc_clear_irq();
    NVIC_ClearPendingIRQ(PPC_IRQn);

    /* Print fault message*/
    LOG_MSG("Oops... PPC fault!!!");

    /* Inform TF-M core that isolation boundary has been violated */
    tfm_access_violation_handler();
}

uint32_t tfm_spm_hal_get_ns_VTOR(void)
{
    return memory_regions.non_secure_code_start;
}

uint32_t tfm_spm_hal_get_ns_MSP(void)
{
    return *((uint32_t *)memory_regions.non_secure_code_start);
}

uint32_t tfm_spm_hal_get_ns_entry_point(void)
{
    return *((uint32_t *)(memory_regions.non_secure_code_start+ 4));
}

enum tfm_plat_err_t tfm_spm_hal_set_secure_irq_priority(int32_t irq_line,
                                                        uint32_t priority)
{
    uint32_t quantized_priority = priority >> (8U - __NVIC_PRIO_BITS);
    NVIC_SetPriority(irq_line, quantized_priority);
    return TFM_PLAT_ERR_SUCCESS;
}

void tfm_spm_hal_clear_pending_irq(int32_t irq_line)
{
    NVIC_ClearPendingIRQ(irq_line);
}

void tfm_spm_hal_enable_irq(int32_t irq_line)
{
    NVIC_EnableIRQ(irq_line);
}

void tfm_spm_hal_disable_irq(int32_t irq_line)
{
    NVIC_DisableIRQ(irq_line);
}

enum irq_target_state_t tfm_spm_hal_set_irq_target_state(
                                           int32_t irq_line,
                                           enum irq_target_state_t target_state)
{
    uint32_t result;

    if (target_state == TFM_IRQ_TARGET_STATE_SECURE) {
        result = NVIC_ClearTargetState(irq_line);
    } else {
        result = NVIC_SetTargetState(irq_line);
    }

    if (result) {
        return TFM_IRQ_TARGET_STATE_NON_SECURE;
    } else {
        return TFM_IRQ_TARGET_STATE_SECURE;
    }
}

enum tfm_plat_err_t tfm_spm_hal_enable_fault_handlers(void)
{
    return enable_fault_handlers();
}

enum tfm_plat_err_t tfm_spm_hal_system_reset_cfg(void)
{
    return system_reset_cfg();
}

enum tfm_plat_err_t tfm_spm_hal_init_debug(void)
{
    return init_debug();
}

enum tfm_plat_err_t tfm_spm_hal_nvic_interrupt_target_state_cfg(void)
{
    return nvic_interrupt_target_state_cfg();
}

enum tfm_plat_err_t tfm_spm_hal_nvic_interrupt_enable(void)
{
    return nvic_interrupt_enable();
}
