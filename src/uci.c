#include "uci.h"

int input_waiting()
{
#ifndef WIN32
  fd_set readfds;
  struct timeval tv;
  FD_ZERO (&readfds);
  FD_SET (fileno(stdin), &readfds);
  tv.tv_sec=0; tv.tv_usec=0;
  select(16, &readfds, 0, 0, &tv);

  return (FD_ISSET(fileno(stdin), &readfds));
#else
  static int init = 0, pipe;
  static HANDLE inh;
  DWORD dw;

  if (!init)
  {
    init = 1;
    inh = GetStdHandle(STD_INPUT_HANDLE);
    pipe = !GetConsoleMode(inh, &dw);
    if (!pipe)
    {
      SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
      FlushConsoleInputBuffer(inh);
    }
  }

  if (pipe)
  {
    if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
    return dw;
  }

  else
{
    GetNumberOfConsoleInputEvents(inh, &dw);
    return dw <= 1 ? 0 : dw;
  }

#endif
}

// Read input from stdin
void read_input() {
  ssize_t bytes;
  char input[256] = "", *endc;

  if (input_waiting()) {
    stopped = 1;
    do {
      bytes = read(fileno(stdin), input, 256);
    } while (bytes < 0);

    endc = strchr(input, '\n');
    if (endc) *endc = 0;

    if (strlen(input) > 0) {
      if (strncmp(input, "quit", 4) == 0) quit = 1;
      else if (strncmp(input, "stop", 4) == 0) quit = 1;
    }
  }
}

// Handle IO and time management
void communicate() {
  if (omp_get_thread_num() == 0) {
    if (timeset == 1 && get_time_ms() > stoptime) {
      stopped = 1;
    }
    read_input();
  }
}

// Parse UCI move string
int parse_move(char *move_str) {
  Moves ml;
  ml.count = 0;
  generate_moves(&ml);

  if (strlen(move_str) < 4) return 0;
  if (move_str[0] < 'a' || move_str[0] > 'h') return 0;
  if (move_str[2] < 'a' || move_str[2] > 'h') return 0;

  int src_sqr = (move_str[0] - 'a') + ((8 - (move_str[1] - '0')) * 8);
  int dest_sqr = (move_str[2] - 'a') + ((8 - (move_str[3] - '0')) * 8);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    if (get_move_source(move) == src_sqr && get_move_target(move) == dest_sqr) {
      int pp = get_move_promoted_piece(move);

      if (pp == 0 && (move_str[4] == '\0' || isspace((unsigned char)move_str[4]))) {
        return move;
      }

      // Handle promotions
      if (move_str[4] != '\0') {
        char promo = (char)tolower((unsigned char)move_str[4]);
        if ((pp == wQ || pp == bQ) && promo == 'q') return move;
        if ((pp == wR || pp == bR) && promo == 'r') return move;
        if ((pp == wN || pp == bN) && promo == 'n') return move;
        if ((pp == wB || pp == bB) && promo == 'b') return move;
      }
    }
  }
  return 0;
}

// Parse position command
void parse_position(char *command) {
  command += 9; // skip "position "
  char *cur_char = command;

  // parse "startpos" cmd
  if(strncmp(command, "startpos", 8) == 0) {
    cur_char += 8;
    parse_fen(start_position);
  } else {
    cur_char = strstr(command, "fen");
    if (!cur_char) { parse_fen(start_position); }
    else {
      cur_char += 4;
      parse_fen(cur_char);
    }
  }

  cur_char = strstr(cur_char, "moves");
  if(cur_char) {
    cur_char += 6;

    while(*cur_char) {
      int move = parse_move(cur_char);
      if (!move) { break; }

      repetition_index++;
      repetition_table[repetition_index] = hash_key;

      make_move(move, allow_all_moves);
      while(*cur_char && *cur_char != ' ') {cur_char++;}
      cur_char++;
    }
  }
}

void parse_uci_makemoves(char *command) {
  command += 10;
  char *cur_char = command;

  while (*cur_char) {
    int move = parse_move(cur_char);
    if (!move) { break; }
    make_move(move, allow_all_moves);
    while(*cur_char && *cur_char != ' ') {cur_char++;}
    cur_char++;
  }
}

