/*
 *
 * Operating System Design / Diseño de Sistemas Operativos
 * (c) ARCOS.INF.UC3M.ES
 *
 * @file 	auxiliary.h
 * @brief 	Headers for the auxiliary functions required by filesystem.c.
 * @date	Last revision 01/04/2020
 *
 */


int metadata_setDefault (void);

int metadata_writeToDisk (void);

int metada_readFromDisk(void);

int namei ( char *fname, int tipo);

int ialloc (void);

int alloc (void);

int ifree( int inodo_id);

int bfree( int block_id);
