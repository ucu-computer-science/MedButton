/* ==========================================================
 *    ___             _   _     _			
 *   / _ \ _ __   ___| |_| |__ (_)_ __ __  __
 *  | | | | '_ \ / _ \ __| '_ \| | '_ \\ \/ /
 *  | |_| | | | |  __/ |_| | | | | | | |>  < 
 *   \___/|_| |_|\___|\__|_| |_|_|_| |_/_/\_\
 *									   
 * Copyright Onethinx, 2018
 * All Rights Reserved
 *
 * UNPUBLISHED, LICENSED SOFTWARE.
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Onethinx BV
 *
 * ==========================================================
*/

#ifndef LORAWAN_KEYS_H
#define LORAWAN_KEYS_H

#include "OnethinxCore01.h"

LoRaWAN_keys_t TTN_OTAAkeys = {
	.KeyType 						= OTAA_10x_key,
	.PublicNetwork					= true,
	.OTAA_10x.DevEui				= {{ 0x00, 0xE3, 0x52, 0x07, 0xC8, 0x0A, 0x56, 0x90 }},
	.OTAA_10x.AppEui				= {{ 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x02, 0xB6, 0x3A }},
	.OTAA_10x.AppKey				= {{ 0x8A, 0x9C, 0x9F, 0xAE, 0x12, 0xFE, 0x58, 0x1D, 0xFB, 0xA8, 0x41, 0x0E, 0xD9, 0x2B, 0xCC, 0x0C }}
};

#endif /* LORAWAN_KEYS_H */
/* [] END OF FILE */
