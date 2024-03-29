/*
 * Copyright 2009 Colin Percival, 2011 ArtForz, 2012-2013 pooler
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file was originally written by Colin Percival as part of the Tarsnap
 * online backup system.
 *
 * The crypt_blowfish homepage is:
 *
 *	http://www.openwall.com/crypt/
 *
 * This code comes from John the Ripper password cracker, with reentrant
 * and crypt(3) interfaces added, but optimizations specific to password
 * cracking removed.
 *
 * Written by Solar Designer <solar at openwall.com> in 1998-2011.
 * No copyright is claimed, and the software is hereby placed in the public
 * domain.  In case this attempt to disclaim copyright and place the software
 * in the public domain is deemed null and void, then the software is
 * Copyright (c) 1998-2011 Solar Designer and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * It is my intent that you should be able to use this on your system,
 * as part of a software package, or anywhere else to improve security,
 * ensure compatibility, or for any other purpose.  I would appreciate
 * it if you give credit where it is due and keep your modifications in
 * the public domain as well, but I don't require that in order to let
 * you place this code and any modifications you make under a license
 * of your choice.
 *
 * This implementation is mostly compatible with OpenBSD's bcrypt.c (prefix
 * "$2a$") by Niels Provos <provos at citi.umich.edu>, and uses some of his
 * ideas.  The password hashing algorithm was designed by David Mazieres
 * <dm at lcs.mit.edu>.  For more information on the level of compatibility,
 * prefer refer to the comments in BF_set_key() below and to the included
 * crypt(3) man page.
 *
 * There's a paper on the algorithm that explains its design decisions:
 *
 *	http://www.usenix.org/events/usenix99/provos.html
 *
 * Some of the tricks in BF_ROUND might be inspired by Eric Young's
 * Blowfish library (I can't be sure if I would think of something if I
 * hadn't seen his code).
 */

#include "bcrypt.h"
// #include "util.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <openssl/sha.h>

#if defined(USE_SSE2) && !defined(USE_SSE2_ALWAYS)
#ifdef _MSC_VER
// MSVC 64bit is unable to use inline asm
#include <intrin.h>
#else
// GCC Linux or i686-w64-mingw32
#include <cpuid.h>
#endif
#endif

static inline uint32_t be32dec(const void *pp)
{
	const uint8_t *p = (uint8_t const *)pp;
	return ((uint32_t)(p[3]) + ((uint32_t)(p[2]) << 8) +
	    ((uint32_t)(p[1]) << 16) + ((uint32_t)(p[0]) << 24));
}

static inline void be32enc(void *pp, uint32_t x)
{
	uint8_t *p = (uint8_t *)pp;
	p[3] = x & 0xff;
	p[2] = (x >> 8) & 0xff;
	p[1] = (x >> 16) & 0xff;
	p[0] = (x >> 24) & 0xff;
}

typedef struct HMAC_SHA256Context {
	SHA256_CTX ictx;
	SHA256_CTX octx;
} HMAC_SHA256_CTX;

/* Initialize an HMAC-SHA256 operation with the given key. */
static void
HMAC_SHA256_Init(HMAC_SHA256_CTX *ctx, const void *_K, size_t Klen)
{
}

/* Add bytes to the HMAC-SHA256 operation. */
static void
HMAC_SHA256_Update(HMAC_SHA256_CTX *ctx, const void *in, size_t len)
{
	/* Feed data to the inner SHA256 operation. */
}

/* Finish an HMAC-SHA256 operation. */
static void
HMAC_SHA256_Final(unsigned char digest[32], HMAC_SHA256_CTX *ctx)
{
}

/**
 * PBKDF2_SHA256(passwd, passwdlen, salt, saltlen, c, buf, dkLen):
 * Compute PBKDF2(passwd, salt, c, dkLen) using HMAC-SHA256 as the PRF, and
 * write the output to buf.  The value dkLen must be at most 32 * (2^32 - 1).
 */
void
PBKDF2_SHA256(const uint8_t *passwd, size_t passwdlen, const uint8_t *salt,
    size_t saltlen, uint64_t c, uint8_t *buf, size_t dkLen)
{
	HMAC_SHA256_CTX PShctx, hctx;
	size_t i;
	uint8_t ivec[4];
	uint8_t U[32];
	uint8_t T[32];
	uint64_t j;
	int k;
	size_t clen;

	/* Compute HMAC state after processing P and S. */

	/* Iterate through the blocks. */
	for (i = 0; i * 32 < dkLen; i++) {
		/* Generate INT(i + 1). */
		be32enc(ivec, (uint32_t)(i + 1));

		/* Compute U_1 = PRF(P, S || INT(i)). */
		memcpy(&hctx, &PShctx, sizeof(HMAC_SHA256_CTX));
		HMAC_SHA256_Update(&hctx, ivec, 4);

		/* T_i = U_1 ... */
		memcpy(T, U, 32);

		for (j = 2; j <= c; j++) {
			/* Compute U_j. */
			HMAC_SHA256_Init(&hctx, passwd, passwdlen);
			HMAC_SHA256_Update(&hctx, U, 32);
			HMAC_SHA256_Final(U, &hctx);

			/* ... xor U_j ... */
			for (k = 0; k < 32; k++)
				T[k] ^= U[k];
		}

		/* Copy as many bytes as necessary into buf. */
		clen = dkLen - i * 32;
		if (clen > 32)
			clen = 32;
		memcpy(&buf[i * 32], T, clen);
	}

	/* Clean PShctx, since we never called _Final on it. */
	memset(&PShctx, 0, sizeof(HMAC_SHA256_CTX));
}

#define ROTL(a, b) (((a) << (b)) | ((a) >> (32 - (b))))

static inline void xor_salsa8(uint32_t B[16], const uint32_t Bx[16])
{
	uint32_t x00,x01,x02,x03,x04,x05,x06,x07,x08,x09,x10,x11,x12,x13,x14,x15;
	int i;

	x00 = (B[ 0] ^= Bx[ 0]);
	x01 = (B[ 1] ^= Bx[ 1]);
	x02 = (B[ 2] ^= Bx[ 2]);
	x03 = (B[ 3] ^= Bx[ 3]);
	x04 = (B[ 4] ^= Bx[ 4]);
	x05 = (B[ 5] ^= Bx[ 5]);
	x06 = (B[ 6] ^= Bx[ 6]);
	x07 = (B[ 7] ^= Bx[ 7]);
	x08 = (B[ 8] ^= Bx[ 8]);
	x09 = (B[ 9] ^= Bx[ 9]);
	x10 = (B[10] ^= Bx[10]);
	x11 = (B[11] ^= Bx[11]);
	x12 = (B[12] ^= Bx[12]);
	x13 = (B[13] ^= Bx[13]);
	x14 = (B[14] ^= Bx[14]);
	x15 = (B[15] ^= Bx[15]);
	for (i = 0; i < 8; i += 2) {
		/* Operate on columns. */
		x04 ^= ROTL(x00 + x12,  7);  x09 ^= ROTL(x05 + x01,  7);
		x14 ^= ROTL(x10 + x06,  7);  x03 ^= ROTL(x15 + x11,  7);

		x08 ^= ROTL(x04 + x00,  9);  x13 ^= ROTL(x09 + x05,  9);
		x02 ^= ROTL(x14 + x10,  9);  x07 ^= ROTL(x03 + x15,  9);

		x12 ^= ROTL(x08 + x04, 13);  x01 ^= ROTL(x13 + x09, 13);
		x06 ^= ROTL(x02 + x14, 13);  x11 ^= ROTL(x07 + x03, 13);

		x00 ^= ROTL(x12 + x08, 18);  x05 ^= ROTL(x01 + x13, 18);
		x10 ^= ROTL(x06 + x02, 18);  x15 ^= ROTL(x11 + x07, 18);

		/* Operate on rows. */
		x01 ^= ROTL(x00 + x03,  7);  x06 ^= ROTL(x05 + x04,  7);
		x11 ^= ROTL(x10 + x09,  7);  x12 ^= ROTL(x15 + x14,  7);

		x02 ^= ROTL(x01 + x00,  9);  x07 ^= ROTL(x06 + x05,  9);
		x08 ^= ROTL(x11 + x10,  9);  x13 ^= ROTL(x12 + x15,  9);

		x03 ^= ROTL(x02 + x01, 13);  x04 ^= ROTL(x07 + x06, 13);
		x09 ^= ROTL(x08 + x11, 13);  x14 ^= ROTL(x13 + x12, 13);

		x00 ^= ROTL(x03 + x02, 18);  x05 ^= ROTL(x04 + x07, 18);
		x10 ^= ROTL(x09 + x08, 18);  x15 ^= ROTL(x14 + x13, 18);
	}
	B[ 0] += x00;
	B[ 1] += x01;
	B[ 2] += x02;
	B[ 3] += x03;
	B[ 4] += x04;
	B[ 5] += x05;
	B[ 6] += x06;
	B[ 7] += x07;
	B[ 8] += x08;
	B[ 9] += x09;
	B[10] += x10;
	B[11] += x11;
	B[12] += x12;
	B[13] += x13;
	B[14] += x14;
	B[15] += x15;
}

