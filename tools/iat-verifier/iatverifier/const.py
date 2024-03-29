# -----------------------------------------------------------------------------
# Copyright (c) 2019, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
# -----------------------------------------------------------------------------

# IAT custom claims
ARM_RANGE = -75000
PROFILE_ID = ARM_RANGE
CLIENT_ID = ARM_RANGE - 1
SECURITY_LIFECYCLE = ARM_RANGE - 2
IMPLEMENTATION_ID = ARM_RANGE - 3
BOOT_SEED = ARM_RANGE - 4
HARDWARE_ID = ARM_RANGE - 5
SW_COMPONENTS = ARM_RANGE - 6
NO_MEASUREMENTS = ARM_RANGE - 7
CHALLENGE = ARM_RANGE - 8  # nonce
INSTANCE_ID = ARM_RANGE - 9  # UEID
ORIGINATOR = ARM_RANGE - 10  # originator


# SW component IDs
SW_COMPONENT_RANGE = 0
SW_COMPONENT_TYPE = SW_COMPONENT_RANGE + 1
MEASUREMENT_VALUE = SW_COMPONENT_RANGE + 2
SW_COMPONENT_VERSION = SW_COMPONENT_RANGE + 4
SIGNER_ID = SW_COMPONENT_RANGE + 5
MEASUREMENT_DESCRIPTION = SW_COMPONENT_RANGE + 6


NAMES = {
    INSTANCE_ID: 'INSTANCE_ID',
    CHALLENGE: 'CHALLENGE',
    IMPLEMENTATION_ID: 'IMPLEMENTATION_ID',
    SW_COMPONENTS: 'SW_COMPONENTS',
    SW_COMPONENT_TYPE: 'SW_COMPONENT_TYPE',
    SW_COMPONENT_VERSION: 'SW_COMPONENT_VERSION',
    CLIENT_ID: 'CLIENT_ID',
    SECURITY_LIFECYCLE: 'SECURITY_LIFECYCLE',
    BOOT_SEED: 'BOOT_SEED',
    SIGNER_ID: 'SIGNER_ID',
    NO_MEASUREMENTS: 'NO_MEASUREMENTS',
    MEASUREMENT_VALUE: 'MEASUREMENT_VALUE',
    MEASUREMENT_DESCRIPTION: 'MEASUREMENT_DESCRIPTION',
    HARDWARE_ID: 'HARDWARE_ID',
    ORIGINATOR: 'ORIGINATOR',
    PROFILE_ID: 'PROFILE_ID',
}


IS_UTF_8 = ['SW_COMPONENT_TYPE', 'SW_COMPONENT_VERSION',
            'MEASUREMENT_DESCRIPTION', 'ORIGNATOR', 'PROFILE_ID']


MANDATORY_CLAIMS = [
    INSTANCE_ID,
    IMPLEMENTATION_ID,
    CLIENT_ID,
    CHALLENGE,
    SECURITY_LIFECYCLE,
    PROFILE_ID,
    BOOT_SEED,
]

MANDATORY_SW_COMPONENT_CLAIMS = [
    SW_COMPONENT_TYPE,
    MEASUREMENT_VALUE,
]

ALLOWED_SW_COMPONENT_CLAIMS = [
    SW_COMPONENT_TYPE,
    SW_COMPONENT_VERSION,
    MEASUREMENT_VALUE,
    MEASUREMENT_DESCRIPTION,
    SIGNER_ID,
]


HASH_SIZES = [32, 48, 64]


# Security Lifecycle claims
SL_UNKNOWN = 0x1000
SL_PSA_ROT_PROVISIONING = 0x2000
SL_SECURED = 0x3000
SL_NON_PSA_ROT_DEBUG = 0x4000
SL_RECOVERABLE_PSA_ROT_DEBUG = 0x5000
SL_PSA_LIFECYCLE_DECOMMISSIONED = 0x6000


SL_NAMES = [
    'SL_UNKNOWN',
    'SL_PSA_ROT_PROVISIONING',
    'SL_SECURED',
    'SL_NON_PSA_ROT_DEBUG',
    'SL_RECOVERABLE_PSA_ROT_DEBUG',
    'SL_PSA_LIFECYCLE_DECOMMISSIONED',
]

SL_SHIFT = 12
