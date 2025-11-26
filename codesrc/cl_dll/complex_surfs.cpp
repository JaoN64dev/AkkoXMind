#include "hud.h"
#include "cl_util.h"

#define TILE_SIZE 64
#define CYCLE     128
#define SPEED     64

#include <cmath>

extern unsigned short d_8to16table[256];

typedef unsigned char pixel_t;

/*
================
R_GenTurbTile
================
*/
#define TILE_SIZE 64
#define SPEED 2.0f

void R_GenTurbTile(pixel_t* pbasetex, void* pdest)
{
	byte* pd = (byte*)pdest;

	float time = gEngfuncs.GetClientTime();

	for (int i = 0; i < TILE_SIZE; i++)
	{
		for (int j = 0; j < TILE_SIZE; j++)
		{
			float s = j + sinf((i + time * 15.0f) * 0.15f) * 5.0f;
			float t = i + cosf((j + time * 15.0f) * 0.15f) * 5.0f;

			int is = ((int)s) & 63;
			int it = ((int)t) & 63;

			*pd++ = pbasetex[(it << 6) + is];
		}
	}
}

void R_GenTurbTile16(pixel_t* pbasetex, void* pdest)
{
	unsigned short* pd = (unsigned short*)pdest;

	float time = gEngfuncs.GetClientTime();

	for (int i = 0; i < TILE_SIZE; i++)
	{
		for (int j = 0; j < TILE_SIZE; j++)
		{
			float s = j + sinf((i + time * 15.0f) * 0.15f) * 5.0f;
			float t = i + cosf((j + time * 15.0f) * 0.15f) * 5.0f;

			int is = ((int)s) & 63;
			int it = ((int)t) & 63;

			byte texel = pbasetex[(it << 6) + is];

			// converter 8bit -> 16bit RGB565 sem d_8to16table
			int r = texel;
			int g = texel;
			int b = texel;

			unsigned short rgb565 =
				((r >> 3) << 11) |
				((g >> 2) << 5) |
				((b >> 3));

			*pd++ = rgb565;
		}
	}
}

//this is straight ported from quake. now to render it in a func.