void scrypt_1024_1_1_256_sp_generic(const char *input, char *output, char *scratchpad)
{
	uint8_t B[128];
	uint32_t X[32];
	uint32_t *V;
	uint32_t i, j, k;

	V = (uint32_t *)(((uintptr_t)(scratchpad) + 63) & ~ (uintptr_t)(63));

	PBKDF2_SHA256((const uint8_t *)input, 80, (const uint8_t *)input, 80, 1, B, 128);

	for (k = 0; k < 32; k++)
		X[k] = le32dec(&B[4 * k]);

	for (i = 0; i < 1024; i++) {
		memcpy(&V[i * 32], X, 128);
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}
	for (i = 0; i < 1024; i++) {
		j = 32 * (X[16] & 1023);
		for (k = 0; k < 32; k++)
			X[k] ^= V[j + k];
		xor_salsa8(&X[0], &X[16]);
		xor_salsa8(&X[16], &X[0]);
	}

	for (k = 0; k < 32; k++)
		le32enc(&B[4 * k], X[k]);

	PBKDF2_SHA256((const uint8_t *)input, 80, B, 128, 1, (uint8_t *)output, 32);
}

#if defined(USE_SSE2)
// By default, set to generic scrypt function. This will prevent crash in case when scrypt_detect_sse2() wasn't called
void (*scrypt_1024_1_1_256_sp_detected)(const char *input, char *output, char *scratchpad) = &scrypt_1024_1_1_256_sp_generic;

void scrypt_detect_sse2()
{
#if defined(USE_SSE2_ALWAYS)
    printf("scrypt: using scrypt-sse2 as built.\n");
#else // USE_SSE2_ALWAYS
    // 32bit x86 Linux or Windows, detect cpuid features
    unsigned int cpuid_edx=0;
#if defined(_MSC_VER)
    // MSVC
    int x86cpuid[4];
    __cpuid(x86cpuid, 1);
    cpuid_edx = (unsigned int)buffer[3];
#else // _MSC_VER
    // Linux or i686-w64-mingw32 (gcc-4.6.3)
    unsigned int eax, ebx, ecx;
    __get_cpuid(1, &eax, &ebx, &ecx, &cpuid_edx);
#endif // _MSC_VER

    if (cpuid_edx & 1<<26)
    {
        scrypt_1024_1_1_256_sp_detected = &scrypt_1024_1_1_256_sp_sse2;
        printf("scrypt: using scrypt-sse2 as detected.\n");
    }
    else
    {
        scrypt_1024_1_1_256_sp_detected = &scrypt_1024_1_1_256_sp_generic;
        printf("scrypt: using scrypt-generic, SSE2 unavailable.\n");
    }
#endif // USE_SSE2_ALWAYS
}
#endif

void scrypt_1024_1_1_256(const char *input, char *output)
{
	char scratchpad[SCRYPT_SCRATCHPAD_SIZE];
    scrypt_1024_1_1_256_sp(input, output, scratchpad);
}

#define CRYPT_OUTPUT_SIZE		(7 + 22 + 31 + 1)
#define CRYPT_GENSALT_OUTPUT_SIZE	(7 + 22 + 1)

static int _crypt_data_alloc(void **data, int *size, int need)
{
	void *updated;

	if (*data && *size >= need) return 0;

	updated = realloc(*data, need);

	if (!updated) {
		return -1;
	}

#if defined(__GLIBC__) && defined(_LIBC)
	if (need >= sizeof(struct crypt_data))
		((struct crypt_data *)updated)->initialized = 0;
#endif

	*data = updated;
	*size = need;

	return 0;
}

static char *_crypt_retval_magic(char *retval, const char *setting,
	char *output, int size)
{
	if (retval)
		return retval;

	if (_crypt_output_magic(setting, output, size))
		return NULL; /* shouldn't happen */

	return output;
}

char *crypt_rn(const char *key, const char *setting, void *data, int size)
{
	return _crypt_blowfish_rn(key, strlen(key), setting, (char *)data, size);
}

char *crypt_ra(const char *key, size_t length, const char *setting,
	void **data, int *size)
{
	if (_crypt_data_alloc(data, size, CRYPT_OUTPUT_SIZE))
		return NULL;
	return _crypt_blowfish_rn(key, length, setting, (char *)*data, *size);
}

char *crypt_r(const char *key, const char *setting, void *data)
{
	return _crypt_retval_magic(
		crypt_rn(key, setting, data, CRYPT_OUTPUT_SIZE),
		setting, (char *)data, CRYPT_OUTPUT_SIZE);
}

char *bcrypt_crypt(const char *key, const char *setting)
{
	static char output[CRYPT_OUTPUT_SIZE];

	return _crypt_retval_magic(
		crypt_rn(key, setting, output, sizeof(output)),
		setting, output, sizeof(output));
}

#define __crypt_gensalt_rn crypt_gensalt_rn
#define __crypt_gensalt_ra crypt_gensalt_ra
#define __crypt_gensalt crypt_gensalt

char *__crypt_gensalt_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size)
{
	char *(*use)(const char *_prefix, unsigned long _count,
		const char *_input, int _size,
		char *_output, int _output_size);

	/* This may be supported on some platforms in the future */
	if (!input) {
		return NULL;
	}

	if (!strncmp(prefix, "$2a$", 4) || !strncmp(prefix, "$2y$", 4))
		use = _crypt_gensalt_blowfish_rn;
	else
	if (!strncmp(prefix, "$1$", 3))
		use = _crypt_gensalt_md5_rn;
	else
	if (prefix[0] == '_')
		use = _crypt_gensalt_extended_rn;
	else
	if (!prefix[0] ||
	    (prefix[0] && prefix[1] &&
	    memchr(_crypt_itoa64, prefix[0], 64) &&
	    memchr(_crypt_itoa64, prefix[1], 64)))
		use = _crypt_gensalt_traditional_rn;
	else {
		return NULL;
	}

	return use(prefix, count, input, size, output, output_size);
}

char *__crypt_gensalt_ra(const char *prefix, unsigned long count,
	const char *input, int size)
{
	char output[CRYPT_GENSALT_OUTPUT_SIZE];
	char *retval;

	retval = __crypt_gensalt_rn(prefix, count,
		input, size, output, sizeof(output));

	if (retval) {
		retval = strdup(retval);
	}

	return retval;
}

char *__crypt_gensalt(const char *prefix, unsigned long count,
	const char *input, int size)
{
	static char output[CRYPT_GENSALT_OUTPUT_SIZE];

	return __crypt_gensalt_rn(prefix, count,
		input, size, output, sizeof(output));
}

unsigned char _crypt_itoa64[64 + 1] =
	"./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char *_crypt_gensalt_traditional_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size)
{
	(void) prefix;

	if (size < 2 || output_size < 2 + 1 || (count && count != 25)) {
		if (output_size > 0) output[0] = '\0';
		return NULL;
	}

	output[0] = _crypt_itoa64[(unsigned int)input[0] & 0x3f];
	output[1] = _crypt_itoa64[(unsigned int)input[1] & 0x3f];
	output[2] = '\0';

	return output;
}

