#include "board.h"

int get_lsb_index(U64 bitboard) {
  return bitboard ? __builtin_ctzll(bitboard) : -1;
}
#include "tt.h"

// switched black pieces to be used for white and same for black pieces
// due to better visual appearance
char *unicode_pieces[12] = {"♟", "♞", "♝", "♜", "♛", "♚",
                            "♙", "♘", "♗", "♖", "♕", "♔"};
char ascii_pieces[13] = "PNBRQKpnbrqk";
int decode_ascii_pieces[] = {
    ['P'] = wP, ['N'] = wN, ['B'] = wB, ['R'] = wR, ['Q'] = wQ, ['K'] = wK,
    ['p'] = bP, ['n'] = bN, ['b'] = bB, ['r'] = bR, ['q'] = bQ, ['k'] = bK,
};

U64 bitboards[12];        // pieces bbs
U64 sides_occupancies[3]; // sides
int piece_on_squares[64]; // for faster move gen + make, -1 = empty

int side_to_move = -1;
int can_castle; // WCK WCQ BCQ BCK
int en_passant = no_square;


const char *square_to_notation[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"
};

int char_to_square(const char *square) {
  if (!square) return no_square;

  char file = square[0];
  char rank = square[1];

  /* Validate that the two chars are valid file/rank digits */
  if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
    return no_square;
  }

  int file_index = file - 'a';                 /* 0..7 */
  int rank_index = 8 - (rank - '0');          /* '8'->0, '1'->7 */
  int idx = rank_index * 8 + file_index;      /* 0..63 */

  if (idx < 0 || idx >= no_square) return no_square;
  return idx;
}


const U64 not_A_file = 18374403900871474942ULL;
const U64 not_H_file = 9187201950435737471ULL;
const U64 not_HG_file = 4557430888798830399ULL;
const U64 not_AB_file = 18229723555195321596ULL;
const U64 not_B_file = 18302063728033398269ULL;
const U64 not_G_file = 13816973012072644543ULL;
const U64 not_rank_1 = 72057594037927935ULL;
const U64 not_rank_8 = 18446744073709551360ULL;
const U64 A_file = 0x0101010101010101ULL;
const U64 H_file = 0x8080808080808080ULL;

const U64 rank_1 = 0xFF00000000000000ULL;
const U64 rank_2 = 0x00FF000000000000ULL;
const U64 rank_7 = 0x000000000000FF00ULL;
const U64 rank_8 = 0x00000000000000FFULL;

U64 repetition_table[REP_TABLE_SIZE];
int repetition_index = 0;

// position repetition detection
int is_repetition()
{
    // loop over repetition indicies range
    for (int index = 0; index < repetition_index; index++)
        // if we found the hash key same with a current
        if (repetition_table[index] == hash_key)
          return 1;

  return 0;
}

void reset_states_and_board() {
  memset(bitboards, 0ULL, 96);
  memset(sides_occupancies, 0ULL, 24);
  memset(piece_on_squares, -1, sizeof(piece_on_squares));

  memset(repetition_table, 0ULL, sizeof(repetition_table));
  repetition_index = 0;

  can_castle = 0;
  en_passant = no_square;
  side_to_move = -1;
}

void set_sides_occupancies() {
  sides_occupancies[white] = bitboards[wP] | bitboards[wN] | bitboards[wB] |
                             bitboards[wR] | bitboards[wQ] | bitboards[wK];
  sides_occupancies[black] = bitboards[bP] | bitboards[bN] | bitboards[bB] |
                             bitboards[bR] | bitboards[bQ] | bitboards[bK];
  sides_occupancies[both] = sides_occupancies[white] | sides_occupancies[black];
}

// order: 8/7/6/5/4/3/2/1 (top to bottom) | 12345678 (left to right) /12345678
void parse_fen(char *fen) {
  reset_states_and_board();

  // Parse board position
  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8;) {
      int square = RF_2SQ(rank, file);

      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        int piece = decode_ascii_pieces[*fen++];
        set_bit(bitboards[piece], square);
        piece_on_squares[square] = piece;
        file++;
      } else if (*fen >= '1' && *fen <= '8') {
        file += *fen++ - '0';
      } else {
        fen++;
      }
    }
  }

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse side to move
  side_to_move = (*fen == 'w') ? white : black;
  fen++;

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse castling rights
  can_castle = 0;
  if (*fen != '-') {
    while (*fen != ' ') {
      switch (*fen++) {
      case 'K':
        can_castle |= WCK;
        break;
      case 'Q':
        can_castle |= WCQ;
        break;
      case 'k':
        can_castle |= BCK;
        break;
      case 'q':
        can_castle |= BCQ;
        break;
      }
    }
  } else {
    fen++;
  }

  // Skip spaces
  while (*fen == ' ')
    fen++;

  // Parse en passant square
  if (*fen == '-') {
    en_passant = no_square;
    fen++;
  } else {
    en_passant = char_to_square(fen);
    fen += 2;
  }

  // Skip remaining FEN components (ply and move count)
  while (*fen && *fen != ' ')
    fen++;

  // Finalize board states
  set_sides_occupancies();
  hash_key = update_hash_key();
}



