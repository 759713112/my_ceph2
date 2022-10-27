/*******************************************************************************
  Copyright (c) 2009-2019, Intel Corporation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/**
 ******************************************************************************
 * @file zuc_internal.h
 *
 * @description
 *      This header file defines the internal API's and data types for the
 *      3GPP algorithm ZUC.
 *
 *****************************************************************************/

#ifndef ZUC_INTERNAL_H_
#define ZUC_INTERNAL_H_

#include <stdio.h>
#include <stdint.h>

#include "intel-ipsec-mb.h"
#include "immintrin.h"
#include "include/wireless_common.h"

/* 64 bytes of Keystream will be generated */
#define ZUC_KEYSTR_LEN                      (64)
#define NUM_LFSR_STATES                     (16)
#define ZUC_WORD                            (32)

/* Range of input data for ZUC is from 1 to 65504 bits */
#define ZUC_MIN_LEN     1
#define ZUC_MAX_LEN     65504

#ifdef DEBUG
#ifdef _WIN32
#define DEBUG_PRINT(_fmt, ...) \
        fprintf(stderr, "%s()::%d " _fmt , __FUNCTION__, __LINE__, __VA_ARGS__)
#else
#define DEBUG_PRINT(_fmt, ...) \
        fprintf(stderr, "%s()::%d " _fmt , __func__, __LINE__, __VA_ARGS__)
#endif
#else
#define DEBUG_PRINT(_fmt, ...)
#endif

/**
 ******************************************************************************
 * @description
 *      Macro will loop through keystream of length 64bytes and xor with the
 *      input buffer placing the result in the output buffer.
 *      KeyStream bytes must be swaped on 32bit boundary before this operation
 *
 *****************************************************************************/
#define ZUC_XOR_KEYSTREAM(pIn64, pOut64, pKeyStream64)		\
{									\
	int i =0;							\
	union SwapBytes_t {						\
		uint64_t l64;						\
		uint32_t w32[2];					\
	}swapBytes;							\
	/* loop through the key stream and xor 64 bits at a time */	\
	for(i =0; i < ZUC_KEYSTR_LEN/8; i++) {				\
		swapBytes.l64 = *pKeyStream64++;			\
		swapBytes.w32[0] = bswap4(swapBytes.w32[0]); \
		swapBytes.w32[1] = bswap4(swapBytes.w32[1]); \
		*pOut64++ = *pIn64++ ^ swapBytes.l64;			\
	}								\
}

/**
 *****************************************************************************
 * @description
 *      Packed structure to store the ZUC state for a single packet. *
 *****************************************************************************/
typedef struct zuc_state_s {
    uint32_t lfsrState[16];
    /**< State registers of the LFSR */
    uint32_t fR1;
    /**< register of F */
    uint32_t fR2;
    /**< register of F */
    uint32_t bX0;
    /**< Output X0 of the bit reorganization */
    uint32_t bX1;
    /**< Output X1 of the bit reorganization */
    uint32_t bX2;
    /**< Output X2 of the bit reorganization */
    uint32_t bX3;
    /**< Output X3 of the bit reorganization */
} ZucState_t;

/**
 *****************************************************************************
 * @description
 *      Packed structure to store the ZUC state for a single packet. *
 *****************************************************************************/
typedef struct zuc_state_4_s {
    uint32_t lfsrState[16][4];
    /**< State registers of the LFSR */
    uint32_t fR1[4];
    /**< register of F */
    uint32_t fR2[4];
    /**< register of F */
    uint32_t bX0[4];
    /**< Output X0 of the bit reorganization for 4 packets */
    uint32_t bX1[4];
    /**< Output X1 of the bit reorganization for 4 packets */
    uint32_t bX2[4];
    /**< Output X2 of the bit reorganization for 4 packets */
    uint32_t bX3[4];
    /**< Output X3 of the bit reorganization for 4 packets */
} ZucState4_t;

/**
 *****************************************************************************
 * @description
 *      Structure to store pointers to the 4 keys to be used as input to
 *      @ref asm_ZucInitialization_4 and @ref asm_ZucGenKeystream64B_4
 *****************************************************************************/
typedef struct zuc_key_4_s {
    const uint8_t *pKey1;
    /**< Pointer to 128-bit key for packet 1 */
    const uint8_t *pKey2;
    /**< Pointer to 128-bit key for packet 2 */
    const uint8_t *pKey3;
    /**< Pointer to 128-bit key for packet 3 */
    const uint8_t *pKey4;
    /**< Pointer to 128-bit key for packet 4 */
} ZucKey4_t;

