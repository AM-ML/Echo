#pragma once

#ifndef _UCI_H__
#define _UCI_H__

#include "search.h"
#include "helper.h"

int input_waiting();

// read GUI/user input
void read_input();


// a bridge function to interact between search and GUI input
void communicate();
int parse_move(char *move_str);
void parse_position(char *command);
void parse_uci_makemoves(char *command);
void parse_go(char *command);
void uci_loop();

#endif
