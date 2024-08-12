#include "S7V30.h"
// �� ������ ������ ����� ����� !!!
const unsigned char AESKey[AESKey_SIZE] __attribute__ ((aligned (4)))= 
{
  0x45,0x97,0x5F,0xA2,0x3B,0x20,0x46,0x34,0x64,0x44,0x6B,0x27,0xC7,0x8C,0xE7,0xA5,
  0xAB,0x5E,0x6C,0x33,0x55,0x78,0xB7,0x06,0x9E,0x50,0xBF,0xD0,0x65,0x5B,0x4B,0xC9
};
 
const unsigned char AES_init_vector[AES_init_vector_SIZE] __attribute__ ((aligned (4)))=
{
  0x7E,0x19,0x46,0x65,0x1E,0x27,0xCB,0x50,0xB6,0x35,0xE4,0xAC,0x00,0x00,0x00,0x01
};
 
const unsigned char AES_aad[AES_aad_SIZE] __attribute__ ((aligned (4)))=
{
  0x9A,0xA0,0x91,0xE0,0x38,0x88,0x86,0xD4
};
 
const unsigned char RSA_public_key[RSA_public_key_SIZE] __attribute__ ((aligned (4)))= // DER encoded RSA public key
{
  0x30,0x82,0x01,0x0A,0x02,0x82,0x01,0x01,0x00,0x9F,0x51,0x4F,0x4A,0xC9,0x7C,0x79,
  0x42,0x57,0x1B,0x48,0x65,0x60,0x3E,0x90,0xDF,0xB8,0x92,0x3F,0x8A,0x5C,0x77,0x4E,
  0x3A,0x9A,0xAD,0x3F,0x0D,0xFB,0x4F,0x12,0x8B,0x49,0x56,0xDC,0x59,0x83,0xA8,0x3B,
  0x03,0xED,0x7D,0xBE,0x84,0xEA,0xEC,0x5E,0xDC,0xEE,0x6D,0xA6,0x3C,0xF4,0x1C,0xE4,
  0x27,0x03,0x15,0x34,0xA7,0xDD,0x5A,0x9B,0xCA,0x23,0x2E,0x13,0xE7,0x25,0x19,0xDD,
  0x78,0x3F,0x1B,0xC1,0x6D,0xFD,0xE7,0x6D,0x69,0xA0,0x69,0x05,0xFA,0x3D,0xF3,0x43,
  0xD8,0x96,0xC9,0xAE,0xFB,0x63,0x04,0x82,0xF9,0x94,0xBA,0x19,0x9C,0xBF,0x60,0x7D,
  0xB0,0x52,0xFC,0xB6,0x03,0xC5,0x85,0xD6,0x41,0x6E,0xF0,0xF4,0xB2,0x70,0x58,0x28,
  0x60,0xE2,0x27,0xA0,0x27,0x6D,0x2A,0x08,0xD5,0x4F,0x29,0x45,0x50,0xFE,0x6D,0x98,
  0xB5,0xFE,0xFD,0xE1,0xC0,0x7E,0xD2,0xC5,0xD4,0xCE,0x4A,0x07,0x98,0x1F,0x7D,0x2A,
  0x14,0xF7,0x8A,0x43,0xB8,0x21,0x2E,0x19,0x2E,0x28,0xAB,0x24,0x70,0x05,0x0F,0xD9,
  0x97,0x89,0xB3,0x05,0x62,0x6E,0xE5,0xBD,0xA4,0x73,0xD9,0xD2,0x78,0x30,0x95,0xA4,
  0xFF,0x05,0xF0,0x21,0x46,0x63,0x97,0x6B,0x33,0x66,0x1C,0xE0,0xC1,0x3F,0xC1,0x7A,
  0xD2,0x2D,0x8C,0x26,0x23,0x1B,0xC6,0xE7,0x35,0x0F,0xB0,0x6F,0xFB,0x7F,0x30,0x27,
  0xFF,0x0C,0x07,0x7D,0xD8,0xDD,0x47,0x8A,0x10,0x9D,0xFE,0x5E,0x1E,0x3F,0x89,0x1A,
  0x76,0xB1,0x13,0xEF,0x1F,0x54,0x71,0x7C,0x89,0x17,0xD1,0x1B,0x08,0x82,0x5D,0x17,
  0x28,0xB2,0xC3,0x2D,0x18,0x5F,0x97,0x1D,0xC5,0x02,0x03,0x01,0x00,0x01
};
 