/**
 *****************************************************************************
 * @description
 *      Structure to store pointers to the 4 IV's to be used as input to
 *      @ref asm_ZucInitialization_4 and @ref asm_ZucGenKeystream64B_4
 *****************************************************************************/
typedef struct zuc_iv_4_s {
    const uint8_t *pIv1;
    /**< Pointer to 128-bit initialization vector for packet 1 */
    const uint8_t *pIv2;
    /**< Pointer to 128-bit initialization vector for packet 2 */
    const uint8_t *pIv3;
    /**< Pointer to 128-bit initialization vector for packet 3 */
    const uint8_t *pIv4;
    /**< Pointer to 128-bit initialization vector for packet 4 */
} ZucIv4_t;

/**
 ******************************************************************************
 *
 * @description
 *      Definition of the external function that implements the initialization
 *      stage of the ZUC algorithm. The function will initialize the state
 *      for a single packet operation.
 *
 * @param[in] pKey                  Pointer to the 128-bit initial key that
 *                                  will be used when initializing the ZUC
 *                                  state.
 * @param[in] pIv                   Pointer to the 128-bit initial vector that
 *                                  will be used when initializing the ZUC
 *                                  state.
 * @param[in,out] pState            Pointer to a ZUC state structure of type
 *                                  @ref ZucState_t that will be populated
 *                                  with the initialized ZUC state.
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL void asm_ZucInitialization(const void *pKey,
                                         const void *pIv,
                                         ZucState_t *pState);

/**
 ******************************************************************************
 * @description
 *      Definition of the external function that implements the initialization
 *      stage of the ZUC algorithm for 4 packets. The function will initialize
 *      the state for 4 individual packets.
 *
 * @param[in] pKey                  Pointer to an array of 128-bit initial keys
 *                                  that will be used when initializing the ZUC
 *                                  state.
 * @param[in] pIv                   Pointer to an array of 128-bit initial
 *                                  vectors that will be used when initializing
 *                                  the ZUC state.
 * @param[in,out] pState            Pointer to a ZUC state structure of type
 *                                  @ref ZucState4_t that will be populated
 *                                  with the initialized ZUC state.
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL void asm_ZucInitialization_4_sse(ZucKey4_t *pKeys,
                                               ZucIv4_t *pIvs,
                                               ZucState4_t *pState);

IMB_DLL_LOCAL void asm_ZucInitialization_4_avx(ZucKey4_t *pKeys,
                                               ZucIv4_t *pIvs,
                                               ZucState4_t *pState);

/**
 ******************************************************************************
 *
 * @description
 *      Definition of the external function that implements the working
 *      stage of the ZUC algorithm. The function will generate 64 bytes of
 *      keystream.
 *
 * @param[in,out] pKeystream        Pointer to an input buffer that will
 *                                  contain the generated keystream.

 * @param[in] pState                Pointer to a ZUC state structure of type
 *                                  @ref ZucState_t
 *
 * @pre
 *      A successful call to @ref asm_ZucInitialization to initialize the ZUC
 *      state.
 *
 *****************************************************************************/
IMB_DLL_LOCAL void asm_ZucGenKeystream64B(uint32_t *pKeystream,
                                          ZucState_t *pState);

/**
 ******************************************************************************
 *
 * @description
 *      Definition of the external function that implements the working
 *      stage of the ZUC algorithm. The function will generate 8 bytes of
 *      keystream.
 *
 * @param[in,out] pKeystream        Pointer to an input buffer that will
 *                                  contain the generated keystream.

 * @param[in] pState                Pointer to a ZUC state structure of type
 *                                  @ref ZucState_t
 *
 * @pre
 *      A successful call to @ref asm_ZucInitialization to initialize the ZUC
 *      state.
 *
 *****************************************************************************/
IMB_DLL_LOCAL void asm_ZucGenKeystream8B(void *pKeystream,
                                         ZucState_t *pState);

/**
 ******************************************************************************
 *
 * @description
 *      Definition of the external function that implements the working
 *      stage of the ZUC algorithm. The function will generate 64 bytes of
 *      keystream for four packets in parallel.
 *
 * @param[in] pState                Pointer to a ZUC state structure of type
 *                                  @ref ZucState4_t
 *
 * @param[in,out] pKeyStr1          Pointer to an input buffer that will
 *                                  contain the generated keystream for packet
 *                                  one.
 * @param[in,out] pKeyStr2          Pointer to an input buffer that will
 *                                  contain the generated keystream for packet
 *                                  two.
 * @param[in,out] pKeyStr3          Pointer to an input buffer that will
 *                                  contain the generated keystream for packet
 *                                  three.
 * @param[in,out] pKeyStr4          Pointer to an input buffer that will
 *                                  contain the generated keystream for packet
 *                                  four.
 *
 * @pre
 *      A successful call to @ref asm_ZucInitialization_4 to initialize the ZUC
 *      state.
 *
 *****************************************************************************/
