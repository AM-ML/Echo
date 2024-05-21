#include <stdio.h>
#include <byteswap.h>


#define File_LTBEnd(l_end) (l_end ^ 7)  // file-rank little to big endian mapping conversion
#define File_BTLEnd(b_end) (b_end ^ 7)  // file-rank big to little endian mapping conversion
#define Rank_LTBEnd(l_end) (l_end ^ 56) // rank-file little to big endian mapping conversion
#define Rank_BTLEnd(b_end) (b_end ^ 56) // rank-file big to little endian mapping conversion

// diagonal bit shifting movements
#define UpLeft(index) (index+7)
#define UpRight(index) (index+9)
#define DownRight(index) (index-7)
#define DownLeft(index) (index-9)

// orthognal bit shifting movements
#define Up(index) (index+8)
#define Right(index) (index+1)
#define Left(index) (index-1)
#define Down(index) (index-8)

// chess board square conversion
#define FR2SQ(f, r) (8*r + f)
#define bitScanLSB(bb) (__builtin_ctzll(bb))
#define bitScanMSB(bb) (__builtin_clzll(bb))

struct CBoard
{
	// Pieces BitBoard
	U64 wPawns, wKnights, wBishops, wRooks, wQueens, wKing,
		bPawns, bKnights, bBishops, bRooks, bQueens, bKing;

	U64 Pieces; // combines all pieces bitboards, wPawns | wKnights | ...

};

// functions
void addPiece(U64 *bb, int square);
void removePiece(U64 *bb, int index);

// board twisters
U64 flipVertical(U64 x) (bswap_64(x));


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
