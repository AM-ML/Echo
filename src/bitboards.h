#ifndef _BITBOARDS_HHH_
#define _BITBOARDS_HHH_

#include <stdio.h>
#include <byteswap.h>
#include "defs.h"


#define File_LTBEnd(l_end) (l_end ^ 7)  // file-rank little to big endian mapping conversion
#define File_BTLEnd(b_end) (b_end ^ 7)  // file-rank big to little endian mapping conversion
#define Rank_LTBEnd(l_end) (l_end ^ 56) // rank-file little to big endian mapping conversion
#define Rank_BTLEnd(b_end) (b_end ^ 56) // rank-file big to little endian mapping conversion

/* diagonal bit shifting Predictors */
#define UpLeft(index) (index+7)
#define UpRight(index) (index+9)
#define DownRight(index) (index-7)
#define DownLeft(index) (index-9)

/* orthognal bit shifting Predictors */
#define Up(index) (index+8)
#define Right(index) (index+1)
#define Left(index) (index-1)
#define Down(index) (index-8)

/* chess board square conversion */
#define FR2SQ(f, r) (8*r + f)
// finds the index of the lowest piece, or the number of empty bottom squares in bb
#define bitScanLSB(bb) (__builtin_ctzll(bb))
// finds the index of the highest piece or,
// you get the #empty squares from the top of bb by doing (64 - bitScanMSB(bb))
#define bitScanMSB(bb) (__builtin_clzll(bb))

// functions
void addPiece(U64 *bb, int square);
void removePiece(U64 *bb, int index);

// board twisters
#define flipVertical(x) (bswap_64(x));


// piece shifters / movers (potential to precalculate the output based on input)
void Shift_U(U64 *bb, int index, int times);
void Shift_L(U64 *bb, int index, int times);
void Shift_D(U64 *bb, int index, int times);
void Shift_R(U64 *bb, int index, int times);

void Shift_UR(U64 *bb, int index, int times);
void Shift_UL(U64 *bb, int index, int times);
void Shift_DR(U64 *bb, int index, int times);
void Shift_DL(U64 *bb, int index, int times);

void PrintBitBoard(U64 bb);

typedef struct
{
	// Pieces BitBoard
	U64 wPawns, wKnights, wBishops, wRooks, wQueens, wKing,
		bPawns, bKnights, bBishops, bRooks, bQueens, bKing;

} Board;

void initBoard(Board *board);
U64 getPos(Board *board);
U64 flipHorizontal (U64 x);

#endif