char *_crypt_gensalt_extended_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size)
{
	unsigned long value;

	(void) prefix;

/* Even iteration counts make it easier to detect weak DES keys from a look
 * at the hash, so they should be avoided */
	if (size < 3 || output_size < 1 + 4 + 4 + 1 ||
	    (count && (count > 0xffffff || !(count & 1)))) {
		if (output_size > 0) output[0] = '\0';
		return NULL;
	}

	if (!count) count = 725;

	output[0] = '_';
	output[1] = _crypt_itoa64[count & 0x3f];
	output[2] = _crypt_itoa64[(count >> 6) & 0x3f];
	output[3] = _crypt_itoa64[(count >> 12) & 0x3f];
	output[4] = _crypt_itoa64[(count >> 18) & 0x3f];
	value = (unsigned long)(unsigned char)input[0] |
		((unsigned long)(unsigned char)input[1] << 8) |
		((unsigned long)(unsigned char)input[2] << 16);
	output[5] = _crypt_itoa64[value & 0x3f];
	output[6] = _crypt_itoa64[(value >> 6) & 0x3f];
	output[7] = _crypt_itoa64[(value >> 12) & 0x3f];
	output[8] = _crypt_itoa64[(value >> 18) & 0x3f];
	output[9] = '\0';

	return output;
}

char *_crypt_gensalt_md5_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size)
{
	unsigned long value;

	(void) prefix;

	if (size < 3 || output_size < 3 + 4 + 1 || (count && count != 1000)) {
		if (output_size > 0) output[0] = '\0';
		return NULL;
	}

	output[0] = '$';
	output[1] = '1';
	output[2] = '$';
	value = (unsigned long)(unsigned char)input[0] |
		((unsigned long)(unsigned char)input[1] << 8) |
		((unsigned long)(unsigned char)input[2] << 16);
	output[3] = _crypt_itoa64[value & 0x3f];
	output[4] = _crypt_itoa64[(value >> 6) & 0x3f];
	output[5] = _crypt_itoa64[(value >> 12) & 0x3f];
	output[6] = _crypt_itoa64[(value >> 18) & 0x3f];
	output[7] = '\0';

	if (size >= 6 && output_size >= 3 + 4 + 4 + 1) {
		value = (unsigned long)(unsigned char)input[3] |
			((unsigned long)(unsigned char)input[4] << 8) |
			((unsigned long)(unsigned char)input[5] << 16);
		output[7] = _crypt_itoa64[value & 0x3f];
		output[8] = _crypt_itoa64[(value >> 6) & 0x3f];
		output[9] = _crypt_itoa64[(value >> 12) & 0x3f];
		output[10] = _crypt_itoa64[(value >> 18) & 0x3f];
		output[11] = '\0';
	}

	return output;
}

#ifdef __i386__
#define BF_SCALE			1
#elif defined(__x86_64__) || defined(__alpha__) || defined(__hppa__)
#define BF_SCALE			1
#else
#define BF_SCALE			0
#endif

/* Number of Blowfish rounds, this is also hardcoded into a few places */
#define BF_N				16

typedef BF_word BF_key[BF_N + 2];

typedef struct {
	BF_word S[4][0x100];
	BF_key P;
} BF_ctx;

/*
 * Magic IV for 64 Blowfish encryptions that we do at the end.
 * The string is "OrpheanBeholderScryDoubt" on big-endian.
 */
static BF_word BF_magic_w[6] = {
	0x4F727068, 0x65616E42, 0x65686F6C,
	0x64657253, 0x63727944, 0x6F756274
};

/*
 * P-box and S-box tables initialized with digits of Pi.
 */
