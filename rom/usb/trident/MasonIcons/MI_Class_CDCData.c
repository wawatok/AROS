#ifdef USE_CLASS_CDCDATA_COLORS
const ULONG Class_CDCData_colors[96] =
{
	0x96969696,0x96969696,0x96969696,
	0x2d2d2d2d,0x28282828,0x9e9e9e9e,
	0x00000000,0x65656565,0x9a9a9a9a,
	0x35353535,0x75757575,0xaaaaaaaa,
	0x65656565,0x8a8a8a8a,0xbabababa,
	0x0c0c0c0c,0x61616161,0xffffffff,
	0x24242424,0x5d5d5d5d,0x24242424,
	0x35353535,0x8a8a8a8a,0x35353535,
	0x86868686,0xb2b2b2b2,0x3d3d3d3d,
	0x0c0c0c0c,0xe3e3e3e3,0x00000000,
	0x4d4d4d4d,0x9e9e9e9e,0x8e8e8e8e,
	0x82828282,0x00000000,0x00000000,
	0xdfdfdfdf,0x35353535,0x35353535,
	0xdbdbdbdb,0x65656565,0x39393939,
	0xdbdbdbdb,0x8e8e8e8e,0x41414141,
	0xdfdfdfdf,0xbabababa,0x45454545,
	0xefefefef,0xe7e7e7e7,0x14141414,
	0x82828282,0x61616161,0x4d4d4d4d,
	0xa6a6a6a6,0x7e7e7e7e,0x61616161,
	0xcacacaca,0x9a9a9a9a,0x75757575,
	0x9a9a9a9a,0x55555555,0xaaaaaaaa,
	0xffffffff,0x00000000,0xffffffff,
	0xffffffff,0xffffffff,0xffffffff,
	0xdfdfdfdf,0xdfdfdfdf,0xdfdfdfdf,
	0xcacacaca,0xcacacaca,0xcacacaca,
	0xbabababa,0xbabababa,0xbabababa,
	0xaaaaaaaa,0xaaaaaaaa,0xaaaaaaaa,
	0x8a8a8a8a,0x8a8a8a8a,0x8a8a8a8a,
	0x65656565,0x65656565,0x65656565,
	0x4d4d4d4d,0x4d4d4d4d,0x4d4d4d4d,
	0x3c3c3c3c,0x3c3c3c3c,0x3b3b3b3b,
	0x00000000,0x00000000,0x00000000,
};
#endif

#define CLASS_CDCDATA_WIDTH        16
#define CLASS_CDCDATA_HEIGHT       16
#define CLASS_CDCDATA_DEPTH         5
#define CLASS_CDCDATA_COMPRESSION   1
#define CLASS_CDCDATA_MASKING       2

#ifdef USE_CLASS_CDCDATA_HEADER
const struct BitMapHeader Class_CDCData_header =
{ 16,16,88,102,5,2,1,0,0,1,1,800,600 };
#endif

#ifdef USE_CLASS_CDCDATA_BODY
const UBYTE Class_CDCData_body[218] = {
0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,
0x00,0xff,0x00,0xff,0x00,0x01,0x01,0x80,0x01,0x21,0x80,0x01,0x1e,0x00,0x01,
0x20,0x80,0x01,0x20,0x80,0x01,0x8a,0xc0,0x01,0x9f,0xc0,0x01,0xa0,0x00,0xff,
0x40,0x01,0xc0,0x40,0x01,0x55,0x6a,0x01,0x00,0xca,0x01,0x2a,0x70,0x01,0xc0,
0x7a,0x01,0xc0,0x7a,0x01,0x40,0xd4,0x01,0x15,0x54,0x01,0x2a,0x60,0x01,0xc0,
0x74,0x01,0xc0,0x74,0x01,0x95,0x60,0x01,0x5e,0xe0,0x01,0x20,0x80,0x01,0xc0,
0xe0,0x01,0xc0,0xe0,0x01,0x40,0x80,0x01,0x70,0x80,0x01,0x0f,0x00,0x01,0x50,
0x00,0x01,0x50,0x00,0x01,0x45,0x40,0x01,0x4f,0xc0,0x01,0x50,0x00,0x01,0x20,
0x00,0x01,0x60,0x00,0x01,0x2a,0xb5,0x01,0x00,0x65,0x01,0x15,0x38,0x01,0x60,
0x3d,0x01,0x60,0x3d,0x01,0x20,0x6a,0x01,0x0a,0xaa,0x01,0x15,0x30,0x01,0x60,
0x3a,0x01,0x60,0x3a,0x01,0x4a,0xb0,0x01,0x2f,0x70,0x01,0x10,0x40,0x01,0x60,
0x70,0x01,0x60,0x70,0x01,0x35,0x40,0x01,0x3a,0xc0,0x01,0x02,0x80,0x01,0x22,
0xc0,0x01,0x22,0xc0,0x01,0x0f,0x80,0x01,0x0f,0x80,0xff,0x00,0x01,0x0f,0x80,
0x01,0x0f,0x80,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,
0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00, };
#endif
