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
	.OTAA_10x.DevEui				= {{ 0x00, 0x3A, 0xB1, 0x24, 0x7C, 0x87, 0x1A, 0x60 }},
	.OTAA_10x.AppEui				= {{ 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x03, 0x99, 0x3B }},
	.OTAA_10x.AppKey				= {{ 0x3E, 0xB6, 0x2B, 0x9E, 0x19, 0x49, 0x7F, 0xFB, 0xAB, 0x85, 0xEF, 0x54, 0x10, 0xC9, 0x45, 0x85 }}
};

#endif /* LORAWAN_KEYS_H */
/* [] END OF FILE */