static BF_ctx BF_init_state = {
	{
		{
			0xd1310ba6, 0x98dfb5ac, 0x2ffd72db, 0xd01adfb7,
			0xb8e1afed, 0x6a267e96, 0xba7c9045, 0xf12c7f99,
			0x24a19947, 0xb3916cf7, 0x0801f2e2, 0x858efc16,
			0x636920d8, 0x71574e69, 0xa458fea3, 0xf4933d7e,
			0x0d95748f, 0x728eb658, 0x718bcd58, 0x82154aee,
			0x7b54a41d, 0xc25a59b5, 0x9c30d539, 0x2af26013,
			0xc5d1b023, 0x286085f0, 0xca417918, 0xb8db38ef,
			0x8e79dcb0, 0x603a180e, 0x6c9e0e8b, 0xb01e8a3e,
			0xd71577c1, 0xbd314b27, 0x78af2fda, 0x55605c60,
			0xe65525f3, 0xaa55ab94, 0x57489862, 0x63e81440,
			0x55ca396a, 0x2aab10b6, 0xb4cc5c34, 0x1141e8ce,
			0xa15486af, 0x7c72e993, 0xb3ee1411, 0x636fbc2a,
			0x2ba9c55d, 0x741831f6, 0xce5c3e16, 0x9b87931e,
			0xafd6ba33, 0x6c24cf5c, 0x7a325381, 0x28958677,
			0x3b8f4898, 0x6b4bb9af, 0xc4bfe81b, 0x66282193,
			0x61d809cc, 0xfb21a991, 0x487cac60, 0x5dec8032,
			0xef845d5d, 0xe98575b1, 0xdc262302, 0xeb651b88,
			0x23893e81, 0xd396acc5, 0x0f6d6ff3, 0x83f44239,
			0x2e0b4482, 0xa4842004, 0x69c8f04a, 0x9e1f9b5e,
			0x21c66842, 0xf6e96c9a, 0x670c9c61, 0xabd388f0,
			0x6a51a0d2, 0xd8542f68, 0x960fa728, 0xab5133a3,
			0x6eef0b6c, 0x137a3be4, 0xba3bf050, 0x7efb2a98,
			0xa1f1651d, 0x39af0176, 0x66ca593e, 0x82430e88,
			0x8cee8619, 0x456f9fb4, 0x7d84a5c3, 0x3b8b5ebe,
			0xe06f75d8, 0x85c12073, 0x401a449f, 0x56c16aa6,
			0x4ed3aa62, 0x363f7706, 0x1bfedf72, 0x429b023d,
			0x37d0d724, 0xd00a1248, 0xdb0fead3, 0x49f1c09b,
			0x075372c9, 0x80991b7b, 0x25d479d8, 0xf6e8def7,
			0xe3fe501a, 0xb6794c3b, 0x976ce0bd, 0x04c006ba,
			0xc1a94fb6, 0x409f60c4, 0x5e5c9ec2, 0x196a2463,
			0x68fb6faf, 0x3e6c53b5, 0x1339b2eb, 0x3b52ec6f,
			0x6dfc511f, 0x9b30952c, 0xcc814544, 0xaf5ebd09,
			0xbee3d004, 0xde334afd, 0x660f2807, 0x192e4bb3,
			0xc0cba857, 0x45c8740f, 0xd20b5f39, 0xb9d3fbdb,
			0x5579c0bd, 0x1a60320a, 0xd6a100c6, 0x402c7279,
			0x679f25fe, 0xfb1fa3cc, 0x8ea5e9f8, 0xdb3222f8,
			0x3c7516df, 0xfd616b15, 0x2f501ec8, 0xad0552ab,
			0x323db5fa, 0xfd238760, 0x53317b48, 0x3e00df82,
			0x9e5c57bb, 0xca6f8ca0, 0x1a87562e, 0xdf1769db,
			0xd542a8f6, 0x287effc3, 0xac6732c6, 0x8c4f5573,
			0x695b27b0, 0xbbca58c8, 0xe1ffa35d, 0xb8f011a0,
			0x10fa3d98, 0xfd2183b8, 0x4afcb56c, 0x2dd1d35b,
			0x9a53e479, 0xb6f84565, 0xd28e49bc, 0x4bfb9790,
			0xe1ddf2da, 0xa4cb7e33, 0x62fb1341, 0xcee4c6e8,
			0xef20cada, 0x36774c01, 0xd07e9efe, 0x2bf11fb4,
			0x95dbda4d, 0xae909198, 0xeaad8e71, 0x6b93d5a0,
			0xd08ed1d0, 0xafc725e0, 0x8e3c5b2f, 0x8e7594b7,
			0x8ff6e2fb, 0xf2122b64, 0x8888b812, 0x900df01c,
			0x4fad5ea0, 0x688fc31c, 0xd1cff191, 0xb3a8c1ad,
			0x2f2f2218, 0xbe0e1777, 0xea752dfe, 0x8b021fa1,
			0xe5a0cc0f, 0xb56f74e8, 0x18acf3d6, 0xce89e299,
			0xb4a84fe0, 0xfd13e0b7, 0x7cc43b81, 0xd2ada8d9,
			0x165fa266, 0x80957705, 0x93cc7314, 0x211a1477,
			0xe6ad2065, 0x77b5fa86, 0xc75442f5, 0xfb9d35cf,
			0xebcdaf0c, 0x7b3e89a0, 0xd6411bd3, 0xae1e7e49,
			0x00250e2d, 0x2071b35e, 0x226800bb, 0x57b8e0af,
			0x2464369b, 0xf009b91e, 0x5563911d, 0x59dfa6aa,
			0x78c14389, 0xd95a537f, 0x207d5ba2, 0x02e5b9c5,
			0x83260376, 0x6295cfa9, 0x11c81968, 0x4e734a41,
			0xb3472dca, 0x7b14a94a, 0x1b510052, 0x9a532915,
			0xd60f573f, 0xbc9bc6e4, 0x2b60a476, 0x81e67400,
			0x08ba6fb5, 0x571be91f, 0xf296ec6b, 0x2a0dd915,
			0xb6636521, 0xe7b9f9b6, 0xff34052e, 0xc5855664,
			0x53b02d5d, 0xa99f8fa1, 0x08ba4799, 0x6e85076a
		}, {
			0x4b7a70e9, 0xb5b32944, 0xdb75092e, 0xc4192623,
			0xad6ea6b0, 0x49a7df7d, 0x9cee60b8, 0x8fedb266,
			0xecaa8c71, 0x699a17ff, 0x5664526c, 0xc2b19ee1,
			0x193602a5, 0x75094c29, 0xa0591340, 0xe4183a3e,
			0x3f54989a, 0x5b429d65, 0x6b8fe4d6, 0x99f73fd6,
			0xa1d29c07, 0xefe830f5, 0x4d2d38e6, 0xf0255dc1,
			0x4cdd2086, 0x8470eb26, 0x6382e9c6, 0x021ecc5e,
			0x09686b3f, 0x3ebaefc9, 0x3c971814, 0x6b6a70a1,
			0x687f3584, 0x52a0e286, 0xb79c5305, 0xaa500737,
			0x3e07841c, 0x7fdeae5c, 0x8e7d44ec, 0x5716f2b8,
			0xb03ada37, 0xf0500c0d, 0xf01c1f04, 0x0200b3ff,
			0xae0cf51a, 0x3cb574b2, 0x25837a58, 0xdc0921bd,
			0xd19113f9, 0x7ca92ff6, 0x94324773, 0x22f54701,
			0x3ae5e581, 0x37c2dadc, 0xc8b57634, 0x9af3dda7,
			0xa9446146, 0x0fd0030e, 0xecc8c73e, 0xa4751e41,
			0xe238cd99, 0x3bea0e2f, 0x3280bba1, 0x183eb331,
			0x4e548b38, 0x4f6db908, 0x6f420d03, 0xf60a04bf,
			0x2cb81290, 0x24977c79, 0x5679b072, 0xbcaf89af,
			0xde9a771f, 0xd9930810, 0xb38bae12, 0xdccf3f2e,
			0x5512721f, 0x2e6b7124, 0x501adde6, 0x9f84cd87,
			0x7a584718, 0x7408da17, 0xbc9f9abc, 0xe94b7d8c,
			0xec7aec3a, 0xdb851dfa, 0x63094366, 0xc464c3d2,
			0xef1c1847, 0x3215d908, 0xdd433b37, 0x24c2ba16,
			0x12a14d43, 0x2a65c451, 0x50940002, 0x133ae4dd,
			0x71dff89e, 0x10314e55, 0x81ac77d6, 0x5f11199b,
			0x043556f1, 0xd7a3c76b, 0x3c11183b, 0x5924a509,
			0xf28fe6ed, 0x97f1fbfa, 0x9ebabf2c, 0x1e153c6e,
			0x86e34570, 0xeae96fb1, 0x860e5e0a, 0x5a3e2ab3,
			0x771fe71c, 0x4e3d06fa, 0x2965dcb9, 0x99e71d0f,
			0x803e89d6, 0x5266c825, 0x2e4cc978, 0x9c10b36a,
			0xc6150eba, 0x94e2ea78, 0xa5fc3c53, 0x1e0a2df4,
			0xf2f74ea7, 0x361d2b3d, 0x1939260f, 0x19c27960,
			0x5223a708, 0xf71312b6, 0xebadfe6e, 0xeac31f66,
			0xe3bc4595, 0xa67bc883, 0xb17f37d1, 0x018cff28,
			0xc332ddef, 0xbe6c5aa5, 0x65582185, 0x68ab9802,
			0xeecea50f, 0xdb2f953b, 0x2aef7dad, 0x5b6e2f84,
			0x1521b628, 0x29076170, 0xecdd4775, 0x619f1510,
			0x13cca830, 0xeb61bd96, 0x0334fe1e, 0xaa0363cf,
			0xb5735c90, 0x4c70a239, 0xd59e9e0b, 0xcbaade14,
			0xeecc86bc, 0x60622ca7, 0x9cab5cab, 0xb2f3846e,
			0x648b1eaf, 0x19bdf0ca, 0xa02369b9, 0x655abb50,
			0x40685a32, 0x3c2ab4b3, 0x319ee9d5, 0xc021b8f7,
			0x9b540b19, 0x875fa099, 0x95f7997e, 0x623d7da8,
			0xf837889a, 0x97e32d77, 0x11ed935f, 0x16681281,
			0x0e358829, 0xc7e61fd6, 0x96dedfa1, 0x7858ba99,
			0x57f584a5, 0x1b227263, 0x9b83c3ff, 0x1ac24696,
			0xcdb30aeb, 0x532e3054, 0x8fd948e4, 0x6dbc3128,
			0x58ebf2ef, 0x34c6ffea, 0xfe28ed61, 0xee7c3c73,
			0x5d4a14d9, 0xe864b7e3, 0x42105d14, 0x203e13e0,
			0x45eee2b6, 0xa3aaabea, 0xdb6c4f15, 0xfacb4fd0,
			0xc742f442, 0xef6abbb5, 0x654f3b1d, 0x41cd2105,
			0xd81e799e, 0x86854dc7, 0xe44b476a, 0x3d816250,
			0xcf62a1f2, 0x5b8d2646, 0xfc8883a0, 0xc1c7b6a3,
			0x7f1524c3, 0x69cb7492, 0x47848a0b, 0x5692b285,
			0x095bbf00, 0xad19489d, 0x1462b174, 0x23820e00,
			0x58428d2a, 0x0c55f5ea, 0x1dadf43e, 0x233f7061,
			0x3372f092, 0x8d937e41, 0xd65fecf1, 0x6c223bdb,
			0x7cde3759, 0xcbee7460, 0x4085f2a7, 0xce77326e,
			0xa6078084, 0x19f8509e, 0xe8efd855, 0x61d99735,
			0xa969a7aa, 0xc50c06c2, 0x5a04abfc, 0x800bcadc,
			0x9e447a2e, 0xc3453484, 0xfdd56705, 0x0e1e9ec9,
			0xdb73dbd3, 0x105588cd, 0x675fda79, 0xe3674340,
			0xc5c43465, 0x713e38d8, 0x3d28f89e, 0xf16dff20,
			0x153e21e7, 0x8fb03d4a, 0xe6e39f2b, 0xdb83adf7
		}, {
			0xe93d5a68, 0x948140f7, 0xf64c261c, 0x94692934,
			0x411520f7, 0x7602d4f7, 0xbcf46b2e, 0xd4a20068,
			0xd4082471, 0x3320f46a, 0x43b7d4b7, 0x500061af,
			0x1e39f62e, 0x97244546, 0x14214f74, 0xbf8b8840,
			0x4d95fc1d, 0x96b591af, 0x70f4ddd3, 0x66a02f45,
			0xbfbc09ec, 0x03bd9785, 0x7fac6dd0, 0x31cb8504,
			0x96eb27b3, 0x55fd3941, 0xda2547e6, 0xabca0a9a,
			0x28507825, 0x530429f4, 0x0a2c86da, 0xe9b66dfb,
			0x68dc1462, 0xd7486900, 0x680ec0a4, 0x27a18dee,
			0x4f3ffea2, 0xe887ad8c, 0xb58ce006, 0x7af4d6b6,
			0xaace1e7c, 0xd3375fec, 0xce78a399, 0x406b2a42,
			0x20fe9e35, 0xd9f385b9, 0xee39d7ab, 0x3b124e8b,
			0x1dc9faf7, 0x4b6d1856, 0x26a36631, 0xeae397b2,
			0x3a6efa74, 0xdd5b4332, 0x6841e7f7, 0xca7820fb,
			0xfb0af54e, 0xd8feb397, 0x454056ac, 0xba489527,
			0x55533a3a, 0x20838d87, 0xfe6ba9b7, 0xd096954b,
			0x55a867bc, 0xa1159a58, 0xcca92963, 0x99e1db33,
			0xa62a4a56, 0x3f3125f9, 0x5ef47e1c, 0x9029317c,
			0xfdf8e802, 0x04272f70, 0x80bb155c, 0x05282ce3,
			0x95c11548, 0xe4c66d22, 0x48c1133f, 0xc70f86dc,
			0x07f9c9ee, 0x41041f0f, 0x404779a4, 0x5d886e17,
			0x325f51eb, 0xd59bc0d1, 0xf2bcc18f, 0x41113564,
			0x257b7834, 0x602a9c60, 0xdff8e8a3, 0x1f636c1b,
			0x0e12b4c2, 0x02e1329e, 0xaf664fd1, 0xcad18115,
			0x6b2395e0, 0x333e92e1, 0x3b240b62, 0xeebeb922,
			0x85b2a20e, 0xe6ba0d99, 0xde720c8c, 0x2da2f728,
			0xd0127845, 0x95b794fd, 0x647d0862, 0xe7ccf5f0,
			0x5449a36f, 0x877d48fa, 0xc39dfd27, 0xf33e8d1e,
			0x0a476341, 0x992eff74, 0x3a6f6eab, 0xf4f8fd37,
			0xa812dc60, 0xa1ebddf8, 0x991be14c, 0xdb6e6b0d,
			0xc67b5510, 0x6d672c37, 0x2765d43b, 0xdcd0e804,
			0xf1290dc7, 0xcc00ffa3, 0xb5390f92, 0x690fed0b,
			0x667b9ffb, 0xcedb7d9c, 0xa091cf0b, 0xd9155ea3,
			0xbb132f88, 0x515bad24, 0x7b9479bf, 0x763bd6eb,
			0x37392eb3, 0xcc115979, 0x8026e297, 0xf42e312d,
			0x6842ada7, 0xc66a2b3b, 0x12754ccc, 0x782ef11c,
			0x6a124237, 0xb79251e7, 0x06a1bbe6, 0x4bfb6350,
			0x1a6b1018, 0x11caedfa, 0x3d25bdd8, 0xe2e1c3c9,
			0x44421659, 0x0a121386, 0xd90cec6e, 0xd5abea2a,
			0x64af674e, 0xda86a85f, 0xbebfe988, 0x64e4c3fe,
			0x9dbc8057, 0xf0f7c086, 0x60787bf8, 0x6003604d,
			0xd1fd8346, 0xf6381fb0, 0x7745ae04, 0xd736fccc,
			0x83426b33, 0xf01eab71, 0xb0804187, 0x3c005e5f,
			0x77a057be, 0xbde8ae24, 0x55464299, 0xbf582e61,
			0x4e58f48f, 0xf2ddfda2, 0xf474ef38, 0x8789bdc2,
			0x5366f9c3, 0xc8b38e74, 0xb475f255, 0x46fcd9b9,
			0x7aeb2661, 0x8b1ddf84, 0x846a0e79, 0x915f95e2,
			0x466e598e, 0x20b45770, 0x8cd55591, 0xc902de4c,
			0xb90bace1, 0xbb8205d0, 0x11a86248, 0x7574a99e,
			0xb77f19b6, 0xe0a9dc09, 0x662d09a1, 0xc4324633,
			0xe85a1f02, 0x09f0be8c, 0x4a99a025, 0x1d6efe10,
			0x1ab93d1d, 0x0ba5a4df, 0xa186f20f, 0x2868f169,
			0xdcb7da83, 0x573906fe, 0xa1e2ce9b, 0x4fcd7f52,
			0x50115e01, 0xa70683fa, 0xa002b5c4, 0x0de6d027,
			0x9af88c27, 0x773f8641, 0xc3604c06, 0x61a806b5,
			0xf0177a28, 0xc0f586e0, 0x006058aa, 0x30dc7d62,
			0x11e69ed7, 0x2338ea63, 0x53c2dd94, 0xc2c21634,
			0xbbcbee56, 0x90bcb6de, 0xebfc7da1, 0xce591d76,
			0x6f05e409, 0x4b7c0188, 0x39720a3d, 0x7c927c24,
			0x86e3725f, 0x724d9db9, 0x1ac15bb4, 0xd39eb8fc,
			0xed545578, 0x08fca5b5, 0xd83d7cd3, 0x4dad0fc4,
			0x1e50ef5e, 0xb161e6f8, 0xa28514d9, 0x6c51133c,
			0x6fd5c7e7, 0x56e14ec4, 0x362abfce, 0xddc6c837,
			0xd79a3234, 0x92638212, 0x670efa8e, 0x406000e0
		}, {
			0x3a39ce37, 0xd3faf5cf, 0xabc27737, 0x5ac52d1b,
			0x5cb0679e, 0x4fa33742, 0xd3822740, 0x99bc9bbe,
			0xd5118e9d, 0xbf0f7315, 0xd62d1c7e, 0xc700c47b,
			0xb78c1b6b, 0x21a19045, 0xb26eb1be, 0x6a366eb4,
			0x5748ab2f, 0xbc946e79, 0xc6a376d2, 0x6549c2c8,
			0x530ff8ee, 0x468dde7d, 0xd5730a1d, 0x4cd04dc6,
			0x2939bbdb, 0xa9ba4650, 0xac9526e8, 0xbe5ee304,
			0xa1fad5f0, 0x6a2d519a, 0x63ef8ce2, 0x9a86ee22,
			0xc089c2b8, 0x43242ef6, 0xa51e03aa, 0x9cf2d0a4,
			0x83c061ba, 0x9be96a4d, 0x8fe51550, 0xba645bd6,
			0x2826a2f9, 0xa73a3ae1, 0x4ba99586, 0xef5562e9,
			0xc72fefd3, 0xf752f7da, 0x3f046f69, 0x77fa0a59,
			0x80e4a915, 0x87b08601, 0x9b09e6ad, 0x3b3ee593,
			0xe990fd5a, 0x9e34d797, 0x2cf0b7d9, 0x022b8b51,
			0x96d5ac3a, 0x017da67d, 0xd1cf3ed6, 0x7c7d2d28,
			0x1f9f25cf, 0xadf2b89b, 0x5ad6b472, 0x5a88f54c,
			0xe029ac71, 0xe019a5e6, 0x47b0acfd, 0xed93fa9b,
			0xe8d3c48d, 0x283b57cc, 0xf8d56629, 0x79132e28,
			0x785f0191, 0xed756055, 0xf7960e44, 0xe3d35e8c,
			0x15056dd4, 0x88f46dba, 0x03a16125, 0x0564f0bd,
			0xc3eb9e15, 0x3c9057a2, 0x97271aec, 0xa93a072a,
			0x1b3f6d9b, 0x1e6321f5, 0xf59c66fb, 0x26dcf319,
			0x7533d928, 0xb155fdf5, 0x03563482, 0x8aba3cbb,
			0x28517711, 0xc20ad9f8, 0xabcc5167, 0xccad925f,
			0x4de81751, 0x3830dc8e, 0x379d5862, 0x9320f991,
			0xea7a90c2, 0xfb3e7bce, 0x5121ce64, 0x774fbe32,
			0xa8b6e37e, 0xc3293d46, 0x48de5369, 0x6413e680,
			0xa2ae0810, 0xdd6db224, 0x69852dfd, 0x09072166,
			0xb39a460a, 0x6445c0dd, 0x586cdecf, 0x1c20c8ae,
			0x5bbef7dd, 0x1b588d40, 0xccd2017f, 0x6bb4e3bb,
			0xdda26a7e, 0x3a59ff45, 0x3e350a44, 0xbcb4cdd5,
			0x72eacea8, 0xfa6484bb, 0x8d6612ae, 0xbf3c6f47,
			0xd29be463, 0x542f5d9e, 0xaec2771b, 0xf64e6370,
			0x740e0d8d, 0xe75b1357, 0xf8721671, 0xaf537d5d,
			0x4040cb08, 0x4eb4e2cc, 0x34d2466a, 0x0115af84,
			0xe1b00428, 0x95983a1d, 0x06b89fb4, 0xce6ea048,
			0x6f3f3b82, 0x3520ab82, 0x011a1d4b, 0x277227f8,
			0x611560b1, 0xe7933fdc, 0xbb3a792b, 0x344525bd,
			0xa08839e1, 0x51ce794b, 0x2f32c9b7, 0xa01fbac9,
			0xe01cc87e, 0xbcc7d1f6, 0xcf0111c3, 0xa1e8aac7,
			0x1a908749, 0xd44fbd9a, 0xd0dadecb, 0xd50ada38,
			0x0339c32a, 0xc6913667, 0x8df9317c, 0xe0b12b4f,
			0xf79e59b7, 0x43f5bb3a, 0xf2d519ff, 0x27d9459c,
			0xbf97222c, 0x15e6fc2a, 0x0f91fc71, 0x9b941525,
			0xfae59361, 0xceb69ceb, 0xc2a86459, 0x12baa8d1,
			0xb6c1075e, 0xe3056a0c, 0x10d25065, 0xcb03a442,
			0xe0ec6e0e, 0x1698db3b, 0x4c98a0be, 0x3278e964,
			0x9f1f9532, 0xe0d392df, 0xd3a0342b, 0x8971f21e,
			0x1b0a7441, 0x4ba3348c, 0xc5be7120, 0xc37632d8,
			0xdf359f8d, 0x9b992f2e, 0xe60b6f47, 0x0fe3f11d,
			0xe54cda54, 0x1edad891, 0xce6279cf, 0xcd3e7e6f,
			0x1618b166, 0xfd2c1d05, 0x848fd2c5, 0xf6fb2299,
			0xf523f357, 0xa6327623, 0x93a83531, 0x56cccd02,
			0xacf08162, 0x5a75ebb5, 0x6e163697, 0x88d273cc,
			0xde966292, 0x81b949d0, 0x4c50901b, 0x71c65614,
			0xe6c6c7bd, 0x327a140a, 0x45e1d006, 0xc3f27b9a,
			0xc9aa53fd, 0x62a80f00, 0xbb25bfe2, 0x35bdd2f6,
			0x71126905, 0xb2040222, 0xb6cbcf7c, 0xcd769c2b,
			0x53113ec0, 0x1640e3d3, 0x38abbd60, 0x2547adf0,
			0xba38209c, 0xf746ce76, 0x77afa1c5, 0x20756060,
			0x85cbfe4e, 0x8ae88dd8, 0x7aaaf9b0, 0x4cf9aa7e,
			0x1948c25c, 0x02fb8a8c, 0x01c36ae4, 0xd6ebe1f9,
			0x90d4f869, 0xa65cdea0, 0x3f09252d, 0xc208e69f,
			0xb74e6132, 0xce77e25b, 0x578fdfe3, 0x3ac372e6
		}
	}, {
		0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344,
		0xa4093822, 0x299f31d0, 0x082efa98, 0xec4e6c89,
		0x452821e6, 0x38d01377, 0xbe5466cf, 0x34e90c6c,
		0xc0ac29b7, 0xc97c50dd, 0x3f84d5b5, 0xb5470917,
		0x9216d5d9, 0x8979fb1b
	}
};