// Parse go command
void parse_go(char *command) {
    int depth = -1;
    timeset = 0;
    nodelimit = 0;

    // init argument
    char *argument = NULL;


    if ((argument = strstr(command, "binc")) && side_to_move == black)
        inc = atoi(argument + 5);

    if ((argument = strstr(command, "winc")) && side_to_move == white)
        inc = atoi(argument + 5);

    if ((argument = strstr(command, "wtime")) && side_to_move == white)
        time = atoi(argument + 6);

    if ((argument = strstr(command, "btime")) && side_to_move == black)
        time = atoi(argument + 6);

    if ((argument = strstr(command, "movestogo")))
        movestogo = atoi(argument + 10);

    if ((argument = strstr(command, "movetime")))
        movetime = atoi(argument + 9);

    if ((argument = strstr(command, "depth")))
        depth = atoi(argument + 6);

    if ((argument = strstr(command, "nodes")))
        nodelimit = atoi(argument + 6);

    if (movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    starttime = get_time_ms();

    if (time != -1) {
        timeset = 1;
        time /= movestogo;
        if (time > 1500) time -= 50;
        stoptime = starttime + time + inc;
    }

    if (depth == -1) depth = 64;

    if ((argument = strstr(command, "infinite"))) timeset = 0;

    // Print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
           time, starttime, stoptime, depth, timeset);

    // Start search
    search_position(depth);
}

void run_bench(int depth) {
  char *bench_fens[] = {
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
      "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
      "8/k7/3p4/p2P1p2/P2P1p2/8/8/K7 w - - 0 1",
      "rnbq1rk1/pp2ppbp/3p1np1/2pP4/2P5/2N2NP1/PP2PPBP/R1BQK2R w KQ c6 0 7",
      "r3k2r/2pb1ppp/2pp1q2/p7/1P2P3/P1N1P3/2P3PP/R2QKB1R w KQkq - 0 1"};

  U64 total_nodes = 0;
  int start_time = get_time_ms();

  for (int i = 0; i < 8; i++) {
    printf("\nPosition %d/8: %s\n", i + 1, bench_fens[i]);
    parse_fen(bench_fens[i]);
    clear_tt();
    search_position(depth);
    total_nodes += global_nodes;
  }

  int total_time = get_time_ms() - start_time;
  printf("\n================================================ Benchmark Results: Total Nodes: %llu Total Time: %d ms NPS: %llu\n", total_nodes,
         total_time, (total_time > 0) ? (total_nodes * 1000 / (U64)total_time) : 0);
}

void parse_uci_ponderhit() {
  pondering = 0;
  stopped = 0;
}


void uci_loop() {
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  char input[2000];

  puts("id name echo");
  puts("id name am-ml");
  puts("uciok");

  while (1) {
    memset(input, 0, sizeof(input));
    fflush(stdout);

    if (!fgets(input, 2000, stdin)) break;
    if (input[0] == '\n') continue;

    if (strncmp(input, "isready", 7) == 0) {
      puts("readyok"); continue;
    }

    if (strncmp(input, "position", 8) == 0) {
      parse_position(input); clear_tt();
      continue;
    }

    if (strncmp(input, "makemoves", 9) == 0) {
      parse_uci_makemoves(input);
      continue;
    }
    if (strncmp(input, "ucinewgame", 10) == 0) {
      parse_position("position startpos"); clear_tt();
      continue;
    }

    if (strncmp(input, "go", 2) == 0) {
      parse_go(input); continue;
    }

    if (strncmp(input, "bench", 5) == 0) {
      int depth = 10;
      if (input[6] != '\0') {
        depth = atoi(input + 6);
      }
      run_bench(depth);
      continue;
    }

    if (strncmp(input, "eval", 4) == 0) {
      printf("eval: %d\n", eval()); continue;
    }

    if (strncmp(input, "quit", 4) == 0 ) {
      break; continue;
    }

    if (strncmp(input, "uci", 3) == 0) {
      puts("uciok"); continue;
    }

    if (strncmp(input, "print", 5) == 0) {
      print_board(1); continue;
    }
    if (strncmp(input, "ponderhit", 9) == 0) {
      parse_uci_ponderhit();
      continue;
    }
    if (strncmp(input, "ponder", 6) == 0) {
      pondering = 1;
      timeset = 0;
    }
  }
}
