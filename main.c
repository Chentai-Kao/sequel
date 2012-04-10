/*
 * main.c -- Main program
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "selfDefineVars.h"
#include "utils.h"
#include "cmdParser.h"

int main(int argc, char* argv[])
{
  /* Some major variables. */
  char input_str_buf[INPUT_STR_SIZE] = ""; // input from user (buffer)
  char input_str[INPUT_STR_SIZE] = ""; // input from user
  n_SQL_table = 0; // number of SQL tables
  system_output_mode = LIST; // output as csv format
  sys_out_stream = stdout;
  sys_in_stream = stdin;
  sys_echo = OFF;

  /* When file name is given as the first argument, read file. */
  FILE* in_file;
  if (argc == 1)
    in_file = stdin;
  else {
    if ((in_file = fopen(argv[1], "r")) == NULL) {
      fprintf(stderr, "Error: cannot open file \"%s\"\n", argv[1]);
      exit(EXIT_FAILURE);
    }
  }

  /* Command line. */
  prompt_choice = TEXT_PROMPT;
  print_info();
  while (1) {
    print_prompt(prompt_choice);
    if (fgets(input_str_buf, INPUT_STR_SIZE, sys_in_stream) == NULL) {
      if (sys_in_stream != stdin)
        fclose(sys_in_stream);
      sys_in_stream = stdin;
      clear_backward(8);
      continue;
    }

    trim_trailing_endline(input_str_buf);
    if (strlen(input_str) != 0)
      strcat(input_str, " ");
    strcat(input_str, input_str_buf);
    if (is_system_cmd(input_str))
      exec_system_cmd(input_str);
    else if (check_trailing_colon(input_str) == 0) {
      prompt_choice = DOT_PROMPT;
      continue;
    }
    else {
      trim_spaces_separate_marks(input_str);
      if ((sys_in_stream != stdin) || (sys_echo == ON))
        fprintf(sys_out_stream, "%s\n", input_str);
      exec_SQL_cmd(input_str);
    }

    /* Reset. */
    prompt_choice = TEXT_PROMPT;
    input_str_buf[0] = '\0';
    input_str[0] = '\0';
  }

  exit(EXIT_SUCCESS);
}
