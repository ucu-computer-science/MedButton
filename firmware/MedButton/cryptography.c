#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#define LFSR32_INITSTATE      (0xd8959bc9)
#define LFSR31_INITSTATE      (0x2bb911f8)
#define LFSR29_INITSTATE      (0x060c31b7)
#define MAX_PRNG_VALUE        (255UL)

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

 
uint8_t *encode(uint8_t *message, const uint32_t message_size){
    /* All data arrays should be 4-byte aligned */
    CY_ALIGN(4) uint8_t aesKey[CY_CRYPTO_AES_128_KEY_SIZE] = {
        0x2Bu, 0x7Eu, 0x15u, 0x16u, 0x28u, 0xAEu, 0xD2u, 0xA6u,
        0xABu, 0xF7u, 0x15u, 0x88u, 0x09u, 0xCFu, 0x4Fu, 0x3Cu
    };
    // CY_ALIGN(4) message; we need to align the message
    CY_ALIGN(4) uint8_t aesCbcCipherText[message_size];
    CY_ALIGN(4) uint32_t IV[16];

    /* Generated Random Number */
    uint32_t rndNum = 0;
    cy_stc_crypto_context_prng_t  cryptoPrngContext;
    cy_en_crypto_status_t         cryptoStatus;
    cryptoStatus = Cy_Crypto_Prng_Init(
                            LFSR32_INITSTATE,
                            LFSR31_INITSTATE,
                            LFSR29_INITSTATE, &cryptoPrngContext);
    if (cryptoStatus == CY_RSLT_SUCCESS){
        for (int i = 0; i < 16; i++){
            cryptoStatus = Cy_Crypto_Prng_Generate(MAX_PRNG_VALUE, &rndNum, &cryptoPrngContext);
            if (cryptoStatus != CY_RSLT_SUCCESS){
                break;
            }
            IV[i] = rndNum;
        }
    }

    cy_stc_crypto_context_aes_t   cryptoAesContext;
    /* Initialize Crypto AES functionality  */
    cryptoStatus = Cy_Crypto_Aes_Init(
                        (uint32_t *)aesKey,
                        CY_CRYPTO_KEY_AES_128,
                        &cryptoAesContext);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    /* Wait crypto become available */
    cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    /* Encrypt */ 
    cryptoStatus = Cy_Crypto_Aes_Cbc_Run(
                        CY_CRYPTO_ENCRYPT,
                        message_size,
                        &IV,
                        &aesCbcCipherText,
                        message,
                        &cryptoAesContext);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    /* Wait crypto become available */
    cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    return aesCbcCipherText;
}

uint8_t *decode(uint8_t *cipher_message, const uint32_t message_size, uint8_t *IV, cy_stc_crypto_context_aes_t cryptoAesContext){
    CY_ALIGN(4) uint8_t decoded_message[message_size];
    cy_en_crypto_status_t         cryptoStatus;

    /* Wait crypto become available */
    cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    /* Decrypt */ 
    cryptoStatus = Cy_Crypto_Aes_Cbc_Run(
                        CY_CRYPTO_DECRYPT,
                        message_size,
                        &IV,
                        &decoded_message,
                        cipher_message,
                        &cryptoAesContext);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    /* Wait crypto become available */
    cryptoStatus = Cy_Crypto_Sync(CY_CRYPTO_SYNC_BLOCKING);
    if (cryptoStatus != CY_RSLT_SUCCESS){
        /* ... error... */
        return 0;
    }
    return decoded_message;
}

// int main(void)
// {
    
// }

/* [] END OF FILE */
