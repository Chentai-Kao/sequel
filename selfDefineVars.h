/*
 * selfDefineVars.h -- Some prototypes of variables.
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

#ifndef _SELFDEFINEVARS
#define _SELFDEFINEVARS

/* Maximum size of the input string. */
#ifndef INPUT_STR_SIZE
#define INPUT_STR_SIZE 1024
#endif

/* Prompt display. */
#ifndef PROMPT_SIZE
#define PROMPT_SIZE 8
#endif

/* Limit of the number of SQL table. */
#ifndef NUM_SQL_TABLE
#define NUM_SQL_TABLE 10
#endif

/* Limit of the number of attributes (SQL table). Must smaller than 128. */
#ifndef NUM_ATTR_SIZE
#define NUM_ATTR_SIZE 20
#endif

/* SQL table name. */
#ifndef TABLE_NAME_SIZE
#define TABLE_NAME_SIZE 20
#endif

/* (SQL table) attribute name. */
#ifndef ATTR_NAME_SIZE
#define ATTR_NAME_SIZE 20
#endif

/* (SQL table) string of data type. */
#ifndef DATA_TYPE_SIZE
#define DATA_TYPE_SIZE 20
#endif

/* String size limit of data type CHAR. */
#ifndef STR_CHAR_SIZE
#define STR_CHAR_SIZE 1024
#endif

/* String size limit of data type VARCHAR. */
#ifndef STR_VARCHAR_SIZE
#define STR_VARCHAR_SIZE 1024
#endif

/* Prompt type, TEXT_PROMPT("sequel>") or DOT_PROMPT("...>"). */
enum prompt_choice_t {
  TEXT_PROMPT = 0,
  DOT_PROMPT
} prompt_choice;

/* System output mode. */
enum system_output_mode_t {
  CSV,
  LIST
} system_output_mode;

FILE* sys_out_stream; // stdout or ".output FILENAME"
FILE* sys_in_stream; // stdin or ".read FILENAME"
FILE* db_file; // ./sequel test.db

/* .echo on|off */
enum sys_echo_t {
  ON,
  OFF
} sys_echo;

/* Command mode.
 * Data definition: CREATE, ALTER, TRUNCATE, DROP.
 * Data manipulation: INSERT, UPDATE, DELETE.
 * Query: SELECT.
 * (dummy: UNSET)
 */
enum cmd_mode_t {
  CREATE,
  ALTER,
  TRUNCATE,
  DROP,
  INSERT,
  UPDATE,
  DELETE,
  SELECT,
  UNSET
} cmd_mode;

/* Data type of an attribute (SQL table). */
typedef enum {
  CHAR,
  VARCHAR,
  INT,
  FLOAT,
  DOUBLE,
  VOID
} attr_data_type_t;

/* Operators allowed in "WHERE" clause. */
typedef enum {
  EQUAL,
  NOT_EQUAL,
  GREATER,
  LESS,
  GREATER_EQUAL,
  LESS_EQUAL,
  UNDEFINED_OPERATOR
} relation_operator_t;

/* A cell in the SQL table. This contains all possible data types. */
typedef struct SQL_cell_tt SQL_cell_t;
struct SQL_cell_tt {
  /* Properties. */
  char is_null; // 1 if this cell is "(null)", 0 otherwise

  /* All possible data types. */
  char*  str_char;
  char*  str_varchar;
  int    num_int;
  float  num_float;
  double num_double;
};

/* The root node of a data (row). It contains pointers of cells and the
 * next data. For data containing multiple cells (columns),
 * construct the cell array, "cell = malloc(num_cells * sizeof(SQL_cell_t));"
 */
typedef struct Data_row_tt Data_row_t;
struct Data_row_tt {
  SQL_cell_t* cell;
  Data_row_t* next_data;
};

/* The head of an attribute (column). It contains the attribute's name, 
 * data type, and constraint.
 */
typedef struct Data_col_tt Data_col_t;
struct Data_col_tt {
  /* Name and data type. */
  char name[ATTR_NAME_SIZE];
  attr_data_type_t data_type; // data type of this attribute
  char str_char_size; // size of CHAR, ex: 15 for "CHAR(15)"
  char str_varchar_size; // size of VARCHAR, ex: 15 for "VARCHAR(15)"

  /* Constraints. */
  char is_unique; // 1 if UNIQUE, 0 otherwise
  char is_not_null; // 1 if NOT NULL, 0 otherwise
  char is_primary_key; // 1 if PRIMARY KEY, 0 otherwise
  char is_check; // (opt) 1 if CHECK (give range of data), 0 otherwise
  char is_default;//(opt) 1 if DEFAULT (give default value of data), 0 otherwise
};

/* SQL table. */
typedef struct SQL_table_tt SQL_table_t;
struct SQL_table_tt {
  char        name[TABLE_NAME_SIZE];
  int         n_row; // number of rows (data)
  int         n_col; // number of cols (cell)
  Data_col_t  attribute[NUM_ATTR_SIZE]; // head of each attribute (col)
  Data_row_t* next_data; // pointer to the first data (row)
};

/* Currently active SQL table. */
SQL_table_t* active_SQL_table;

/* All SQL tables. */
SQL_table_t* all_SQL_table[NUM_SQL_TABLE];
int n_SQL_table; // number of all SQL tables

#endif