const unsigned char RSA_private_key[RSA_private_key_SIZE] __attribute__ ((aligned (4)))= // DER encoded RSA private key
{
  0x30,0x82,0x04,0xA5,0x02,0x01,0x00,0x02,0x82,0x01,0x01,0x00,0x9F,0x51,0x4F,0x4A,
  0xC9,0x7C,0x79,0x42,0x57,0x1B,0x48,0x65,0x60,0x3E,0x90,0xDF,0xB8,0x92,0x3F,0x8A,
  0x5C,0x77,0x4E,0x3A,0x9A,0xAD,0x3F,0x0D,0xFB,0x4F,0x12,0x8B,0x49,0x56,0xDC,0x59,
  0x83,0xA8,0x3B,0x03,0xED,0x7D,0xBE,0x84,0xEA,0xEC,0x5E,0xDC,0xEE,0x6D,0xA6,0x3C,
  0xF4,0x1C,0xE4,0x27,0x03,0x15,0x34,0xA7,0xDD,0x5A,0x9B,0xCA,0x23,0x2E,0x13,0xE7,
  0x25,0x19,0xDD,0x78,0x3F,0x1B,0xC1,0x6D,0xFD,0xE7,0x6D,0x69,0xA0,0x69,0x05,0xFA,
  0x3D,0xF3,0x43,0xD8,0x96,0xC9,0xAE,0xFB,0x63,0x04,0x82,0xF9,0x94,0xBA,0x19,0x9C,
  0xBF,0x60,0x7D,0xB0,0x52,0xFC,0xB6,0x03,0xC5,0x85,0xD6,0x41,0x6E,0xF0,0xF4,0xB2,
  0x70,0x58,0x28,0x60,0xE2,0x27,0xA0,0x27,0x6D,0x2A,0x08,0xD5,0x4F,0x29,0x45,0x50,
  0xFE,0x6D,0x98,0xB5,0xFE,0xFD,0xE1,0xC0,0x7E,0xD2,0xC5,0xD4,0xCE,0x4A,0x07,0x98,
  0x1F,0x7D,0x2A,0x14,0xF7,0x8A,0x43,0xB8,0x21,0x2E,0x19,0x2E,0x28,0xAB,0x24,0x70,
  0x05,0x0F,0xD9,0x97,0x89,0xB3,0x05,0x62,0x6E,0xE5,0xBD,0xA4,0x73,0xD9,0xD2,0x78,
  0x30,0x95,0xA4,0xFF,0x05,0xF0,0x21,0x46,0x63,0x97,0x6B,0x33,0x66,0x1C,0xE0,0xC1,
  0x3F,0xC1,0x7A,0xD2,0x2D,0x8C,0x26,0x23,0x1B,0xC6,0xE7,0x35,0x0F,0xB0,0x6F,0xFB,
  0x7F,0x30,0x27,0xFF,0x0C,0x07,0x7D,0xD8,0xDD,0x47,0x8A,0x10,0x9D,0xFE,0x5E,0x1E,
  0x3F,0x89,0x1A,0x76,0xB1,0x13,0xEF,0x1F,0x54,0x71,0x7C,0x89,0x17,0xD1,0x1B,0x08,
  0x82,0x5D,0x17,0x28,0xB2,0xC3,0x2D,0x18,0x5F,0x97,0x1D,0xC5,0x02,0x03,0x01,0x00,
  0x01,0x02,0x82,0x01,0x00,0x0F,0x0B,0xE1,0x56,0xB9,0x57,0xC1,0xA6,0x03,0xCF,0x06,
  0x53,0xD8,0xB6,0x9F,0xFD,0x19,0x13,0xEA,0xDF,0x58,0xE3,0x45,0xA6,0xA0,0x44,0x1B,
  0x6F,0xE9,0x1B,0xAB,0x2F,0x2F,0x72,0x7E,0x26,0x10,0x1A,0x9B,0xD4,0x3F,0xA2,0x40,
  0x13,0x92,0x92,0x57,0x55,0x3B,0x49,0x6C,0xC9,0x55,0xEE,0xC2,0x97,0x72,0xA1,0x96,
  0x93,0x02,0x03,0x54,0x32,0x4F,0x87,0x43,0x06,0xF2,0x08,0x92,0xD3,0x8F,0xF6,0x41,
  0x9A,0xEF,0x3F,0x63,0xA4,0x75,0xAB,0x65,0x8E,0xA0,0x3A,0x30,0xDE,0xAC,0x43,0x08,
  0xFD,0x21,0xF9,0x63,0x9B,0x49,0xB2,0x17,0xC8,0x27,0x84,0xE7,0xE9,0xF6,0xB4,0x7A,
  0x46,0x39,0xE9,0xED,0x75,0xC9,0x92,0x05,0x32,0x33,0x0D,0x08,0x67,0x3A,0x04,0xAF,
  0x09,0xFA,0x33,0xDD,0x98,0x8D,0x95,0xC0,0xE8,0x20,0x5C,0x71,0x40,0x8F,0xFB,0x77,
  0x8C,0xF9,0xF0,0x86,0xEF,0xC8,0x9F,0x94,0x82,0x93,0x17,0xC0,0x90,0x80,0xE8,0x7C,
  0x84,0x3E,0x99,0x70,0x4B,0xD1,0x96,0x51,0x68,0xDA,0xD4,0xAB,0xF3,0x91,0x1E,0xEB,
  0xEA,0x84,0xF4,0xA3,0x29,0xEA,0xBE,0x03,0x5A,0xAA,0x5A,0xA2,0x8C,0x17,0xD2,0xE0,
  0x9A,0xB7,0x9A,0x68,0xA6,0x49,0xBE,0xA1,0xFC,0xCC,0x92,0x03,0x3E,0x2B,0xDB,0x21,
  0x83,0x82,0xE0,0x3B,0x61,0x3A,0x62,0xDA,0x38,0x21,0x21,0xE1,0xDC,0xEE,0xC9,0x80,
  0xA0,0xF2,0x8A,0x8B,0x90,0xF3,0x56,0x2D,0x75,0xE9,0x81,0x68,0x0B,0x89,0xE8,0x19,
  0x9F,0x5B,0x1F,0xBE,0x5E,0x59,0xD1,0xE4,0x65,0x43,0x7F,0xA7,0xAD,0x5F,0x9B,0xEF,
  0x2B,0x64,0xA2,0x9C,0x43,0x02,0x81,0x81,0x00,0xCD,0x9A,0x02,0xB9,0x2C,0x8D,0x8D,
  0xB5,0xCF,0xBE,0xEF,0x4E,0x58,0x40,0x28,0x3D,0xE3,0x62,0xD1,0x41,0x08,0x58,0x16,
  0x85,0xDD,0xC9,0x27,0xA9,0x27,0xC3,0x55,0x48,0x72,0x59,0xCA,0xDF,0xA4,0x3A,0xD2,
  0xC9,0x24,0xDA,0x5D,0x81,0x52,0x45,0xB4,0x17,0x6E,0x90,0xBB,0x08,0x9C,0x81,0x67,
  0x33,0x02,0xC0,0x11,0x06,0xD7,0x55,0xCF,0x4A,0x6F,0x1E,0xEF,0x80,0xD6,0x48,0xEC,
  0x5D,0x70,0xF7,0x1D,0x34,0x65,0x7B,0x5F,0x1E,0x65,0x3C,0x27,0x50,0x56,0x35,0x1F,
  0x64,0xAE,0x26,0xA4,0xAF,0xF3,0xEC,0x30,0x4F,0x83,0xAF,0xA9,0xD4,0x40,0x02,0xCB,
  0x1F,0x1C,0x5C,0xD1,0x38,0xCA,0x7B,0x8E,0xE0,0x51,0x2A,0x49,0x8F,0xB8,0x53,0x4B,
  0x88,0xF0,0x5E,0xEB,0x53,0x46,0xDE,0x2B,0xEB,0x02,0x81,0x81,0x00,0xC6,0x5E,0xDE,
  0x6C,0x56,0xC8,0xAB,0x6F,0xF8,0xD0,0x7E,0x29,0xDF,0xB3,0x9B,0x08,0x73,0x06,0x99,
  0x69,0x9D,0x98,0x07,0xF2,0x63,0x26,0x12,0xC8,0xC2,0xA0,0xAE,0xCA,0xFA,0x3F,0x1C,
  0x2D,0x94,0x3D,0xF2,0x38,0x36,0x21,0x58,0x58,0x86,0x46,0x12,0xD5,0x82,0x32,0x5C,
  0x2B,0x5D,0xD7,0xDB,0xFD,0xAB,0xC0,0xCE,0x9A,0xFC,0xB0,0x71,0xC1,0x73,0x08,0xF4,
  0x90,0x31,0x99,0xD1,0x63,0xE0,0xD2,0x70,0x93,0x5F,0x4D,0xDB,0x9E,0x11,0x95,0x3E,
  0x64,0x2B,0xD0,0x34,0x83,0xC1,0xCE,0xC1,0xB0,0x9B,0x9E,0xCE,0x0F,0x51,0xB9,0x9B,
  0xC7,0x81,0xF0,0xFF,0x35,0x6B,0x54,0x19,0x4F,0xB5,0x48,0x12,0x02,0xD9,0xDF,0xC8,
  0x82,0xC8,0x38,0xC6,0x04,0xBF,0xE0,0x68,0xC2,0x5A,0xAB,0xE1,0x0F,0x02,0x81,0x81,
  0x00,0xC4,0x43,0xED,0xAF,0x98,0xF1,0x33,0xC2,0xC8,0x22,0xEB,0x4C,0x6D,0x80,0x29,
  0x2A,0x13,0x03,0x44,0xE0,0x00,0xB8,0x1E,0x20,0x01,0xFC,0xC1,0x08,0x7F,0xF5,0xBE,
  0x56,0x9F,0x1A,0x4A,0x48,0xBC,0xFC,0x8E,0x35,0x07,0x22,0x10,0xBA,0x9F,0xAF,0xFC,
  0x5F,0x95,0xE3,0x41,0x37,0xD2,0x36,0xD1,0x2D,0xE7,0xC5,0x33,0x3B,0xC5,0x99,0x9B,
  0x7C,0xE1,0xA0,0xB5,0x5E,0xD6,0xAF,0xDC,0x3A,0xC2,0xD8,0x92,0x9F,0x6E,0xF4,0x9B,
  0x73,0x67,0x8E,0x49,0xB5,0x2D,0xE6,0x1E,0x26,0xBC,0xFC,0x14,0xA0,0x06,0x88,0x84,
  0x01,0x93,0xA2,0xBF,0xCA,0xE6,0xA4,0x04,0x04,0x99,0xCD,0x9C,0xC7,0xCA,0xBA,0xB2,
  0x44,0x3E,0x16,0xD1,0x0C,0x0A,0x18,0x31,0x75,0xF2,0x87,0x99,0xBA,0xF0,0x5E,0xAB,
  0xAB,0x02,0x81,0x81,0x00,0xA7,0x9D,0x61,0xB3,0x89,0x60,0xE5,0xFB,0x26,0xF2,0x84,
  0xA8,0x3D,0x2D,0x35,0xC2,0x5D,0xEF,0x25,0x91,0xE8,0x67,0x9D,0xCF,0x61,0x97,0xBA,
  0x27,0x88,0xCC,0xDC,0xE5,0x05,0xAE,0x6D,0xD0,0xD5,0x81,0xC2,0x39,0x5D,0x03,0xAF,
  0xF2,0x9D,0x02,0x78,0xC7,0x8F,0xCB,0x9D,0xB5,0xC7,0x05,0xAE,0x0D,0x1A,0x23,0x40,
  0x70,0x15,0x33,0xC1,0xEB,0x03,0x98,0x7D,0x8D,0x99,0x5B,0xAF,0x62,0x05,0x85,0xCB,
  0x39,0xBB,0x3D,0xBE,0x64,0x41,0xD7,0x29,0x70,0xDA,0xD4,0x82,0x8B,0x12,0x27,0xF1,
  0xE1,0x98,0x9D,0x06,0xE0,0x4A,0xDD,0xDA,0x93,0x29,0x68,0xC9,0x3C,0x0A,0x04,0x75,
  0x9E,0xB4,0x3E,0xC1,0x79,0x47,0x3F,0x9F,0xDD,0x61,0xF1,0x16,0xE0,0xCC,0x80,0x82,
  0x04,0x37,0x39,0x24,0x61,0x02,0x81,0x81,0x00,0x89,0x13,0x30,0x54,0x00,0x69,0x9F,
  0x37,0xB3,0xDA,0xF5,0x1E,0xF1,0x6D,0xE5,0x46,0x15,0x09,0x48,0x82,0x3A,0xD7,0xBC,
  0x1E,0x44,0x0C,0x85,0xD4,0x7E,0xD3,0x00,0x2E,0x9B,0x82,0x3B,0x47,0x51,0x77,0x75,
  0x3E,0x7F,0x87,0x1B,0xB8,0x40,0x7A,0x4A,0x8E,0x6A,0xCB,0x83,0x3D,0x59,0x72,0xEA,
  0x48,0x4E,0xBD,0xE0,0x4C,0x21,0x9F,0x5F,0xE1,0x41,0x47,0xF3,0xDD,0xF0,0x2E,0x49,
  0xF7,0xC1,0x4C,0xFE,0x86,0x32,0x5C,0x72,0x59,0x92,0x4D,0x65,0x0E,0x29,0xE2,0x98,
  0x7D,0xFB,0xE8,0xE8,0x4F,0x1E,0x0F,0x12,0xB3,0xDC,0xA7,0xD5,0xE9,0x97,0x8A,0x32,
  0x02,0x76,0x6D,0xDA,0x76,0x1D,0x61,0xBA,0xC2,0x78,0xA5,0x87,0xF0,0xFB,0x74,0x2B,
  0x61,0x71,0x0A,0x10,0x8C,0x61,0x0F,0x5A,0x91
};
 
