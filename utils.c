/*
 * utils.c -- Some useful tools.
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
#include <string.h>

#include "utils.h"

void
print_info()
{
  puts("sequel version 0.0.1 2012-03-25");
  puts("Enter \".help\" for instructions");
  puts("Enter SQL statments terminated with a \";\"");
}

void
print_prompt(enum prompt_choice_t p)
{
  if (p == TEXT_PROMPT)
    fprintf(stdout, "sequel> ");
  else
    fprintf(stdout, "   ...> ");
  return;
}

void
print_help()
{
  puts(".dump ?TABLE? ...      Dump the database in an SQL text format");
  puts("                         If TABLE specified, only dump tables matching"
      );
  puts("                         LIKE pattern TABLE.");
  puts(".echo ON|OFF           Turn command echo on or off");
  puts(".exit                  Exit this program");
  puts(".help                  Show this message");
  puts(".quit                  Exit this program");
  puts(".read FILENAME         Execute SQL in FILENAME");
  puts(".schema ?TABLE?        Show the CREATE statements");
  puts("                         If TABLE specified, only show tables matching"
      );
  puts("                         LIKE pattern TABLE.");
  puts(".mode MODE             Set output mode where MODE is one of:");
  puts("                         csv      Comma-separated values");
  puts("                         list     Values delimited by .separator string"
      );
  puts(".output FILENAME       Send output to FILENAME");
  puts(".output stdout         Send output to the screen");

  puts(".tables                List names of tables");
  return;
}

void
print_syntax_error(char* s)
{
  fprintf(stderr, "Error: near \"%s\": syntax error\n", s);
  return;
}

int
check_trailing_colon(char* s)
{
  int i;
  for (i = (strlen(s) - 1); i >= 0; --i) {
    if ((s[i] == ' ') || (s[i] == '\t'))
      continue;
    else if (s[i] == ';')
      return 1;
    else
      break;
  }
  return 0;
}

int
is_system_cmd(char* s)
{
  if (s[0] == '.')
    return 1;

  return 0;
}

void
print_system_cmd_syntax_error(char* s)
{
  fprintf(stderr, "Error: unknown command or invalid arguments:  \"%s\". "
      "Enter \".help\" for help\n", s);
  return;
}

void
clear_backward(int num)
{
  int i;
  for (i = 0; i < num; ++i)
    fprintf(stdout, "\b \b");
  return;
}