/* **********************
1111 = qkQK = 15
wK    moved = 1111 & 1100 = 12
h1 wR moved = 1111 & 1110 = 14
a1 wR moved = 1111 & 1101 = 13

bK    moved = 1111 & 0011 = 3
h8 bR moved = 1111 & 1011 = 11
a8 bR moved = 1111 & 0111 = 7
************************* */

const int castling_rights[64] = {
    7,  15, 15, 15, 3,  15, 15, 11, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 13, 15, 15, 15, 12, 15, 15, 14,
};

int is_valid_encoded_move(int move) {
    int src = get_move_source(move);
    int dst = get_move_target(move);
    int pc  = get_move_piece(move);

    /* simple field range checks */
    if (src < 0 || src >= 64) return 0;
    if (dst < 0 || dst >= 64) return 0;
    if (pc  < 0 || pc  >= 12) return 0; /* piece values 0..11 */
    if (move == 0) return 0; // hopefully fixes null move a8a8 bug
    return 1;
}
int add_move(Moves *move_list, int move) {
  if (!move_list) return 0;
  if (!is_valid_encoded_move(move)) {
    /* malformed: ignore it */
    return 0;
  }
  if (move_list->count >= MOVES_CAPACITY) {
    /* capacity exceeded */
    return 0;
  }
  move_list->moves[move_list->count++] = move;
  return 1;
}

// --- add move helpers
char ascii_promoted_pieces[] = {[0] = '\0', // get_move_promoted_piece = '0000'
                                            // or 'wP' ? (illegal) => print '\0'
                                [wQ] = 'q', [wR] = 'r', [wB] = 'b', [wN] = 'n',
                                [bQ] = 'q', [bR] = 'r', [bB] = 'b', [bN] = 'n'};

// for UCI purposes
void print_move(int move) {
  printf("%s%s%c\n", square_to_notation[get_move_source(move)],
         square_to_notation[get_move_target(move)],
         ascii_promoted_pieces[get_move_promoted_piece(move)]);
}
char* get_move_str(int move) {
  static char buffer[8];
  snprintf(buffer, 8, "%s%s%c", square_to_notation[get_move_source(move)],
         square_to_notation[get_move_target(move)],
         ascii_promoted_pieces[get_move_promoted_piece(move)]);
  return buffer;
}



// for debugging purposes
void print_move_list(Moves *move_list) {
  printf("\n");

  if (!move_list->count)
    out("No moves in the list.");

  else {
    for (int i = 0; i < move_list->count; i++) {
      int move = move_list->moves[i];

      char *capture = get_move_capture_flag(move) ? "capture   " : "";
      char *castling_f = get_move_castling_flag(move) ? "castle   " : "";
      char *en_passant_f =
          get_move_en_passant_flag(move) ? "en_passant   " : "";
      char *double_push =
          get_move_double_push_flag(move) ? "double push   " : "";


      size_t len = strlen(capture) + strlen(castling_f) + strlen(en_passant_f) + strlen(double_push) + 1;
      char *out = malloc(len);
      if (!out) { perror("malloc"); exit(EXIT_FAILURE); }
      out[0] = '\0';
      strcat(out, capture);
      strcat(out, castling_f);
      strcat(out, en_passant_f);
      strcat(out, double_push);

      printf("%c %s-%s%c\t%s\n", ascii_pieces[get_move_piece(move)],
             square_to_notation[get_move_source(move)],
             square_to_notation[get_move_target(move)],
             ascii_promoted_pieces[get_move_promoted_piece(move)], out);

      free(out);
    }

    printf("\n\033[1;94mMoves: \033[1;93m%d\033[0;0m\n\n", move_list->count);
  }
}


/**********************************\
 ==================================

       Time controls variables

 ==================================
\**********************************/

// exit from engine flag
int quit = 0;
int movestogo = 30;
int movetime = -1;
int time = -1;

int inc = 0;
int starttime = 0;
int stoptime = 0;
int timeset = 0;
int stopped = 0;
int nodelimit = 0;

int ply;  // half-move counter

int killer_moves[2][MAX_PLY]; // [side][ply]
int history_moves[12][64]; // [piece][square]

// global variable for uci "ponderhit"
int ponder_move = 0;
int pondering = 0;
