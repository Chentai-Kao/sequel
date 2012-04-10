/*
 * utils.h -- Some useful tools.
 * Copyright (C) 2012  Cobra Kao  <ru6xul6@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _UTILS
#define _UTILS

#include "selfDefineVars.h"

/* Print info when program starts. */
void print_info();

/* Print the prompt. */
void print_prompt(enum prompt_choice_t p);

/* Print the help page. */
void print_help();

/* Print the syntax error warning. */
void print_syntax_error(char* s);

/* Check the trailing ';' of command. Return 0 if no ';', 1 if ';' exists. */
int check_trailing_colon(char* s);

/* Check the starting dot '.' of command. Return 0 if no '.', 1 otherwise. */
int is_system_cmd(char* s);

/* Print the syntax error for system commands. */
void print_system_cmd_syntax_error(char* s);

/* Remove the printed characters in console. 'num' of characters backwards. */
void clear_backward(int num);

#endif
