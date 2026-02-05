#pragma once

#ifndef _UCI_H__
#define _UCI_H__

#include "search.h"
#include "helper.h"
#include <omp.h>

int input_waiting();
void read_input();
void communicate();
int parse_move(char *move_str);
void parse_position(char *command);
void parse_uci_makemoves(char *command);
void parse_go(char *command);
void run_bench(int depth);
void uci_loop();

#endif
