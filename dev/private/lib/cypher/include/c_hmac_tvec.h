/*******************************************************************************
**
**	V300b Updated Board Support Package
**	$Id: v300b_hmac_tvec.h 21519 2013-10-08 00:12:14Z spcorcoran $
**
**	COPYRIGHT INFORMATION:	
**		This software is proprietary and confidential.  
**		By using this software you agree to the terms and conditions of the 
**		associated Lumidigm Inc. License Agreement.
**
**		(c) Copyright 2014 Lumidigm, Inc. (Unpublished Copyright) ALL RIGHTS RESERVED.
**
*******************************************************************************/
#pragma once

/*
** FIPS PUB 198 Self-Test Vectors for SHA-1
**	A.1 SHA1 64B Key
**	A.2 SHA1 20B Key
**  A.3 SHA1 100B Key
**	A.4 SHA1 49B Key Truncated to 12B HMAC
*/

/*
** Known Answer Test 1
** KAT1 SHA-1 64 Byte Key
*/
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT1_KEY[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f 
};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT1_MSG[]   = {0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x23, 0x31};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT1_MAC[] = {
	0x4f, 0x4c, 0xa3, 0xd5, 0xd6, 0x8b, 0xa7, 0xcc, 0x0a, 0x12, 0x08, 0xc9, 0xc6, 0x1e, 0x9c, 0x5d,
	0xa0, 0x40, 0x3c, 0x0a };

/*
**	Known Answer Test 2
** 	SHA-1 20 Byte Key
*/
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT2_KEY[] = {
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,	0x3c, 0x3d, 0x3e, 0x3f, 
	0x40, 0x41, 0x42, 0x43 
	};	

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT2_MSG[] = {0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x23, 0x32};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT2_MAC[] = {
	0x09, 0x22, 0xd3, 0x40, 0x5f, 0xaa, 0x3d, 0x19, 0x4f, 0x82, 0xa4, 0x58, 0x30, 0x73, 0x7d, 0x5c,
	0xc6, 0xc7, 0x5d, 0x24 };

/*
** Known Answer Test 3
** SHA-1 100 Byte Key
*/
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT3_KEY[] = {
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3 };
	
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT3_MSG[] = {0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x23, 0x33};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT3_MAC[] = {
	0xbc, 0xf4, 0x1e, 0xab, 0x8b, 0xb2, 0xd8, 0x02, 0xf3, 0xd0, 0x5c, 0xaf, 0x7c, 0xb0, 0x92, 0xec, 
	0xf8, 0xd1, 0xa3, 0xaa };

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT4_KEY[] = {
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f, 0xa0 };
	
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT4_MSG[] = {0x53, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x20, 0x23, 0x34};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT4_MAC[] = { 0x9e, 0xa8, 0x86, 0xef, 0xe2, 0x68, 0xdb, 0xec, 0xce, 0x42, 0x0c, 0x75 };

// SHA-256 40 Byte Key
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT5_KEY[] = {
	0x6f, 0x35, 0x62, 0x8d, 0x65, 0x81, 0x34, 0x35, 0x53, 0x4b, 0x5d, 0x67, 0xfb, 0xdb, 0x54, 0xcb, 
	0x33, 0x40, 0x3d, 0x04, 0xe8, 0x43, 0x10, 0x3e, 0x63, 0x99, 0xf8, 0x06, 0xcb, 0x5d, 0xf9, 0x5f, 
	0xeb, 0xbd, 0xd6, 0x12, 0x36, 0xf3, 0x32, 0x45
};
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT5_MSG[] = {
	0x75, 0x2c, 0xff, 0x52, 0xe4, 0xb9, 0x07, 0x68, 0x55, 0x8e, 0x53, 0x69, 0xe7, 0x5d, 
	0x97, 0xc6, 0x96, 0x43, 0x50, 0x9a, 0x5e, 0x59, 0x04, 0xe0, 0xa3, 0x86, 0xcb, 0xe4,
	0xd0, 0x97, 0x0e, 0xf7, 0x3f, 0x91, 0x8f, 0x67, 0x59, 0x45, 0xa9, 0xae, 0xfe, 0x26,
	0xda, 0xea, 0x27, 0x58, 0x7e, 0x8d, 0xc9, 0x09, 0xdd, 0x56, 0xfd, 0x04, 0x68, 0x80, 
	0x5f, 0x83, 0x40, 0x39, 0xb3, 0x45, 0xf8, 0x55, 0xcf, 0xe1, 0x9c, 0x44, 0xb5, 0x5a, 
	0xf2, 0x41, 0xff, 0xf3, 0xff, 0xcd, 0x80, 0x45, 0xcd, 0x5c, 0x28, 0x8e, 0x6c, 0x4e, 
	0x28, 0x4c, 0x37, 0x20, 0x57, 0x0b, 0x58, 0xe4, 0xd4, 0x7b, 0x8f, 0xee, 0xed, 0xc5, 
	0x2f, 0xd1, 0x40, 0x1f, 0x69, 0x8a, 0x20, 0x9f, 0xcc, 0xfa, 0x3b, 0x4c, 0x0d, 0x9a, 
	0x79, 0x7b, 0x04, 0x6a, 0x27, 0x59, 0xf8, 0x2a, 0x54, 0xc4, 0x1c, 0xcd, 0x7b, 0x5f, 
	0x59, 0x2b	
};
SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT5_MAC[] = {
	0x05, 0xd1, 0x24, 0x3e, 0x64, 0x65, 0xed, 0x96, 0x20, 0xc9, 0xae, 0xc1, 0xc3, 0x51, 0xa1, 0x86 };


SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT6_KEY[] = {
	0xee, 0x0a, 0x81, 0xa8, 0xbd, 0x52, 0xc9, 0xb1, 0x42, 0x20, 0x83, 0x52, 0x2d, 0x37, 0xf8, 0x07,
	0x18, 0x96, 0xba, 0x62, 0x5f, 0xfa, 0x22, 0xad, 0x32, 0xa4, 0xfd, 0xd1, 0xe8, 0x5c, 0x83, 0x77, 
	0x96, 0xb6, 0x89, 0x6c, 0xe1, 0x94, 0xf7, 0x4a
};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT6_MSG[] = {
	0x0c, 0x24, 0x5d, 0xe3, 0xb2, 0x50, 0xc3, 0x32, 0x82, 0xea, 0x1a, 0x02, 0xd0, 0x07, 0xf0, 0x3b, 
	0x34, 0xed, 0x42, 0x76, 0x31, 0x28, 0x3e, 0xb6, 0x14, 0xdb, 0x4d, 0x52, 0x1f, 0x55, 0x51, 0x36, 
	0xe7, 0xe4, 0x2b, 0x4c, 0xfb, 0xee, 0x81, 0x34, 0xc6, 0x3d, 0xbe, 0x3b, 0xb7, 0x9b, 0x5a, 0x8b, 
	0x9f, 0x9f, 0x5b, 0x9f, 0x5a, 0xc6, 0x1c, 0xfa, 0xb1, 0xc5, 0x4d, 0x19, 0x7f, 0x1e, 0x3b, 0xa6, 
	0x13, 0xf2, 0x51, 0xee, 0xd6, 0x16, 0xdf, 0x95, 0x2d, 0x69, 0x1b, 0x88, 0xa1, 0x64, 0x66, 0x34, 
	0x3e, 0xf2, 0xd0, 0xf6, 0x38, 0x82, 0xdd, 0xd2, 0xd5, 0x5b, 0x8a, 0x67, 0x86, 0x30, 0x8b, 0x22,
	0x57, 0xf5, 0xd7, 0xb3, 0x8a, 0xf1, 0x66, 0xbd, 0x7f, 0x13, 0x39, 0xd2, 0xd8, 0x89, 0x9c, 0x9e, 
	0xda, 0x8f, 0xa8, 0x62, 0x15, 0x85, 0x0b, 0xa5, 0x47, 0x45, 0x0c, 0x26, 0x7e, 0xb3, 0xc9, 0x14
};

SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT6_MAC[] = {
	0x33, 0x5e, 0xe9, 0xa4, 0xc9, 0x6b, 0xfc, 0xfc, 0x38, 0xc7, 0x6f, 0x7a, 0xce, 0x6c, 0x84, 0xad, 
	0xfd, 0x0a, 0x57, 0xa9, 0x4e, 0xfc, 0x23, 0xb2
};


SECTION_SDRAM0_BANK1	// NOT SPEED CRITICAL
u8 KAT1_HMAC[] = {
	0x3B, 0x63, 0x5D, 0x0E, 0x77, 0x9D, 0x1D, 0xB0, 0xFD, 0xE7, 0x3E, 0x85, 0x56, 0x7B, 0x4B, 0xBE,
	0x7E, 0x44, 0x95, 0xCE, 0xC0, 0xE8, 0x3D, 0x76, 0xA7, 0x89, 0x6D, 0xEA, 0x13, 0xD7, 0x27, 0xDB
};

