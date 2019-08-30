// modified from wpa_supplicant 2.9
/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

const unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


unsigned char * base64_gen_decode(const unsigned char *src, unsigned len,
					 unsigned *out_len,
					 const unsigned char *table)
{
	unsigned char dtable[256], *out, *pos, block[4], tmp;
	unsigned i, count, olen;
	int pad = 0;
	unsigned extra_pad;

	memset(dtable, 0x80, 256);
	for (i = 0; i < sizeof(base64_table) - 1; i++)
		dtable[table[i]] = (unsigned char) i;
	dtable['='] = 0;

	count = 0;
	for (i = 0; i < len; i++) {
		if (dtable[src[i]] != 0x80)
			count++;
	}

	if (count == 0)
		return 0;
	extra_pad = (4 - count % 4) % 4;

	olen = (count + extra_pad) / 4 * 3;
	pos = out = malloc(olen);
	if (out == 0)
		return 0;

	count = 0;
	for (i = 0; i < len + extra_pad; i++) {
		unsigned char val;

		if (i >= len)
			val = '=';
		else
			val = src[i];
		tmp = dtable[val];
		if (tmp == 0x80)
			continue;

		if (val == '=')
			pad++;
		block[count] = tmp;
		count++;
		if (count == 4) {
			*pos++ = (block[0] << 2) | (block[1] >> 4);
			*pos++ = (block[1] << 4) | (block[2] >> 2);
			*pos++ = (block[2] << 6) | block[3];
			count = 0;
			if (pad) {
				if (pad == 1)
					pos--;
				else if (pad == 2)
					pos -= 2;
				else {
					/* Invalid padding */
					free(out);
					return 0;
				}
				break;
			}
		}
	}

	*out_len = pos - out;
	return out;
}
