// Copyright (c) 2013, Zoltán Vetési
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Zoltán Vetési nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL ZOLTÁN VETÉSI BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/*** !!! AUTO GENERATED DO NOT EDIT !!! ***/

#ifndef STR_DECODE_TABLE_H_
#define STR_DECODE_TABLE_H_

/** STATES **/

#define STR_ASCII              1
#define STR_UTF8_START_OCT_2   2
#define STR_UTF8_START_OCT_3   3
#define STR_UTF8_START_OCT_4   4
#define STR_QUOTE              5
#define STR_ESC_START          6
#define STR_NULL               7
#define STR_UTF8_INVALID       8
#define STR_UTF8_PART          9

/** TABLE **/

static unsigned char str_state_table[] = {
	 7,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  6,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, 
	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 
	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 
	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 
	 9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, 
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, 
	 4,  4,  4,  4,  4,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8
};

/** UTF-8 utilities **/

#define UTF8_OCT2_MASK 0x1F
#define UTF8_OCT3_MASK 0x0F
#define UTF8_OCT4_MASK 0x07
#define UTF8_OCT_PART_MASK 0x3F


/** Switch-Case **/

#define STR_ASCII_CASE \
	case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:  \
	case 0x07: case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C:  \
	case 0x0D: case 0x0E: case 0x0F: case 0x10: case 0x11: case 0x12:  \
	case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:  \
	case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E:  \
	case 0x1F: case 0x20: case 0x21: case 0x23: case 0x24: case 0x25:  \
	case 0x26: case 0x27: case 0x28: case 0x29: case 0x2A: case 0x2B:  \
	case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x30: case 0x31:  \
	case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:  \
	case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D:  \
	case 0x3E: case 0x3F: case 0x40: case 0x41: case 0x42: case 0x43:  \
	case 0x44: case 0x45: case 0x46: case 0x47: case 0x48: case 0x49:  \
	case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:  \
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55:  \
	case 0x56: case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5B:  \
	case 0x5D: case 0x5E: case 0x5F: case 0x60: case 0x61: case 0x62:  \
	case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: case 0x68:  \
	case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E:  \
	case 0x6F: case 0x70: case 0x71: case 0x72: case 0x73: case 0x74:  \
	case 0x75: case 0x76: case 0x77: case 0x78: case 0x79: case 0x7A:  \
	case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F

#define STR_UTF8_START_OCT_2_CASE \
	case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5:  \
	case 0xC6: case 0xC7: case 0xC8: case 0xC9: case 0xCA: case 0xCB:  \
	case 0xCC: case 0xCD: case 0xCE: case 0xCF: case 0xD0: case 0xD1:  \
	case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:  \
	case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD:  \
	case 0xDE: case 0xDF

#define STR_UTF8_START_OCT_3_CASE \
	case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5:  \
	case 0xE6: case 0xE7: case 0xE8: case 0xE9: case 0xEA: case 0xEB:  \
	case 0xEC: case 0xED: case 0xEE: case 0xEF

#define STR_UTF8_START_OCT_4_CASE \
	case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4

#define STR_UTF8_PART_CASE \
	case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85:  \
	case 0x86: case 0x87: case 0x88: case 0x89: case 0x8A: case 0x8B:  \
	case 0x8C: case 0x8D: case 0x8E: case 0x8F: case 0x90: case 0x91:  \
	case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:  \
	case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D:  \
	case 0x9E: case 0x9F: case 0xA0: case 0xA1: case 0xA2: case 0xA3:  \
	case 0xA4: case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xA9:  \
	case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF:  \
	case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5:  \
	case 0xB6: case 0xB7: case 0xB8: case 0xB9: case 0xBA: case 0xBB:  \
	case 0xBC: case 0xBD: case 0xBE: case 0xBF

#define STR_QUOTE_CASE \
	case 0x22

#define STR_ESC_START_CASE \
	case 0x5C

#define STR_NULL_CASE \
	case 0x00

#define STR_UTF8_INVALID_CASE \
	case 0xF5: case 0xF6: case 0xF7: case 0xF8: case 0xF9: case 0xFA:  \
	case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF


#endif /* STR_DECODE_TABLE_H_ */
