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
#define STR_UTF8_INVALID       7
#define STR_UTF8_PART          8

/** TABLE **/

static unsigned char str_state_table[] = {
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  5,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  6,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	 2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
	 3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
	 4,  4,  4,  4,  4,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7
};

/** UTF-8 utilities **/

#define UTF8_OCT2_MASK 0x1F
#define UTF8_OCT3_MASK 0x0F
#define UTF8_OCT4_MASK 0x07
#define UTF8_OCT_PART_MASK 0x3F

#endif /* STR_DECODE_TABLE_H_ */