const unsigned char RSA_priv_exp_modul[RSA_priv_exp_modul_SIZE] __attribute__ ((aligned (4)))= 
{
  0x0F,0x0B,0xE1,0x56,0xB9,0x57,0xC1,0xA6,0x03,0xCF,0x06,0x53,0xD8,0xB6,0x9F,0xFD,
  0x19,0x13,0xEA,0xDF,0x58,0xE3,0x45,0xA6,0xA0,0x44,0x1B,0x6F,0xE9,0x1B,0xAB,0x2F,
  0x2F,0x72,0x7E,0x26,0x10,0x1A,0x9B,0xD4,0x3F,0xA2,0x40,0x13,0x92,0x92,0x57,0x55,
  0x3B,0x49,0x6C,0xC9,0x55,0xEE,0xC2,0x97,0x72,0xA1,0x96,0x93,0x02,0x03,0x54,0x32,
  0x4F,0x87,0x43,0x06,0xF2,0x08,0x92,0xD3,0x8F,0xF6,0x41,0x9A,0xEF,0x3F,0x63,0xA4,
  0x75,0xAB,0x65,0x8E,0xA0,0x3A,0x30,0xDE,0xAC,0x43,0x08,0xFD,0x21,0xF9,0x63,0x9B,
  0x49,0xB2,0x17,0xC8,0x27,0x84,0xE7,0xE9,0xF6,0xB4,0x7A,0x46,0x39,0xE9,0xED,0x75,
  0xC9,0x92,0x05,0x32,0x33,0x0D,0x08,0x67,0x3A,0x04,0xAF,0x09,0xFA,0x33,0xDD,0x98,
  0x8D,0x95,0xC0,0xE8,0x20,0x5C,0x71,0x40,0x8F,0xFB,0x77,0x8C,0xF9,0xF0,0x86,0xEF,
  0xC8,0x9F,0x94,0x82,0x93,0x17,0xC0,0x90,0x80,0xE8,0x7C,0x84,0x3E,0x99,0x70,0x4B,
  0xD1,0x96,0x51,0x68,0xDA,0xD4,0xAB,0xF3,0x91,0x1E,0xEB,0xEA,0x84,0xF4,0xA3,0x29,
  0xEA,0xBE,0x03,0x5A,0xAA,0x5A,0xA2,0x8C,0x17,0xD2,0xE0,0x9A,0xB7,0x9A,0x68,0xA6,
  0x49,0xBE,0xA1,0xFC,0xCC,0x92,0x03,0x3E,0x2B,0xDB,0x21,0x83,0x82,0xE0,0x3B,0x61,
  0x3A,0x62,0xDA,0x38,0x21,0x21,0xE1,0xDC,0xEE,0xC9,0x80,0xA0,0xF2,0x8A,0x8B,0x90,
  0xF3,0x56,0x2D,0x75,0xE9,0x81,0x68,0x0B,0x89,0xE8,0x19,0x9F,0x5B,0x1F,0xBE,0x5E,
  0x59,0xD1,0xE4,0x65,0x43,0x7F,0xA7,0xAD,0x5F,0x9B,0xEF,0x2B,0x64,0xA2,0x9C,0x43,
  0x9F,0x51,0x4F,0x4A,0xC9,0x7C,0x79,0x42,0x57,0x1B,0x48,0x65,0x60,0x3E,0x90,0xDF,
  0xB8,0x92,0x3F,0x8A,0x5C,0x77,0x4E,0x3A,0x9A,0xAD,0x3F,0x0D,0xFB,0x4F,0x12,0x8B,
  0x49,0x56,0xDC,0x59,0x83,0xA8,0x3B,0x03,0xED,0x7D,0xBE,0x84,0xEA,0xEC,0x5E,0xDC,
  0xEE,0x6D,0xA6,0x3C,0xF4,0x1C,0xE4,0x27,0x03,0x15,0x34,0xA7,0xDD,0x5A,0x9B,0xCA,
  0x23,0x2E,0x13,0xE7,0x25,0x19,0xDD,0x78,0x3F,0x1B,0xC1,0x6D,0xFD,0xE7,0x6D,0x69,
  0xA0,0x69,0x05,0xFA,0x3D,0xF3,0x43,0xD8,0x96,0xC9,0xAE,0xFB,0x63,0x04,0x82,0xF9,
  0x94,0xBA,0x19,0x9C,0xBF,0x60,0x7D,0xB0,0x52,0xFC,0xB6,0x03,0xC5,0x85,0xD6,0x41,
  0x6E,0xF0,0xF4,0xB2,0x70,0x58,0x28,0x60,0xE2,0x27,0xA0,0x27,0x6D,0x2A,0x08,0xD5,
  0x4F,0x29,0x45,0x50,0xFE,0x6D,0x98,0xB5,0xFE,0xFD,0xE1,0xC0,0x7E,0xD2,0xC5,0xD4,
  0xCE,0x4A,0x07,0x98,0x1F,0x7D,0x2A,0x14,0xF7,0x8A,0x43,0xB8,0x21,0x2E,0x19,0x2E,
  0x28,0xAB,0x24,0x70,0x05,0x0F,0xD9,0x97,0x89,0xB3,0x05,0x62,0x6E,0xE5,0xBD,0xA4,
  0x73,0xD9,0xD2,0x78,0x30,0x95,0xA4,0xFF,0x05,0xF0,0x21,0x46,0x63,0x97,0x6B,0x33,
  0x66,0x1C,0xE0,0xC1,0x3F,0xC1,0x7A,0xD2,0x2D,0x8C,0x26,0x23,0x1B,0xC6,0xE7,0x35,
  0x0F,0xB0,0x6F,0xFB,0x7F,0x30,0x27,0xFF,0x0C,0x07,0x7D,0xD8,0xDD,0x47,0x8A,0x10,
  0x9D,0xFE,0x5E,0x1E,0x3F,0x89,0x1A,0x76,0xB1,0x13,0xEF,0x1F,0x54,0x71,0x7C,0x89,
  0x17,0xD1,0x1B,0x08,0x82,0x5D,0x17,0x28,0xB2,0xC3,0x2D,0x18,0x5F,0x97,0x1D,0xC5
};
 
 
const unsigned char Flash_access_pass[Flash_access_pass_SIZE] __attribute__ ((aligned (4)))=
{
  // SWD/JTAG access code strings:
  // For protection type 0 : JTAG/SWD no need access code
  // For protection type 1 : 138E77916B3A807383FB2A403418A0E3
  // For protection type 2 : 138E77916B3A807383FB2A403418A0A3
  // For protection type 3 : JTAG/SWD disabled
  0x13,0x8E,0x77,0x91,0x6B,0x3A,0x80,0x73,0x83,0xFB,0x2A,0x40,0x34,0x18,0xA0,0x23
};
const unsigned char Monitor_pass[Monitor_pass_SIZE] = "#1mkW*#Vk1tDW*PG";
const unsigned char Engnr_menu_pass[Engnr_menu_pass_SIZE] = "fLkQCKIduKr8*I2h";
const unsigned char WIFI_pass[WIFI_pass_SIZE] = "pUM0HeiE09jMHyXN";