#include <stdio.h>

int main(void) {
  for (int rank = 8; rank >= 1; rank--) {
    printf("\"A%d\", \"B%d\", \"C%d\", \"D%d\", \"E%d\", \"F%d\", \"G%d\", \"H%d\",\n",
      rank, rank, rank, rank, rank, rank, rank, rank);
  }

  return 0;
}