IMB_DLL_LOCAL void asm_ZucGenKeystream64B_4_sse(ZucState4_t *pState,
                                                uint32_t *pKeyStr1,
                                                uint32_t *pKeyStr2,
                                                uint32_t *pKeyStr3,
                                                uint32_t *pKeyStr4);

IMB_DLL_LOCAL void asm_ZucGenKeystream64B_4_avx(ZucState4_t *pState,
                                                uint32_t *pKeyStr1,
                                                uint32_t *pKeyStr2,
                                                uint32_t *pKeyStr3,
                                                uint32_t *pKeyStr4);

/**
 ******************************************************************************
 * @description
 *      Definition of the external function to update the authentication tag
 *      based on keystream and data (SSE varient)
 *
 * @param[in] T                     Authentication tag
 *
 * @param[in] ks                    Pointer to key stream
 *
 * @param[in] data                  Pointer to the data
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL uint32_t asm_Eia3Round64BSSE(uint32_t T, const void *ks,
                                           const void *data);

/**
 ******************************************************************************
 * @description
 *      Definition of the external function to return the authentication
 *      update value to be XOR'ed with current authentication tag (SSE variant)
 *
 * @param[in] ks                    Pointer to key stream
 *
 * @param[in] data                  Pointer to the data
 *
 * @param[in] n_words               Number of data bits to be processed
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL uint32_t asm_Eia3RemainderSSE(const void *ks, const void *data,
                                            const uint64_t n_words);

/**
 ******************************************************************************
 * @description
 *      Definition of the external function to update the authentication tag
 *      based on keystream and data (AVX variant)
 *
 * @param[in] T                     Authentication tag
 *
 * @param[in] ks                    Pointer to key stream
 *
 * @param[in] data                  Pointer to the data
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL uint32_t asm_Eia3Round64BAVX(uint32_t T, const void *ks,
                                           const void *data);

/**
 ******************************************************************************
 * @description
 *      Definition of the external function to return the authentication
 *      update value to be XOR'ed with current authentication tag (AVX variant)
 *
 * @param[in] ks                    Pointer to key stream
 *
 * @param[in] data                  Pointer to the data
 *
 * @param[in] n_words               Number of data bits to be processed
 *
 * @pre
 *      None
 *
 *****************************************************************************/
IMB_DLL_LOCAL uint32_t asm_Eia3RemainderAVX(const void *ks, const void *data,
                                            const uint64_t n_words);


/* the s-boxes */
extern const uint8_t S0[256];
extern const uint8_t S1[256];

void zuc_eea3_1_buffer_sse(const void *pKey, const void *pIv,
                           const void *pBufferIn, void *pBufferOut,
                           const uint32_t lengthInBytes);

void zuc_eea3_4_buffer_sse(const void * const pKey[4],
                           const void * const pIv[4],
                           const void * const pBufferIn[4],
                           void *pBufferOut[4],
                           const uint32_t lengthInBytes[4]);

void zuc_eea3_n_buffer_sse(const void * const pKey[], const void * const pIv[],
                           const void * const pBufferIn[], void *pBufferOut[],
                           const uint32_t lengthInBytes[],
                           const uint32_t numBuffers);

void zuc_eia3_1_buffer_sse(const void *pKey, const void *pIv,
                           const void *pBufferIn, const uint32_t lengthInBits,
                           uint32_t *pMacI);

void zuc_eea3_1_buffer_avx(const void *pKey, const void *pIv,
                           const void *pBufferIn, void *pBufferOut,
                           const uint32_t lengthInBytes);

void zuc_eea3_4_buffer_avx(const void * const pKey[4],
                           const void * const pIv[4],
                           const void * const pBufferIn[4],
                           void *pBufferOut[4],
                           const uint32_t lengthInBytes[4]);

void zuc_eea3_n_buffer_avx(const void * const pKey[], const void * const pIv[],
                           const void * const pBufferIn[], void *pBufferOut[],
                           const uint32_t lengthInBytes[],
                           const uint32_t numBuffers);

void zuc_eia3_1_buffer_avx(const void *pKey, const void *pIv,
                           const void *pBufferIn, const uint32_t lengthInBits,
                           uint32_t *pMacI);


#endif /* ZUC_INTERNAL_H_ */

