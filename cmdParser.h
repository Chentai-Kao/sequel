/*
 * cmdParser.h -- Parsing the input command.
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

#ifndef _CMDPARSER
#define _CMDPARSER

#include "selfDefineVars.h"

/* Remove the trailing '\n', which is appended by fgets(), but not gets(). */
void trim_trailing_endline(char* s);

/* Parse and execute the user input as system command. */
void exec_system_cmd(char* s);

/* Parse and execute the user input as SQL command. */
void exec_SQL_cmd(char* cmd);

/* Trim all spaces and tabs to be one space. No starting spaces,
 * trailing colon is separated by extra space to be one token. */
void trim_spaces_separate_marks(char* s);

/* Determine the command mode according to the string 's', 1st work of cmd. */
void determine_cmd_mode(char* s);

#endif
