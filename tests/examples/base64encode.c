// modified from wpa_supplicant 2.9
/*
 * Base64 encoding/decoding (RFC1341)
 * Copyright (c) 2005-2019, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include <stdlib.h>
#include <stdint.h>

const unsigned char base64_table[65] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


unsigned char * base64_gen_encode(const unsigned char *src, unsigned len,
					 unsigned *out_len,
					 const unsigned char *table)
{
    int add_pad = 1;
	unsigned char *out, *pos;
	const unsigned char *end, *in;
	unsigned olen;
	int line_len;

	if (len >= SIZE_MAX / 4)
		return 0;
	olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
	if (add_pad)
		olen += olen / 72; /* line feeds */
	olen++; /* nul termination */
	if (olen < len)
		return 0; /* integer overflow */
	out = malloc(olen);
	if (out == 0)
		return 0;

	end = src + len;
	in = src;
	pos = out;
	line_len = 0;
	while (end - in >= 3) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		*pos++ = table[(((in[0] & 0x03) << 4) | (in[1] >> 4)) & 0x3f];
		*pos++ = table[(((in[1] & 0x0f) << 2) | (in[2] >> 6)) & 0x3f];
		*pos++ = table[in[2] & 0x3f];
		in += 3;
		line_len += 4;
		if (add_pad && line_len >= 72) {
			*pos++ = '\n';
			line_len = 0;
		}
	}

	if (end - in) {
		*pos++ = table[(in[0] >> 2) & 0x3f];
		if (end - in == 1) {
			*pos++ = table[((in[0] & 0x03) << 4) & 0x3f];
			if (add_pad)
				*pos++ = '=';
		} else {
			*pos++ = table[(((in[0] & 0x03) << 4) |
					(in[1] >> 4)) & 0x3f];
			*pos++ = table[((in[1] & 0x0f) << 2) & 0x3f];
		}
		if (add_pad)
			*pos++ = '=';
		line_len += 4;
	}

	if (add_pad && line_len)
		*pos++ = '\n';

	*pos = '\0';
	if (out_len)
		*out_len = pos - out;
	return out;
}
