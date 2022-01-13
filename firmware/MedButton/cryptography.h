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

#ifndef CRYPTOGRAPHY_HEADER_INCLUDED
#define CRYPTOGRAPHY_HEADER_INCLUDED

void encrypt_message(uint8_t* message, uint8_t size, uint8_t* encrypted_msg);

void decrypt_message(uint8_t* message, uint8_t size, uint8_t* encrypted_msg);

void handle_error(void);

#endif