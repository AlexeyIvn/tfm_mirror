/*
 * Copyright (c) 2017 ARM Limited
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

/*
 * This file is derivative of CMSIS V5.01 \Device\ARM\ARMCM33\Source\system_ARMCM33.c
 */

#include "system_cmsdk_ref_twincpu.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define  XTAL            (50000000UL)
#define  SYSTEM_CLOCK    (XTAL)
#define  SYSTEM_REFCLK   (32768UL)

/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK;
uint32_t PeripheralClock = SYSTEM_CLOCK;
uint32_t RefClock        = SYSTEM_REFCLK;

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
  SystemCoreClock = SYSTEM_CLOCK;
  PeripheralClock = SYSTEM_CLOCK;
  RefClock        = SYSTEM_REFCLK;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
  SCB->VTOR = (uint32_t) &__Vectors;
#endif

  SCB->CPACR |= ((3U << 10U*2U) |           /* set CP10 Full Access */
                 (3U << 11U*2U)  );         /* set CP11 Full Access */

#ifdef UNALIGNED_SUPPORT_DISABLE
//  SCB->CCR |= SCB_CCR_UNALIGN_TRP_Msk;
#endif

  SystemCoreClock = SYSTEM_CLOCK;
}
