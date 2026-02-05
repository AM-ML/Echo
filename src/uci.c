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

// read GUI/user input
void read_input()
{
    // bytes to read holder
    ssize_t bytes;

    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting())
    {
        // loop to read bytes from STDIN
        do
        {
            // read bytes from STDIN
            bytes=read(fileno(stdin), input, 256);
        }

        // until bytes available
        while (bytes < 0);

        if (bytes == 0) return; // EOF

        // searches for the first occurrence of '\n'
        endc = strchr(input,'\n');

        // if found new line set value at pointer to 0
        if (endc) *endc=0;

        // if input is available
        if (strlen(input) > 0)
        {
            // match UCI "quit" command
            if (strncmp(input, "quit", 4) == 0)
            {
                // tell engine to terminate exacution
                quit = 1;
                stopped = 1;
            }

            // // match UCI "stop" command
            else if (strncmp(input, "stop", 4) == 0)    {
                // tell engine to terminate exacution
                stopped = 1;
            }
        }
    }
}

// a bridge function to interact between search and GUI input
void communicate() {
  // Only the master thread (thread 0) handles IO and Time Management
  if (omp_get_thread_num() == 0) {
    if(timeset == 1 && get_time_ms() > stoptime) {
      stopped = 1;
    }
    read_input();
  }
}

int parse_move(char *move_str) { // move_str: e2e4, e7e8q, etc.
  Moves ml;
  ml.count = 0;
  generate_moves(&ml);

  if (strlen(move_str) < 4) return 0;

  // safety check
  if (move_str[0] < 'a' || move_str[0] > 'h') return 0;
  if (move_str[2] < 'a' || move_str[2] > 'h') return 0;

  int src_sqr  = (move_str[0] - 'a') + ((8 - (move_str[1] - '0')) * 8);
  int dest_sqr = (move_str[2] - 'a') + ((8 - (move_str[3] - '0')) * 8);

  for (int i = 0; i < ml.count; i++) {
    int move = ml.moves[i];

    if (get_move_source(move) == src_sqr &&
        get_move_target(move) == dest_sqr) {

      int pp = get_move_promoted_piece(move);

      /* non-promotion move */
      if (pp == 0 && (move_str[4] == '\0' || isspace((unsigned char)move_str[4]))) {
        return move;
      }

      /* promotion moves (accept lower/upper case), accept both white & black promo constants */
      if (move_str[4] != '\0') {
        char promo = (char)tolower((unsigned char)move_str[4]);

        if ((pp == wQ || pp == bQ) && promo == 'q') return move;
        if ((pp == wR || pp == bR) && promo == 'r') return move;
        if ((pp == wN || pp == bN) && promo == 'n') return move;
        if ((pp == wB || pp == bB) && promo == 'b') return move;
      }
    }
  }

  return 0; // illegal / not found
}


/*  position startpos
 *  position startpos moves e2e4 e7e5
 *  position startpos fen 8/8/8/8/8/8/8/8/8 w - - moves e2e4
 *  */
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

/*
 * go
 * go moves e2e4
 * go depth 6 moves e2e4
 * go moves e2e4 movetime 300 depth 10
 * */
void parse_go(char *command)
{
    // init parameters
    int depth = -1;
    timeset = 0;
    nodelimit = 0;

    // init argument
    char *argument = NULL;


    // match UCI "binc" command
    if ((argument = strstr(command,"binc")) && side_to_move == black)
        // parse black time increment
        inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command,"winc")) && side_to_move == white)
        // parse white time increment
        inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command,"wtime")) && side_to_move == white)
        // parse white time limit
        time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command,"btime")) && side_to_move == black)
        // parse black time limit
        time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command,"movestogo")))
        // parse number of moves to go
        movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command,"movetime")))
        // parse amount of time allowed to spend to make a move
        movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command,"depth")))
        // parse search depth
        depth = atoi(argument + 6);

    if ((argument = strstr(command, "nodes")))
      nodelimit = atoi(argument + 6);

    // if move time is not available
    if(movetime != -1)
    {
        // set time equal to move time
        time = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = get_time_ms();

    // init search depth
    depth = depth;

    // if time control is available
    if(time != -1)
    {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        time /= movestogo;

        // "illegal" (empty) move bug fix
        if (time > 1500) time -= 50;

        // init stoptime
        stoptime = starttime + time + inc;
    }

    // if depth is not available
    if(depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // infinite search
    if ((argument = strstr(command,"infinite"))) {
      timeset = 0;
    }

    // print debug info
    printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
    time, starttime, stoptime, depth, timeset);

    // search position
    search_position(depth);
}

void parse_uci_ponderhit() {
  pondering = 0;
  stopped = 0;
}

void run_bench() {
    const char *fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1",
        "rnbq1rk1/pp2ppbp/3p1np1/2pP4/2P5/2N2NP1/PP2PPBP/R1BQK2R w KQ c6 0 7",
        "r3k2r/2pb1ppp/2pp1q2/p7/1P2P3/P1N1P3/2P3PP/R2QKB1R w KQkq - 0 1"
    };
    int num_fens = 8;
    int depth = 10;
    U64 total_nodes = 0;
    int start_time = get_time_ms();

    for (int i = 0; i < num_fens; i++) {
        printf("\nPosition %d/%d: %s\n", i + 1, num_fens, fens[i]);
        parse_fen((char*)fens[i]);
        search_position(depth);
        total_nodes += global_nodes;
    }

    int end_time = get_time_ms();
    int time_taken = end_time - start_time;
    if (time_taken == 0) time_taken = 1;

    printf("\n================================================\n");
    printf("Benchmark Results:\n");
    printf("Total Nodes: %llu\n", total_nodes);
    printf("Total Time: %d ms\n", time_taken);
    printf("NPS: %llu\n", (total_nodes * 1000) / (U64)time_taken);
    printf("================================================\n");
}


void uci_loop() {
  // clear buffer
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  // define input command (user or gui) length
  char input [2000];

  puts("id name echo");
  puts("id name am-ml");
  puts("uciok");

  while (1) {
    memset(input, 0, sizeof(input)); // clear command input
    fflush(stdout); // ensure output reach

    if (!fgets(input, 2000, stdin)) {
      break;
    }
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
        run_bench(); continue;
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
