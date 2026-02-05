#include "magic.h"

// Relevant bit counts for sliders
const int relevant_bishop_count_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
    5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7,
    7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

const int relevant_rook_count_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

const int relevant_knight_count_bits[64] = {
    2, 3, 4, 4, 4, 4, 3, 2, 3, 4, 6, 6, 6, 6, 4, 3, 4, 6, 8, 8, 8, 8,
    6, 4, 4, 6, 8, 8, 8, 8, 6, 4, 4, 6, 8, 8, 8, 8, 6, 4, 4, 6, 8, 8,
    8, 8, 6, 4, 3, 4, 6, 6, 6, 6, 4, 3, 2, 3, 4, 4, 4, 4, 3, 2};

const int relevant_queen_count_bits[64] = {
    18, 16, 16, 16, 16, 16, 16, 18, 16, 15, 15, 15, 15, 15, 15, 16,
    16, 15, 17, 17, 17, 17, 15, 16, 16, 15, 17, 19, 19, 17, 15, 16,
    16, 15, 17, 19, 19, 17, 15, 16, 16, 15, 17, 17, 17, 17, 15, 16,
    16, 15, 15, 15, 15, 15, 15, 16, 18, 16, 16, 16, 16, 16, 16, 18};



U64 rook_magic_numbers[64] = {
  36046528791461889ULL,   306245049807544320ULL,  36063983539585032ULL,
  36046391353016324ULL,   4647723611581318144ULL, 144119654858752008ULL,
  36029346791555584ULL,   144115465103474948ULL,  2392539453718560ULL,
  2305984296994750464ULL, 864831934665064577ULL,  4620834023965460480ULL,
  576602039681811456ULL,  9147945333293184ULL,    562954382901760ULL,
  9232519977892855936ULL, 1188950576505815104ULL, 72198881818984451ULL,
  18692788199424ULL,      282574756782088ULL,     3467915199476925440ULL,
  563499776376834ULL,     153126785512374785ULL,  580966551230890244ULL,
  612630426397196288ULL,  70369820020736ULL,      35186527965184ULL,
  4611703612769306624ULL, 9223380835095543936ULL, 562967200407568ULL,
  36873226256777220ULL,   2305843292681797761ULL, 9295500000182141056ULL,
  576531189771280384ULL,  576532220844449792ULL,  4644405843593216ULL,
  140754676615170ULL,     576462953482552320ULL,  1168298213896ULL,
  619582587905ULL,        5764748535402627072ULL, 70369281081472ULL,
  216190374836732032ULL,  36046389339259008ULL,   8800389103620ULL,
  1125908505198596ULL,    1441153014902816770ULL, 566940008449ULL,
  18014948269498496ULL,   70437465751616ULL,      2377918195974570112ULL,
  8798241554560ULL,       4647723613687120000ULL, 306385529329549440ULL,
  288511859718619392ULL,  4574417011200ULL,       35461397544977ULL,
  1407514638827521ULL,    281518196916289ULL,     72620819406652434ULL,
  9223935021436641282ULL, 281483633756161ULL,     1168243820548ULL,
  281477128397313ULL
};

