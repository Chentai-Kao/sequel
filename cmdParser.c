/*
 * cmdParser.c -- Parsing the input command.
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

#include "cmdParser.h"
#include "utils.h"
#include "sqlInterpreter.h"

void
trim_trailing_endline(char* s)
{
  if (s[strlen(s) - 1] == '\n')
    s[strlen(s) - 1] = '\0';

  return;
}

void exec_system_cmd(char* s)
{
  char s_cpy[INPUT_STR_SIZE]; // for syntax error output
  strcpy(s_cpy, s);

  char* cmd = strtok(s, ". \t");
  /* ".help". */
  if (strcmp(cmd, "help") == 0)
    print_help();
  /* ".dump". */
  else if (strcmp(cmd, "dump") == 0) {
    if ((cmd = strtok(NULL, " \t")) == NULL)
      dump_all_tables();
    else {
      while (cmd != NULL) {
        dump_single_table_by_name(cmd);
        cmd = strtok(NULL, " \t");
      }
    }
  }
  /* ".echo". */
  else if (strcmp(cmd, "echo") == 0) {
    if ((cmd =  strtok(NULL, " \t")) != NULL) {
      if ((strcmp(cmd, "ON") == 0) || (strcmp(cmd, "on") == 0))
        sys_echo = ON;
      else if ((strcmp(cmd, "OFF") == 0) || (strcmp(cmd, "off") == 0))
        sys_echo = OFF;
      else
        print_system_cmd_syntax_error(s_cpy);
    }
    else
      print_system_cmd_syntax_error(s_cpy);
  }
  /* ".exit", ".quit". */
  else if ((strcmp(cmd, "exit") == 0) || (strcmp(cmd, "quit") == 0)) {
    if (sys_out_stream != stdout)
      fclose(sys_out_stream);
    exit(EXIT_SUCCESS);
  }
  /* ".read". */
  else if (strcmp(cmd, "read") == 0) {
    if ((cmd = strtok(NULL, " \t")) != NULL) {
      if ((sys_in_stream = fopen(cmd, "r")) == NULL) {
        fprintf(stderr, "Error: cannot open \"%s\"\n", cmd);
        sys_in_stream = stdin;
      }
    }
    else
      print_system_cmd_syntax_error(s_cpy);
  }
  /* ".schema". */
  else if (strcmp(cmd, "schema") == 0) {
    if ((cmd = strtok(NULL, " \t")) == NULL)
      schema_all_tables();
    else {
      while (cmd != NULL) {
        schema_single_table_by_name(cmd);
        cmd = strtok(NULL, " \t");
      }
    }
  }
  /* ".mode". */
  else if (strcmp(cmd, "mode") == 0) {
    if ((cmd = strtok(NULL, ". \t")) != NULL) {
      if (strcmp(cmd, "csv") == 0)
        system_output_mode = CSV;
      else if (strcmp(cmd, "list") == 0)
        system_output_mode = LIST;
      else
        fprintf(stderr, "Error: mode should be one of: csv list\n");
    }
    else
      print_system_cmd_syntax_error(s_cpy);
  }
  /* ".output". */
  else if (strcmp(cmd, "output") == 0) {
    if ((cmd = strtok(NULL, " \t")) != NULL) {
      if (strcmp(cmd, "stdout") == 0) { // ".mode stdout"
        if (sys_out_stream != stdout)
          fclose(sys_out_stream);
        sys_out_stream = stdout;
      }
      else
        sys_out_stream = fopen(cmd, "a");
    }
    else
      print_system_cmd_syntax_error(s_cpy);
  }
  /* ".tables". */
  else if (strcmp(cmd, "tables") == 0)
    if ((cmd = strtok(NULL, " \t")) == NULL)
      print_all_table_names();
    else {
      while (cmd != NULL) {
        print_single_table_name_by_name(cmd);
        cmd = strtok(NULL, " \t");
      }
    }
  /* UNKNOWN. */
  else
    print_system_cmd_syntax_error(s_cpy);

  return;
}