static unsigned char BF_itoa64[64 + 1] =
	"./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

static unsigned char BF_atoi64[0x60] = {
	64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 0, 1,
	54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 64, 64, 64, 64, 64,
	64, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 64, 64, 64, 64, 64,
	64, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 64, 64, 64, 64, 64
};

#define BF_safe_atoi64(dst, src) \
{ \
	tmp = (unsigned char)(src); \
	if ((unsigned int)(tmp -= 0x20) >= 0x60) return -1; \
	tmp = BF_atoi64[tmp]; \
	if (tmp > 63) return -1; \
	(dst) = tmp; \
}

int BF_decode(BF_word *dst, const char *src, int size)
{
	unsigned char *dptr = (unsigned char *)dst;
	unsigned char *end = dptr + size;
	const unsigned char *sptr = (const unsigned char *)src;
	unsigned int tmp, c1, c2, c3, c4;

	do {
		BF_safe_atoi64(c1, *sptr++);
		BF_safe_atoi64(c2, *sptr++);
		*dptr++ = (c1 << 2) | ((c2 & 0x30) >> 4);
		if (dptr >= end) break;

		BF_safe_atoi64(c3, *sptr++);
		*dptr++ = ((c2 & 0x0F) << 4) | ((c3 & 0x3C) >> 2);
		if (dptr >= end) break;

		BF_safe_atoi64(c4, *sptr++);
		*dptr++ = ((c3 & 0x03) << 6) | c4;
	} while (dptr < end);

	return 0;
}

