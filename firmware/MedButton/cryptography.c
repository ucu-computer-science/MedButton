#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cy_retarget_io.h"
#define LFSR32_INITSTATE      (0xd8959bc9)
#define LFSR31_INITSTATE      (0x2bb911f8)
#define LFSR29_INITSTATE      (0x060c31b7)

#define MAX_MESSAGE_SIZE                     (100u)
#define AES128_ENCRYPTION_LENGTH             (uint32_t)(16u)
#define AES128_KEY_LENGTH                    (uint32_t)(16u)
#define MAX_PRNG_VALUE                       (255UL)

CY_ALIGN(4) uint8_t message[MAX_MESSAGE_SIZE];
CY_ALIGN(4) uint8_t encrypted_msg[MAX_MESSAGE_SIZE];
CY_ALIGN(4) uint8_t decrypted_msg[MAX_MESSAGE_SIZE];

cy_stc_crypto_aes_state_t aes_state;

CY_ALIGN(4) uint8_t aes_key[16];

cy_en_crypto_status_t cryptoStatus;

void handle_error(void)
{
     /* Disable all interrupts. */
    __disable_irq();

    CY_ASSERT(0);
}

 
void encrypt_message(uint8_t* message, uint8_t size){
    /* All data arrays should be 4-byte aligned */
     uint32_t rndNum = 0;

    /* Enable the Crypto block */
    cy_en_crypto_status_t cryptoStatus;
    cryptoStatus =  Cy_Crypto_Core_Enable(CRYPTO);
     
    if (cryptoStatus == CY_RSLT_SUCCESS){
        for (int i = 0; i < 16; i++){
            cryptoStatus = Cy_Crypto_Core_Prng(CRYPTO, MAX_PRNG_VALUE, &rndNum);
            if (cryptoStatus != CY_RSLT_SUCCESS){
                handle_error();
            }
            uint8_t *uint_ptr = (uint8_t *) &rndNum;
            aes_key[i] = uint_ptr[0];
        }
    }

    uint8_t aes_block_count = 0;
     
    aes_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);
     
    cryptoStatus = Cy_Crypto_Core_Aes_Init(CRYPTO, aes_key, CY_CRYPTO_KEY_AES_128, &aes_state);
     
     if (cryptoStatus == CY_RSLT_SUCCESS) {
          handle_error();
     }

    for (int i = 0; i < aes_block_count ; i++)
    {
        /* Perform AES ECB Encryption mode of operation */
        Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_ENCRYPT,
                               (encrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                               (message + AES128_ENCRYPTION_LENGTH * i),
                                &aes_state);

        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
     }
     
     Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

void decrypt_message(uint8_t* message, uint8_t size) {
    uint8_t aes_block_count = 0;

    aes_block_count =  (size % AES128_ENCRYPTION_LENGTH == 0) ?
                       (size / AES128_ENCRYPTION_LENGTH)
                       : (1 + size / AES128_ENCRYPTION_LENGTH);
     
    cryptoStatus = Cy_Crypto_Core_Aes_Init(CRYPTO, aes_key, CY_CRYPTO_KEY_AES_128, &aes_state);
    if (cryptoStatus == CY_RSLT_SUCCESS) {
          handle_error();
     }
     
     for (int i = 0; i < aes_block_count ; i++)
          {
        /* Perform AES ECB Decryption mode of operation */
        Cy_Crypto_Core_Aes_Ecb(CRYPTO, CY_CRYPTO_DECRYPT,
                               (decrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                               (encrypted_msg + AES128_ENCRYPTION_LENGTH * i),
                               &aes_state);

        /* Wait for Crypto Block to be available */
        Cy_Crypto_Core_WaitForReady(CRYPTO);
    }
     Cy_Crypto_Core_Aes_Free(CRYPTO, &aes_state);
}

/* [] END OF FILE */