U64 bishop_magic_numbers[64] = {
  146369204096991744ULL,  1134698216587296ULL,
  9234774526519672896ULL, 73187928636915974ULL,
  9306541899907200ULL,    2401470968299648ULL,
  1153203563787190304ULL, 2306478569902129168ULL,
  4708492640384ULL,       11602416136897986688ULL,
  9354443096064ULL,       612498483932299264ULL,
  9224502352525000704ULL, 36029355903680512ULL,
  2452211101044195328ULL, 72057870056955904ULL,
  580964627081470080ULL,  9007268003645696ULL,
  40532431010283528ULL,   45071326708252672ULL,
  4786209012842496ULL,    1143638126102528ULL,
  22588371511697408ULL,   288389259944034816ULL,
  4648744022708752ULL,    1143509809762432ULL,
  146384891594948672ULL,  2895818958449938464ULL,
  281543712980992ULL,     140874960998400ULL,
  283678311747584ULL,     316951423615104ULL,
  4521260533482016ULL,    2256266579951872ULL,
  70574970241088ULL,      145273108824320ULL,
  74783970574592ULL,      4611985089885119490ULL,
  1169916946948224ULL,    2308101958000771648ULL,
  299102059365889ULL,     74775515897856ULL,
  282025823080457ULL,     137841608960ULL,
  8800455114816ULL,       1193612299649155136ULL,
  1155175520628900864ULL, 2252900541661696ULL,
  9298807898891682112ULL, 35751576731648ULL,
  1101793591296ULL,       545800192ULL,
  2306265506493038592ULL, 4611757555436257280ULL,
  2269410261991424ULL,    1143500689178760ULL,
  73254008854152256ULL,   344805869568ULL,
  4303881216ULL,          4611706016336314880ULL,
  21990503096840ULL,      141801095680ULL,
  2305847441654022696ULL, 144695738814432384ULL
};

// Map index to occupancy bitmask
U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
  U64 occupancy = 0ULL;

  for (int count = 0; count < bits_in_mask; count++) {
    int square = get_lsb_index(attack_mask); // from top left to bottom right
    pop_bit(attack_mask, square);

    if ((U64)index & (1ULL << count)) { // if square in specified bit array
      set_bit(occupancy, square);       // add bit
    }
  }

  return occupancy;
}

// PRNG state
unsigned int state = 1804289383;

unsigned int get_random_32() {
  unsigned int number = state;
  number ^= number << 13;
  number ^= number >> 17;
  number ^= number << 5;

  state = number;

  return number;
}

U64 get_random_64() {
  U64 n1, n2, n3, n4;

  n1 = (U64)(get_random_32()) & 0XFFFF;
  n2 = (U64)(get_random_32()) & 0XFFFF;
  n3 = (U64)(get_random_32()) & 0XFFFF;
  n4 = (U64)(get_random_32()) & 0XFFFF;

  return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

U64 gen_magic_number() {
  return get_random_64() & get_random_64() & get_random_64() & get_random_64();
}

// Find magic number for a square
U64 find_magic_number(int square, int relevant_bits_count, int flag) {
  U64 occupancies[4096];
  U64 attacks[4096];
  U64 used_attacks[4096];

  U64 attack_mask = flag ? mask_bishop_attacks(square) : mask_rook_attacks(square);
  U64 occupancy_indicies = 1 << relevant_bits_count;

  for (int index = 0; index < occupancy_indicies; index++) {
    occupancies[index] = set_occupancy(index, relevant_bits_count, attack_mask);
    attacks[index] = flag ? relevant_bishop_attacks(square, occupancies[index])
                          : relevant_rook_attacks(square, occupancies[index]);
  }

  for (int random_count = 0; random_count < 800000000; random_count++) {
    U64 magic_number = gen_magic_number();

    if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6)
      continue;

    memset(used_attacks, 0ULL, sizeof(used_attacks));

    int index, fail;

    // loop over occupancy indicies
    for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {
      int magic_index = (int)((occupancies[index] * magic_number) >>
                              (64 - relevant_bits_count));

      if (used_attacks[magic_index] == 0ULL)
        used_attacks[magic_index] = attacks[index];
      else if (used_attacks[magic_index] != attacks[index])
        fail = 1;
    }

    if (!fail)
      return magic_number;
  }
  printf("\b    attempt failed.\n");
  return 0ULL;
}

void init_magic_numbers() {
  for (int square = 0; square < 64; square++) {
    rook_magic_numbers[square] =
        find_magic_number(square, relevant_rook_count_bits[square], rook);
    printf("%lluULL, ", rook_magic_numbers[square]);
  }
  printf("\n");
  for (int square = 0; square < 64; square++) {
    bishop_magic_numbers[square] =
        find_magic_number(square, relevant_bishop_count_bits[square], bishop);
    printf("%lluULL, ", bishop_magic_numbers[square]);
  }
  printf("\n");
}