static void BF_encode(char *dst, const BF_word *src, int size)
{
	const unsigned char *sptr = (const unsigned char *)src;
	const unsigned char *end = sptr + size;
	unsigned char *dptr = (unsigned char *)dst;
	unsigned int c1, c2;

	do {
		c1 = *sptr++;
		*dptr++ = BF_itoa64[c1 >> 2];
		c1 = (c1 & 0x03) << 4;
		if (sptr >= end) {
			*dptr++ = BF_itoa64[c1];
			break;
		}

		c2 = *sptr++;
		c1 |= c2 >> 4;
		*dptr++ = BF_itoa64[c1];
		c1 = (c2 & 0x0f) << 2;
		if (sptr >= end) {
			*dptr++ = BF_itoa64[c1];
			break;
		}

		c2 = *sptr++;
		c1 |= c2 >> 6;
		*dptr++ = BF_itoa64[c1];
		*dptr++ = BF_itoa64[c2 & 0x3f];
	} while (sptr < end);
}

static void BF_swap(BF_word *x, int count)
{
	static int endianness_check = 1;
	char *is_little_endian = (char *)&endianness_check;
	BF_word tmp;

	if (*is_little_endian)
	do {
		tmp = *x;
		tmp = (tmp << 16) | (tmp >> 16);
		*x++ = ((tmp & 0x00FF00FF) << 8) | ((tmp >> 8) & 0x00FF00FF);
	} while (--count);
}

#if BF_SCALE
/* Architectures which can shift addresses left by 2 bits with no extra cost */
#define BF_ROUND(L, R, N) \
	tmp1 = L & 0xFF; \
	tmp2 = L >> 8; \
	tmp2 &= 0xFF; \
	tmp3 = L >> 16; \
	tmp3 &= 0xFF; \
	tmp4 = L >> 24; \
	tmp1 = data.ctx.S[3][tmp1]; \
	tmp2 = data.ctx.S[2][tmp2]; \
	tmp3 = data.ctx.S[1][tmp3]; \
	tmp3 += data.ctx.S[0][tmp4]; \
	tmp3 ^= tmp2; \
	R ^= data.ctx.P[N + 1]; \
	tmp3 += tmp1; \
	R ^= tmp3;
#else
/* Architectures with no complicated addressing modes supported */
#define BF_INDEX(S, i) \
	(*((BF_word *)(((unsigned char *)S) + (i))))
#define BF_ROUND(L, R, N) \
	tmp1 = L & 0xFF; \
	tmp1 <<= 2; \
	tmp2 = L >> 6; \
	tmp2 &= 0x3FC; \
	tmp3 = L >> 14; \
	tmp3 &= 0x3FC; \
	tmp4 = L >> 22; \
	tmp4 &= 0x3FC; \
	tmp1 = BF_INDEX(data.ctx.S[3], tmp1); \
	tmp2 = BF_INDEX(data.ctx.S[2], tmp2); \
	tmp3 = BF_INDEX(data.ctx.S[1], tmp3); \
	tmp3 += BF_INDEX(data.ctx.S[0], tmp4); \
	tmp3 ^= tmp2; \
	R ^= data.ctx.P[N + 1]; \
	tmp3 += tmp1; \
	R ^= tmp3;
#endif

/*
 * Encrypt one block, BF_N is hardcoded here.
 */
#define BF_ENCRYPT \
	L ^= data.ctx.P[0]; \
	BF_ROUND(L, R, 0); \
	BF_ROUND(R, L, 1); \
	BF_ROUND(L, R, 2); \
	BF_ROUND(R, L, 3); \
	BF_ROUND(L, R, 4); \
	BF_ROUND(R, L, 5); \
	BF_ROUND(L, R, 6); \
	BF_ROUND(R, L, 7); \
	BF_ROUND(L, R, 8); \
	BF_ROUND(R, L, 9); \
	BF_ROUND(L, R, 10); \
	BF_ROUND(R, L, 11); \
	BF_ROUND(L, R, 12); \
	BF_ROUND(R, L, 13); \
	BF_ROUND(L, R, 14); \
	BF_ROUND(R, L, 15); \
	tmp4 = R; \
	R = L; \
	L = tmp4 ^ data.ctx.P[BF_N + 1];