void
exec_SQL_cmd(char* cmd)
{
  char cmd_dup[INPUT_STR_SIZE];
  strcpy(cmd_dup, cmd); // prevent "cmd" being destroyed by strtok()
  determine_cmd_mode(strtok(cmd_dup, " "));
  switch (cmd_mode) {
    case CREATE :
      create_table(cmd);
      break;
    case ALTER :
      alter_table(cmd);
      break;
    case TRUNCATE :
      truncate_table(cmd);
      break;
    case DROP :
      drop_table(cmd);
      break;
    case INSERT :
      insert_into_table(cmd);
      break;
    case UPDATE :
      update_table(cmd);
      break;
    case DELETE :
      delete_from_table(cmd);
      break;
    case SELECT :
      select_from_table(cmd);
      break;
    case UNSET :
      print_syntax_error(cmd);
      break;
    default :
      break;
  }

  return;
}

void
trim_spaces_separate_marks(char* s)
{
  char temp[strlen(s) + 1];
  strncpy(temp, s, (strlen(s) + 1));

  int i_temp = 0, i_s = 0; // indices for both strings
  char is_head = 1; // flag
  char has_space = 0; // flag
  char has_colon = 0; // flag
  char ch;

  /* Remove redundant spaces. */
  for (i_temp = 0; i_temp < (strlen(temp) + 1); ++i_temp) {
    switch (temp[i_temp]) {
      /* Whitespace or tab. */
      case ' ' :
      case '\t' :
        if (has_space == 0)
          has_space = 1;
        break;
      /* Intentionally separated marks. */
      case ',' :
      case '(' :
      case ')' :
      case ';' :
        is_head = 0;
        has_space = 0;
        has_colon = 1;
        s[i_s++] = ' ';
        s[i_s++] = temp[i_temp];
        break;
      /* Operators after "WHERE". */
      case '=' :
        is_head = 0;
        has_space = 0;
        s[i_s++] = ' ';
        s[i_s++] = temp[i_temp];
        break;
      case '>' :
        is_head = 0;
        has_space = 0;
        s[i_s++] = ' ';
        s[i_s++] = temp[i_temp];
        if (temp[i_temp+1] == '=') // ">="
          s[i_s++] = temp[++i_temp];
        break;
      case '<' :
        is_head = 0;
        has_space = 0;
        s[i_s++] = ' ';
        s[i_s++] = temp[i_temp];
        if ((temp[i_temp+1] == '=') || (temp[i_temp+1] == '>')) // "<=", "<>"
          s[i_s++] = temp[++i_temp];
        break;
      /* End of string, no space. */
      case '\0' :
        s[i_s++] = temp[i_temp];
        break;
      /* Character. */
      default :
        if (((has_space) || (has_colon)) && (is_head == 0))
          s[i_s++] = ' ';
        is_head = 0;
        has_space = 0;
        has_colon = 0;
        ch = temp[i_temp-1];
        if ((ch == '=') || (ch == '<') || (ch == '>')) // ' ' after operator
          s[i_s++] = ' ';
        s[i_s++] = temp[i_temp];
        break;
    }
  }

  return;
}

void
determine_cmd_mode(char* s)
{

  if (strcmp(s, "CREATE") == 0)
    cmd_mode = CREATE;
  else if (strcmp(s, "ALTER") == 0)
    cmd_mode = ALTER;
  else if (strcmp(s, "TRUNCATE") == 0)
    cmd_mode = TRUNCATE;
  else if (strcmp(s, "DROP") == 0)
    cmd_mode = DROP;
  else if (strcmp(s, "INSERT") == 0)
    cmd_mode = INSERT;
  else if (strcmp(s, "UPDATE") == 0)
    cmd_mode = UPDATE;
  else if (strcmp(s, "DELETE") == 0)
    cmd_mode = DELETE;
  else if (strcmp(s, "SELECT") == 0)
    cmd_mode = SELECT;
  else
    cmd_mode = UNSET;
    
  return;
}
