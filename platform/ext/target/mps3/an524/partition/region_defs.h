/*
 * Copyright (c) 2019 Arm Limited. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __REGION_DEFS_H__
#define __REGION_DEFS_H__

#include "flash_layout.h"

#define BL2_HEAP_SIZE           (0x0001000)
#define BL2_MSP_STACK_SIZE      (0x0001800)

#define S_HEAP_SIZE             (0x0001000)
#define S_MSP_STACK_SIZE_INIT   (0x0000400)
#define S_MSP_STACK_SIZE        (0x0000800)
#define S_PSP_STACK_SIZE        (0x0000800)

#define NS_HEAP_SIZE            (0x0001000)
#define NS_MSP_STACK_SIZE       (0x0000400)
#define NS_PSP_STACK_SIZE       (0x0000C00)

/* This size of buffer is big enough to store an attestation
 * token produced by initial attestation service
 */
#define PSA_INITIAL_ATTEST_TOKEN_MAX_SIZE   (0x200)

/* This configuration uses QSPI for code memory and ISRAM0-3 as data
 * memory.
 */

#ifdef BL2
#ifndef LINK_TO_SECONDARY_PARTITION
#define S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_IMAGE_0_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_1_OFFSET)
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET   (FLASH_AREA_IMAGE_1_OFFSET)
#define S_IMAGE_SECONDARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_0_OFFSET)
#endif /* !LINK_TO_SECONDARY_PARTITION */
#else
#define S_IMAGE_PRIMARY_PARTITION_OFFSET (0x0)
#endif /* BL2 */

#ifndef LINK_TO_SECONDARY_PARTITION
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_0_OFFSET \
                                           + FLASH_S_PARTITION_SIZE)
#else
#define NS_IMAGE_PRIMARY_PARTITION_OFFSET (FLASH_AREA_IMAGE_1_OFFSET \
                                           + FLASH_S_PARTITION_SIZE)
#endif /* !LINK_TO_SECONDARY_PARTITION */


#ifdef BL2
/* Boot partition structure if MCUBoot is used:
 * 1. Bootloader header
 * 2. Image area
 * 3. Trailer
 */
#define BL2_HEADER_SIZE      (0x400)     /* 1 KB */
#define BL2_TRAILER_SIZE     (0x400)     /* 1 KB */
#else
/* No header if no bootloader, but keep IMAGE_CODE_SIZE the same */
#define BL2_HEADER_SIZE      (0x0)
#define BL2_TRAILER_SIZE     (0x800)
#endif /* BL2 */

#define IMAGE_S_CODE_SIZE \
            (FLASH_S_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)
#define IMAGE_NS_CODE_SIZE \
            (FLASH_NS_PARTITION_SIZE - BL2_HEADER_SIZE - BL2_TRAILER_SIZE)

#define CMSE_VENEER_REGION_SIZE     (0x300)

/* Alias definitions for secure and non-secure areas*/
#define S_ROM_ALIAS(x)  (S_ROM_ALIAS_BASE + (x))
#define NS_ROM_ALIAS(x) (NS_ROM_ALIAS_BASE + (x))

#define S_RAM_ALIAS(x)  (S_RAM_ALIAS_BASE + (x))
#define NS_RAM_ALIAS(x) (NS_RAM_ALIAS_BASE + (x))

/* Secure regions */
#define S_IMAGE_PRIMARY_AREA_OFFSET \
             (S_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
#define S_CODE_START    (S_ROM_ALIAS(S_IMAGE_PRIMARY_AREA_OFFSET))
#define S_CODE_SIZE     (IMAGE_S_CODE_SIZE - CMSE_VENEER_REGION_SIZE)
#define S_CODE_LIMIT    (S_CODE_START + S_CODE_SIZE - 1)

/* ISRAM0 and ISRAM1 are assigned to Secure data */
#define S_DATA_START    (S_RAM_ALIAS(0x0))
#define S_DATA_SIZE     (TOTAL_RAM_SIZE / 2 + (TOTAL_RAM_SIZE / 4))
#define S_DATA_LIMIT    (S_DATA_START + S_DATA_SIZE - 1)

/* CMSE Veneers region */
#define CMSE_VENEER_REGION_START  (S_CODE_LIMIT + 1)

/* Non-secure regions */
#define NS_IMAGE_PRIMARY_AREA_OFFSET \
                        (NS_IMAGE_PRIMARY_PARTITION_OFFSET + BL2_HEADER_SIZE)
#define NS_CODE_START   (NS_ROM_ALIAS(NS_IMAGE_PRIMARY_AREA_OFFSET))
#define NS_CODE_SIZE    (IMAGE_NS_CODE_SIZE)
#define NS_CODE_LIMIT   (NS_CODE_START + NS_CODE_SIZE - 1)

/* ISRAM2 and ISRAM3 are assigned to Secure data */
#define NS_DATA_START   (NS_RAM_ALIAS(S_DATA_SIZE))
#define NS_DATA_SIZE    (TOTAL_RAM_SIZE - S_DATA_SIZE)
#define NS_DATA_LIMIT   (NS_DATA_START + NS_DATA_SIZE - 1)

/* NS partition information is used for MPC and SAU configuration */
#define NS_PARTITION_START \
            (NS_ROM_ALIAS(NS_IMAGE_PRIMARY_PARTITION_OFFSET))
#define NS_PARTITION_SIZE  (FLASH_NS_PARTITION_SIZE)

#ifdef BL2
/* Secondary partition for new images in case of firmware upgrade */
#define SECONDARY_PARTITION_START \
            (NS_ROM_ALIAS(S_IMAGE_SECONDARY_PARTITION_OFFSET))
#define SECONDARY_PARTITION_SIZE (FLASH_AREA_IMAGE_1_SIZE)
#define SECONDARY_PARTITION_LIMIT  (SECONDARY_PARTITION_START + \
                                    SECONDARY_PARTITION_SIZE - 1)

#define BL2_ROM_ALIAS_BASE         (0x0)
#define BL2_ROM_ALIAS(x)           (BL2_ROM_ALIAS_BASE + (x))

/* Bootloader regions */
#define BL2_CODE_START    (BL2_ROM_ALIAS(FLASH_AREA_BL2_OFFSET))
#define BL2_CODE_SIZE     (FLASH_AREA_BL2_SIZE)
#define BL2_CODE_LIMIT    (BL2_CODE_START + BL2_CODE_SIZE - 1)

#define BL2_DATA_START    (S_RAM_ALIAS(0x0))
#define BL2_DATA_SIZE     (TOTAL_RAM_SIZE)
#define BL2_DATA_LIMIT    (BL2_DATA_START + BL2_DATA_SIZE - 1)
#endif /* BL2 */

#endif /* __REGION_DEFS_H__ */