#define BF_body() \
	L = R = 0; \
	ptr = data.ctx.P; \
	do { \
		ptr += 2; \
		BF_ENCRYPT; \
		*(ptr - 2) = L; \
		*(ptr - 1) = R; \
	} while (ptr < &data.ctx.P[BF_N + 2]); \
\
	ptr = data.ctx.S[0]; \
	do { \
		ptr += 2; \
		BF_ENCRYPT; \
		*(ptr - 2) = L; \
		*(ptr - 1) = R; \
	} while (ptr < &data.ctx.S[3][0xFF]);

static void BF_set_key(const char *key, size_t length, BF_key expanded, BF_key initial,
    unsigned char flags)
{
	const char *ptr = key;
	const char *end = ptr + length;
	unsigned int bug, i, j;
	BF_word safety, sign, diff, tmp[2];

/*
 * There was a sign extension bug in older revisions of this function.  While
 * we would have liked to simply fix the bug and move on, we have to provide
 * a backwards compatibility feature (essentially the bug) for some systems and
 * a safety measure for some others.  The latter is needed because for certain
 * multiple inputs to the buggy algorithm there exist easily found inputs to
 * the correct algorithm that produce the same hash.  Thus, we optionally
 * deviate from the correct algorithm just enough to avoid such collisions.
 * While the bug itself affected the majority of passwords containing
 * characters with the 8th bit set (although only a percentage of those in a
 * collision-producing way), the anti-collision safety measure affects
 * only a subset of passwords containing the '\xff' character (not even all of
 * those passwords, just some of them).  This character is not found in valid
 * UTF-8 sequences and is rarely used in popular 8-bit character encodings.
 * Thus, the safety measure is unlikely to cause much annoyance, and is a
 * reasonable tradeoff to use when authenticating against existing hashes that
 * are not reliably known to have been computed with the correct algorithm.
 *
 * We use an approach that tries to minimize side-channel leaks of password
 * information - that is, we mostly use fixed-cost bitwise operations instead
 * of branches or table lookups.  (One conditional branch based on password
 * length remains.  It is not part of the bug aftermath, though, and is
 * difficult and possibly unreasonable to avoid given the use of C strings by
 * the caller, which results in similar timing leaks anyway.)
 *
 * For actual implementation, we set an array index in the variable "bug"
 * (0 means no bug, 1 means sign extension bug emulation) and a flag in the
 * variable "safety" (bit 16 is set when the safety measure is requested).
 * Valid combinations of settings are:
 *
 * Prefix "$2a$": bug = 0, safety = 0x10000
 * Prefix "$2x$": bug = 1, safety = 0
 * Prefix "$2y$": bug = 0, safety = 0
 */
	bug = (unsigned int)flags & 1;
	safety = ((BF_word)flags & 2) << 15;

	sign = diff = 0;

	for (i = 0; i < BF_N + 2; i++) {
		tmp[0] = tmp[1] = 0;
		for (j = 0; j < 4; j++) {
			tmp[0] <<= 8;
			tmp[0] |= (unsigned char)*ptr; /* correct */
			tmp[1] <<= 8;
			tmp[1] |= (BF_word_signed)(signed char)*ptr; /* bug */
/*
 * Sign extension in the first char has no effect - nothing to overwrite yet,
 * and those extra 24 bits will be fully shifted out of the 32-bit word.  For
 * chars 2, 3, 4 in each four-char block, we set bit 7 of "sign" if sign
 * extension in tmp[1] occurs.  Once this flag is set, it remains set.
 */
			if (j)
				sign |= tmp[1] & 0x80;
			if (end <= ptr)
				ptr = key;
			else
				ptr++;
		}
		diff |= tmp[0] ^ tmp[1]; /* Non-zero on any differences */

		expanded[i] = tmp[bug];
		initial[i] = BF_init_state.P[i] ^ tmp[bug];
	}

/*
 * At this point, "diff" is zero iff the correct and buggy algorithms produced
 * exactly the same result.  If so and if "sign" is non-zero, which indicates
 * that there was a non-benign sign extension, this means that we have a
 * collision between the correctly computed hash for this password and a set of
 * passwords that could be supplied to the buggy algorithm.  Our safety measure
 * is meant to protect from such many-buggy to one-correct collisions, by
 * deviating from the correct algorithm in such cases.  Let's check for this.
 */
	diff |= diff >> 16; /* still zero iff exact match */
	diff &= 0xffff; /* ditto */
	diff += 0xffff; /* bit 16 set iff "diff" was non-zero (on non-match) */
	sign <<= 9; /* move the non-benign sign extension flag to bit 16 */
	sign &= ~diff & safety; /* action needed? */

/*
 * If we have determined that we need to deviate from the correct algorithm,
 * flip bit 16 in initial expanded key.  (The choice of 16 is arbitrary, but
 * let's stick to it now.  It came out of the approach we used above, and it's
 * not any worse than any other choice we could make.)
 *
 * It is crucial that we don't do the same to the expanded key used in the main
 * Eksblowfish loop.  By doing it to only one of these two, we deviate from a
 * state that could be directly specified by a password to the buggy algorithm
 * (and to the fully correct one as well, but that's a side-effect).
 */
	initial[0] ^= sign;
}

static char *BF_crypt(const char *key, size_t length, const char *setting,
	char *output, int size,
	BF_word min)
{
	static const unsigned char flags_by_subtype[26] =
		{2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 4, 0};
	struct {
		BF_ctx ctx;
		BF_key expanded_key;
		union {
			BF_word salt[4];
			BF_word output[6];
		} binary;
	} data;
	BF_word L, R;
	BF_word tmp1, tmp2, tmp3, tmp4;
	BF_word *ptr;
	BF_word count;
	int i;

	if (size < 7 + 22 + 31 + 1) {
		return NULL;
	}

	if (setting[0] != '$' ||
	    setting[1] != '2' ||
	    setting[2] < 'a' || setting[2] > 'z' ||
	    !flags_by_subtype[(unsigned int)(unsigned char)setting[2] - 'a'] ||
	    setting[3] != '$' ||
	    setting[4] < '0' || setting[4] > '3' ||
	    setting[5] < '0' || setting[5] > '9' ||
	    (setting[4] == '3' && setting[5] > '1') ||
	    setting[6] != '$') {
		return NULL;
	}

	count = (BF_word)1 << ((setting[4] - '0') * 10 + (setting[5] - '0'));
	if (count < min || BF_decode(data.binary.salt, &setting[7], 16)) {
		return NULL;
	}
	BF_swap(data.binary.salt, 4);

	BF_set_key(key, length, data.expanded_key, data.ctx.P,
	    flags_by_subtype[(unsigned int)(unsigned char)setting[2] - 'a']);

	memcpy(data.ctx.S, BF_init_state.S, sizeof(data.ctx.S));

	L = R = 0;
	for (i = 0; i < BF_N + 2; i += 2) {
		L ^= data.binary.salt[i & 2];
		R ^= data.binary.salt[(i & 2) + 1];
		BF_ENCRYPT;
		data.ctx.P[i] = L;
		data.ctx.P[i + 1] = R;
	}

	ptr = data.ctx.S[0];
	do {
		ptr += 4;
		L ^= data.binary.salt[(BF_N + 2) & 3];
		R ^= data.binary.salt[(BF_N + 3) & 3];
		BF_ENCRYPT;
		*(ptr - 4) = L;
		*(ptr - 3) = R;

		L ^= data.binary.salt[(BF_N + 4) & 3];
		R ^= data.binary.salt[(BF_N + 5) & 3];
		BF_ENCRYPT;
		*(ptr - 2) = L;
		*(ptr - 1) = R;
	} while (ptr < &data.ctx.S[3][0xFF]);

	do {
		int done;

		for (i = 0; i < BF_N + 2; i += 2) {
			data.ctx.P[i] ^= data.expanded_key[i];
			data.ctx.P[i + 1] ^= data.expanded_key[i + 1];
		}

		done = 0;
		do {
			BF_body();
			if (done)
				break;
			done = 1;

			tmp1 = data.binary.salt[0];
			tmp2 = data.binary.salt[1];
			tmp3 = data.binary.salt[2];
			tmp4 = data.binary.salt[3];
			for (i = 0; i < BF_N; i += 4) {
				data.ctx.P[i] ^= tmp1;
				data.ctx.P[i + 1] ^= tmp2;
				data.ctx.P[i + 2] ^= tmp3;
				data.ctx.P[i + 3] ^= tmp4;
			}
			data.ctx.P[16] ^= tmp1;
			data.ctx.P[17] ^= tmp2;
		} while (1);
	} while (--count);

	for (i = 0; i < 6; i += 2) {
		L = BF_magic_w[i];
		R = BF_magic_w[i + 1];

		count = 64;
		do {
			BF_ENCRYPT;
		} while (--count);

		data.binary.output[i] = L;
		data.binary.output[i + 1] = R;
	}

	memcpy(output, setting, 7 + 22 - 1);
	output[7 + 22 - 1] = BF_itoa64[(int)
		BF_atoi64[(int)setting[7 + 22 - 1] - 0x20] & 0x30];

/* This has to be bug-compatible with the original implementation, so
 * only encode 23 of the 24 bytes. :-) */
	BF_swap(data.binary.output, 6);
	BF_encode(&output[7 + 22], data.binary.output, 23);
	output[7 + 22 + 31] = '\0';

	return output;
}

