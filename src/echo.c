#include "defs.h"
#include "bitboards.h"

int main(void)
{
  Board board; initBoard(&board);
  addPiece(&board.wPawns, A2);
  addPiece(&board.bKnights, G8);

  Shift_U(&board.wPawns, A2, 1);

  U64 position = getPos(&board);
  PrintBitBoard(position);
  PrintBitBoard(flipHorizontal(position));

	return 0;
}
