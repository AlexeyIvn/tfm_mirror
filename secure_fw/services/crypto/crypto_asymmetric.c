/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include <stddef.h>
#include <stdint.h>

/* FixMe: Use PSA_ERROR_CONNECTION_REFUSED when performing parameter
 *        integrity checks but this will have to be revised
 *        when the full set of error codes mandated by PSA FF
 *        is available.
 */
#include "tfm_mbedcrypto_include.h"

#include "tfm_crypto_api.h"
#include "tfm_crypto_defs.h"

/*!
 * \defgroup public_psa Public functions, PSA
 *
 */

/*!@{*/
psa_status_t tfm_crypto_asymmetric_sign(psa_invec in_vec[],
                                        size_t in_len,
                                        psa_outvec out_vec[],
                                        size_t out_len)
{
    if ((in_len != 2) || (out_len != 1)) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }

    if ((in_vec[0].len != sizeof(struct tfm_crypto_pack_iovec))) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }
    const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;

    psa_key_handle_t handle = iov->key_handle;
    psa_algorithm_t alg = iov->alg;
    const uint8_t *hash = in_vec[1].base;
    size_t hash_length = in_vec[1].len;
    uint8_t *signature = out_vec[0].base;
    size_t signature_size = out_vec[0].len;
    psa_status_t status = tfm_crypto_check_handle_owner(handle, NULL);

    if (status != PSA_SUCCESS) {
        return status;
    }

    return psa_asymmetric_sign(handle, alg, hash, hash_length,
                               signature, signature_size, &(out_vec[0].len));
}

psa_status_t tfm_crypto_asymmetric_verify(psa_invec in_vec[],
                                          size_t in_len,
                                          psa_outvec out_vec[],
                                          size_t out_len)
{
    if ((in_len != 3) || (out_len != 0)) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }

    if ((in_vec[0].len != sizeof(struct tfm_crypto_pack_iovec))) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }
    const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;

    psa_key_handle_t handle = iov->key_handle;
    psa_algorithm_t alg = iov->alg;
    const uint8_t *hash = in_vec[1].base;
    size_t hash_length = in_vec[1].len;
    const uint8_t *signature = in_vec[2].base;
    size_t signature_length = in_vec[2].len;
    psa_status_t status = tfm_crypto_check_handle_owner(handle, NULL);

    if (status != PSA_SUCCESS) {
        return status;
    }

    return psa_asymmetric_verify(handle, alg, hash, hash_length,
                                 signature, signature_length);
}

psa_status_t tfm_crypto_asymmetric_encrypt(psa_invec in_vec[],
                                           size_t in_len,
                                           psa_outvec out_vec[],
                                           size_t out_len)
{
    psa_status_t status;

    if (!((in_len == 2) || (in_len == 3)) || (out_len != 1)) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }

    if ((in_vec[0].len != sizeof(struct tfm_crypto_pack_iovec))) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }
    const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;

    psa_key_handle_t handle = iov->key_handle;
    psa_algorithm_t alg = iov->alg;
    const uint8_t *input = in_vec[1].base;
    size_t input_length = in_vec[1].len;
    const uint8_t *salt = NULL;
    size_t salt_length = 0;
    uint8_t *output = out_vec[0].base;
    size_t output_size = out_vec[0].len;
    psa_key_type_t type;
    size_t key_bits;

    if (in_len == 3) {
        salt = in_vec[2].base;
        salt_length = in_vec[2].len;
    }

    status = tfm_crypto_check_handle_owner(handle, NULL);
    if (status != PSA_SUCCESS) {
        return status;
    }

    status = psa_get_key_information(handle, &type, &key_bits);
    if (status != PSA_SUCCESS) {
        return status;
    }

    /* Check that the output buffer is large enough */
    if (output_size < PSA_ASYMMETRIC_ENCRYPT_OUTPUT_SIZE(type, key_bits, alg)) {
        return PSA_ERROR_BUFFER_TOO_SMALL;
    }

    return psa_asymmetric_encrypt(handle, alg, input, input_length,
                                  salt, salt_length,
                                  output, output_size, &(out_vec[0].len));
}

psa_status_t tfm_crypto_asymmetric_decrypt(psa_invec in_vec[],
                                           size_t in_len,
                                           psa_outvec out_vec[],
                                           size_t out_len)
{
    if (!((in_len == 2) || (in_len == 3)) || (out_len != 1)) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }

    if ((in_vec[0].len != sizeof(struct tfm_crypto_pack_iovec))) {
        return PSA_ERROR_CONNECTION_REFUSED;
    }
    const struct tfm_crypto_pack_iovec *iov = in_vec[0].base;

    psa_key_handle_t handle = iov->key_handle;
    psa_algorithm_t alg = iov->alg;
    const uint8_t *input = in_vec[1].base;
    size_t input_length = in_vec[1].len;
    const uint8_t *salt = NULL;
    size_t salt_length = 0;
    uint8_t *output = out_vec[0].base;
    size_t output_size = out_vec[0].len;
    psa_status_t status;

    if (in_len == 3) {
        salt = in_vec[2].base;
        salt_length = in_vec[2].len;
    }

    status = tfm_crypto_check_handle_owner(handle, NULL);
    if (status != PSA_SUCCESS) {
        return status;
    }

    return psa_asymmetric_decrypt(handle, alg, input, input_length,
                                  salt, salt_length,
                                  output, output_size, &(out_vec[0].len));
}
/*!@}*/