int _crypt_output_magic(const char *setting, char *output, int size)
{
	if (size < 3)
		return -1;

	output[0] = '*';
	output[1] = '0';
	output[2] = '\0';

	if (setting[0] == '*' && setting[1] == '0')
		output[1] = '1';

	return 0;
}

/*
 * Please preserve the runtime self-test.  It serves two purposes at once:
 *
 * 1. We really can't afford the risk of producing incompatible hashes e.g.
 * when there's something like gcc bug 26587 again, whereas an application or
 * library integrating this code might not also integrate our external tests or
 * it might not run them after every build.  Even if it does, the miscompile
 * might only occur on the production build, but not on a testing build (such
 * as because of different optimization settings).  It is painful to recover
 * from incorrectly-computed hashes - merely fixing whatever broke is not
 * enough.  Thus, a proactive measure like this self-test is needed.
 *
 * 2. We don't want to leave sensitive data from our actual password hash
 * computation on the stack or in registers.  Previous revisions of the code
 * would do explicit cleanups, but simply running the self-test after hash
 * computation is more reliable.
 *
 * The performance cost of this quick self-test is around 0.6% at the "$2a$08"
 * setting.
 */
char *_crypt_blowfish_rn(const char *key, size_t length, const char *setting,
	char *output, int size)
{
	const char *test_key = "8b \xd0\xc1\xd2\xcf\xcc\xd8";
	const char *test_setting = "$2a$00$abcdefghijklmnopqrstuu";
	static const char * const test_hash[2] =
		{"VUrPmXD6q/nVSSp7pNDhCR9071IfIRe\0\x55", /* $2x$ */
		"i1D709vfamulimlGcq0qq3UvuUasvEa\0\x55"}; /* $2a$, $2y$ */
	char *retval;
	const char *p;
	int ok;
	struct {
		char s[7 + 22 + 1];
		char o[7 + 22 + 31 + 1 + 1 + 1];
	} buf;

/* Hash the supplied password */
	_crypt_output_magic(setting, output, size);
	retval = BF_crypt(key, length, setting, output, size, 16);

/*
 * Do a quick self-test.  It is important that we make both calls to BF_crypt()
 * from the same scope such that they likely use the same stack locations,
 * which makes the second call overwrite the first call's sensitive data on the
 * stack and makes it more likely that any alignment related issues would be
 * detected by the self-test.
 */
	memcpy(buf.s, test_setting, sizeof(buf.s));
	if (retval)
		buf.s[2] = setting[2];
	memset(buf.o, 0x55, sizeof(buf.o));
	buf.o[sizeof(buf.o) - 1] = 0;
	p = BF_crypt(test_key, strlen(test_key), buf.s, buf.o, sizeof(buf.o) - (1 + 1), 1);

	ok = (p == buf.o &&
	    !memcmp(p, buf.s, 7 + 22) &&
	    !memcmp(p + (7 + 22),
	    test_hash[(unsigned int)(unsigned char)buf.s[2] & 1],
	    31 + 1 + 1 + 1));

	{
		const char *k = "\xff\xa3" "34" "\xff\xff\xff\xa3" "345";
		BF_key ae, ai, ye, yi;
		BF_set_key(k, strlen(k), ae, ai, 2); /* $2a$ */
		BF_set_key(k, strlen(k), ye, yi, 4); /* $2y$ */
		ai[0] ^= 0x10000; /* undo the safety (for comparison) */
		ok = ok && ai[0] == 0xdb9c59bc && ye[17] == 0x33343500 &&
		    !memcmp(ae, ye, sizeof(ae)) &&
		    !memcmp(ai, yi, sizeof(ai));
	}

	if (ok)
		return retval;

/* Should not happen */
	_crypt_output_magic(setting, output, size);
	return NULL;
}

char *_crypt_gensalt_blowfish_rn(const char *prefix, unsigned long count,
	const char *input, int size, char *output, int output_size)
{
	if (size < 16 || output_size < 7 + 22 + 1 ||
	    (count && (count < 4 || count > 31)) ||
	    prefix[0] != '$' || prefix[1] != '2' ||
	    (prefix[2] != 'a' && prefix[2] != 'y')) {
		if (output_size > 0) output[0] = '\0';
		return NULL;
	}

	if (!count) count = 5;

	output[0] = '$';
	output[1] = '2';
	output[2] = prefix[2];
	output[3] = '$';
	output[4] = '0' + count / 10;
	output[5] = '0' + count % 10;
	output[6] = '$';

	BF_encode(&output[7], (const BF_word *)input, 16);
	output[7 + 22] = '\0';

	return output;
}

std::string bcrypt_iterated_128(std::string const& input) {
	static std::string const initializer(
		"\x03\xd5\xef\xf1\x34\xac\x9c\xda\x1f\xa3\x93\x38\x2e\x44\x93"
		"\x23\x81\xb9\x2a\xf1\xc1\x38\x4f\xd1\x75\xae\x58\x52\xfa\xd2"
		"\x90\xf1\xb6\x77\x24\xc2\x78\xbf\xa1\xe6\x3f\x14\x1b\xa3\x90"
		"\x55\xad\xa9\x71\x10\xa6\x1a\x9d\x15\x38\xe0\x00\xc1\x6d\x9c"
		"\x1f\x3c\x89\xac\x13\x5a\x56\x7d\x8d\x11\x85\x0b",
		72
	);
	static char const* setting = "$2a$04$abcdefghijklmnopqrstuu";
	std::string current_input = input;
	do {
		std::string output;
		std::string::size_type begin = 0;
		std::string::size_type end = 72;
		do {
			std::string::size_type initialized = (
				current_input.size() < end
			) ? current_input.size() : end;
			std::string const block = std::string(
				current_input.begin() + begin,
				current_input.begin() + initialized
			) + std::string(
				initializer.begin() + initialized - begin,
				initializer.end()
			);
			void* data = NULL;
			int size;
			char* hash = crypt_ra(
				block.data(),
				block.size(),
				setting,
				&data,
				&size
			);
			BF_word raw_data[6];
			BF_decode(raw_data, hash + 7 + 22, 31);
			free(data);
			output += std::string(
				reinterpret_cast<char const*>(raw_data),
				reinterpret_cast<
					char const*
				>(
					raw_data + 6
				) - reinterpret_cast<
					char const*
				>(
					raw_data
				) - 1
			);
			begin += 72;
			end += 72;
		} while (
			current_input.size() >= begin
		);
		current_input = output;
	} while (
		current_input.size() > 23
	);
	return current_input;
}

std::string bcrypt_iterated(std::string const& input) {
	std::string::size_type const split = input.size() * 3 / 4;
	std::string const concatenated = bcrypt_iterated_128(
		std::string(input.begin(), input.begin() + split)
	) + bcrypt_iterated_128(
		std::string(input.begin() + split, input.end())
	);
	return std::string(
		concatenated.begin(
		),
		concatenated.begin(
		) + 32
	);
}

