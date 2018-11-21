/*
Copyright (c) 2018 Steven Arnow <s@rdw.se>
'rpcx.c' - This file is part of rPCX 

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

	1. The origin of this software must not be misrepresented; you must not
	claim that you wrote the original software. If you use this software
	in a product, an acknowledgment in the product documentation would be
	appreciated but is not required.

	2. Altered source versions must be plainly marked as such, and must not be
	misrepresented as being the original software.

	3. This notice may not be removed or altered from any source
	distribution.
*/

#define LE_TO_WORD(arr, idx) (((arr)[(idx)]) | (((arr)[(idx)+1]) << 8))

#include <stdio.h>
#include <string.h>
#include "rpcx.h"

struct RPCXState {
	int			w;
	int			h;
	int			bpp;
	int			planes;
	int			valid;
} state;


static FILE *fp;


static void _process_bits(struct RPCXInfo *ri, unsigned char data, int row, int col, int plane) {
	int pixels = 8 / state.bpp;
	int i, shift;
	unsigned char mask;

	shift = state.bpp;
	mask = (0xFF << shift) ^ 0xFF;

	for (i = 0; i < pixels; i++)
		if (col * pixels + i < ri->w)
			ri->data[state.w * row + col * pixels + i] |= ((data >> (shift * (pixels - i - 1))) & mask) << (shift * plane);
}


void rpcx_close() {
	fclose(fp);

	return;
}


int rpcx_read(struct RPCXInfo *ri) {
	int row, plane, px, pxpl, j;
	unsigned char d;
	
	if (!state.valid)
		return 0;

	if (state.bpp == 8)
		pxpl = state.w;
	else if (state.bpp == 1) {
		pxpl = state.w >> 3;
		if (state.w & 7)
			pxpl++;
	} else if (state.bpp == 2) {
		pxpl = state.w >> 2;
		if (state.w & 3)
			pxpl++;
	} else {
		pxpl = state.w >> 1;
		if (state.w & 1)
			pxpl++;
	}


	for (row = 0; row < ri->h; row++) 
		for (plane = 0; plane < state.planes; plane++) 
			for (px = 0; px < pxpl;) {
				fread(&d, 1, 1, fp);
				if ((d & 0xC0) == 0xC0) {
					j = (d & 0x3F);
					fread(&d, 1, 1, fp);
				} else
					j = 1;

				for (; j > 0; j--)
					_process_bits(ri, d, row, px++, plane);
			}

	fread(ri->palette, 3, 256, fp);

	return 1;
}


int rpcx_init(const char *fname, struct RPCXInfo *ri) {
	unsigned char data[128];
	signed short xmin, ymin, xmax, ymax;

	state.valid = 0;
	ri->w = ri->h = 0;
	
	if (!(fp = fopen(fname, "rb")))
		return 0;
	
	fread(data, 1, 128, fp);
	
	if (data[0] != 0xA)
		return 0;
	if (data[2] != 1)
		return 0;
	state.bpp = data[3];
	state.planes = data[65];
	xmin = LE_TO_WORD(data, 4);
	ymin = LE_TO_WORD(data, 6);
	xmax = LE_TO_WORD(data, 8);
	ymax = LE_TO_WORD(data, 10);

	xmax++;
	ymax++;

	state.w = xmax - xmin;
	state.h = ymax - ymin;

	if (state.planes != 1 && state.bpp != 1)
		return 0;
	
	state.valid = 1;
	ri->w = state.w;
	ri->h = state.h;
	memcpy(ri->palette, data + 16, 48);
	
	printf("%i x %i, %i planes, %i bits per pixel\n", state.w, state.h, state.planes, state.bpp);
	return 1;
}


