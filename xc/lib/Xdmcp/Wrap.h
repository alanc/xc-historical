/*
 * header file for compatibility with kerberos DES routines
 */

typedef unsigned char des_cblock[8];	/* crypto-block size */

typedef struct des_ks_struct { des_cblock _; } des_key_schedule[16];
