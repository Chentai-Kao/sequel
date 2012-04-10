/*
 * sqlInterpreter.c -- Interpret the sql command
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
#include <assert.h>
#include <ctype.h>

#include "utils.h"
#include "sqlInterpreter.h"

void
create_table(char* cmd) {
  char* s;
  /* Parse table name. */
  strtok(cmd, " "); // "CREATE"
  strtok(NULL, " "); // "TABLE"
  s = strtok(NULL, " "); // table name
  if (find_table_by_name(s) != NULL) {
    fprintf(stderr, "Error: table %s already exists\n", s);
    return;
  }
  /* Create new table. */
  active_SQL_table = malloc(sizeof(SQL_table_t));
  all_SQL_table[n_SQL_table++] = active_SQL_table;
  SQL_table_constructor(active_SQL_table);
  change_SQL_table_name(active_SQL_table, s); // set the table name

  /* Parse each column. */
  strtok(NULL, " "); // "("
  int i_attr = -1;
  while ((s = strtok(NULL, " ")) != NULL) {
    /* Possible "PRIMARY" KEY. */
    if (strcmp(s, "PRIMARY") == 0) { // "PRIMARY" KEY
      strtok(NULL, " "); // PRIMARY "KEY"
      strtok(NULL, " "); // "("
      set_primary_key(strtok(NULL, " ")); // set attribute as primary key
      strtok(NULL, " "); // ")"
      s = strtok(NULL, " ");
      if (strcmp(s, ",") == 0) // "," if "PRIMARY KEY" is not the last line
        continue;
      else
        break;
    }
    /* Possible "UNIQUE". */
    else if (strcmp(s, "UNIQUE") == 0) { // "UNIQUE"
      strtok(NULL, " "); // "("
      set_unique(strtok(NULL, " ")); // set attribute as unique
      strtok(NULL, " "); // ")"
      s = strtok(NULL, " ");
      if (strcmp(s, ",") == 0) // "," if "UNIQUE" is not the last line
        continue;
      else
        break;
    }

    /* Attribute name. */
    ++i_attr;
    active_SQL_table->n_col++; // number of columns of the table
    strcpy(active_SQL_table->attribute[i_attr].name, s);

    /* Attribute data type. */
    s = strtok(NULL, " ");
    if (strcmp(s, "CHAR") == 0) {
      active_SQL_table->attribute[i_attr].data_type = CHAR;
      strtok(NULL, " "); // "("
      s = strtok(NULL, " "); // "N" of CHAR(N)
      active_SQL_table->attribute[i_attr].str_char_size = atoi(s);
      strtok(NULL, " "); // ")"
    }
    else if (strcmp(s, "VARCHAR") == 0) {
      active_SQL_table->attribute[i_attr].data_type = VARCHAR;
      strtok(NULL, " "); // "("
      s = strtok(NULL, " "); // "N" of VARCHAR(N)
      active_SQL_table->attribute[i_attr].str_varchar_size = atoi(s);
      strtok(NULL, " "); // ")"
    }
    else if (strcmp(s, "INT") == 0)
      active_SQL_table->attribute[i_attr].data_type = INT;
    else if (strcmp(s, "FLOAT") == 0)
      active_SQL_table->attribute[i_attr].data_type = FLOAT;
    else if (strcmp(s, "DOUBLE") == 0)
      active_SQL_table->attribute[i_attr].data_type = DOUBLE;

    /* NOT NULL. */
    s = strtok(NULL, " ");
    if (strcmp(s, "NOT") == 0) { // "NOT" NULL
      strtok(NULL, " "); // NOT "NULL"
      active_SQL_table->attribute[i_attr].is_not_null = 1;
      s = strtok(NULL, " ");
    }

    /* Ending ")" of "CREATE TABLE xxx ()". */
    if (strcmp(s, ")") == 0)
      break;
  }

  return;
}

void
alter_table(char* cmd) {
  return;
}

void
truncate_table(char* cmd) {
  return;
}

void
drop_table(char* cmd) {
  char* s;
  strtok(cmd, " "); // "DROP"
  strtok(NULL, " "); // "TABLE"
  s = strtok(NULL, " "); // table name

  /* Find the table. */
  SQL_table_t* table;
  if ((table = find_table_by_name(s)) == NULL) {
    print_table_not_found_error(s);
    return;
  }

  /* Drop the table. */
  int i, j;
  for (i = 0; i < n_SQL_table; ++i) {
    if (all_SQL_table[i]->name == table->name) {
      for (j = i; j < (n_SQL_table - 1); ++j)
        all_SQL_table[j] = all_SQL_table[j + 1];
      all_SQL_table[j] = NULL;
    }
  }
  --n_SQL_table;
  SQL_table_destructor(table);

  return;
}

void
insert_into_table(char* cmd) {
  char* s;
  strtok(cmd, " "); // "INSERT"
  strtok(NULL, " "); // "INTO"
  s = strtok(NULL, " "); // table name

  /* Find the table. */
  SQL_table_t* temp_table;
  if ((temp_table = find_table_by_name(s)) == NULL) {
    print_table_not_found_error(s);
    return;
  }
  active_SQL_table = temp_table;

  /* Parse name of each column.
   * The order of fields to insert is stored in 'selected_col[n_col]',
   * 0 if not selected, 1 means the 1st column to insert, ...
   */
  char selected_col[active_SQL_table->n_col];
  memset(selected_col, 0, (active_SQL_table->n_col * sizeof(char)));
  char insert_all_col = 0; // 1 if "INSERT INTO table VALUES(...)"
  int n_insert_col = 0; // count # of cols for values to be given
  s = strtok(NULL, " "); // could be "(", or "VALUES" given all fields
  if (strcmp(s, "(") == 0) { // "("
    insert_all_col = 0;
    while ((s = strtok(NULL, " ")) != NULL) { // supposed to be "field_value"
      if (strcmp(s, ")") == 0) // "INSERT INTO mytable ()", no field selected
        return;

      ++n_insert_col;
      if (set_order_by_field_name(active_SQL_table, s, n_insert_col,
          selected_col) == -1) {
        print_syntax_error(s);
        return;
      }

      s = strtok(NULL, " "); // "," or the ending ")"
      if (strcmp(s, ")") == 0)
        break;
    }
  }
  else { // when "INSERT INTO table VALUES(...)", set 'selected_col' as 1, 2,...
    insert_all_col = 1;
    n_insert_col = active_SQL_table->n_col;
    int i;
    for (i = 0; i < active_SQL_table->n_col; ++i)
      selected_col[i] = i + 1;
  }

  /* Parse values for each column, save those values to a new data.
   * Use 'selected_col' to specify column.
   */
  if (insert_all_col == 0) // get "VALUES" if "INSERT INTO XX (XX) VALUES"
    strtok(NULL, " "); // "VALUES"
  strtok(NULL, " "); // "("
  int n_given_value = 0; // count # of values given
  Data_row_t* new_data = create_empty_data(active_SQL_table->n_col);
  while ((s = strtok(NULL, " ")) != NULL) { // supposed to be "field_value"
    if (strcmp(s, ")") == 0) // "VALUES ()", no value
      return;

    ++n_given_value;
    int insert_col = get_col_index_by_order(
        n_given_value, selected_col, active_SQL_table->n_col);
    if (insert_col == -1) { // when the field name is not found
      print_syntax_error(s);
      return;
    }
    if (insert_value_to_data(new_data, s, insert_col, // insert value to data
        active_SQL_table->attribute[insert_col].data_type) == -1) {
      free(new_data);
      print_syntax_error(s);
      return;
    }

    s = strtok(NULL, " "); // "," or the ending ")"
    if (strcmp(s, ")") == 0)
      break;
  }
  
  if (n_insert_col != n_given_value) {
    print_n_insert_mismatch_error(active_SQL_table,
        n_insert_col, n_given_value);
    free(new_data);
    return;
  }

  /* Append the new data to the table. */
  if (append_data_to_table(active_SQL_table, new_data) == -1) {
    print_syntax_error(s);
    free(new_data);
    return;
  }

  return;
}

void
update_table(char* cmd) {
  return;
}

void
delete_from_table(char* cmd) {
  return;
}

void
select_from_table(char* cmd) {
  char* s;
  strtok(cmd, " "); // "SELECT"
  
  char selected_field_names[NUM_ATTR_SIZE][ATTR_NAME_SIZE];
  int i;
  for (i = 0; i < NUM_ATTR_SIZE; ++i) // reset as null string
    selected_field_names[i][0] = '\0';

  int n_selected_field = 0;
  char select_all_fields = 0; // a flag, 1 when "SELECT * FROM XXtable"
  while ((s = strtok(NULL, " ")) != NULL) { // supposed to be "field_value"
    if (strcmp(s, "FROM") == 0) // syntax error, no field name specified
      return;
    else if (strcmp(s, "*") == 0) {
      select_all_fields = 1;
      strtok(NULL, " "); // "FROM"
      break;
    }
    else
      strcpy(selected_field_names[n_selected_field++], s);

    if (n_selected_field > NUM_ATTR_SIZE) {
      print_syntax_error(s);
      return;
    }

    s = strtok(NULL, " "); // "," or the ending "FROM"
    if (strcmp(s, "FROM") == 0)
      break;
  }

  s = strtok(NULL, " "); // table name
  /* Find the table. */
  SQL_table_t* temp_table;
  if ((temp_table = find_table_by_name(s)) == NULL) {
    print_table_not_found_error(s);
    return;
  }
  active_SQL_table = temp_table;

  /* Set the order of selected columns, start from 1. */
  char selected_col[active_SQL_table->n_col];
  memset(selected_col, 0, (active_SQL_table->n_col * sizeof(char)));
  for (i = 0; i < n_selected_field; ++i) {
    if (set_order_by_field_name(active_SQL_table, selected_field_names[i],
        i+1, selected_col) == -1) {
      print_syntax_error(selected_field_names[i]);
      return;
    }
  }

  /* Optional "WHERE" filter. */
  s = strtok(NULL, " ");
  char* field_name;
  int i_col;
  relation_operator_t relation_operator;
  char* value;
  int has_filter = 0; // flag
  if (strcmp(s, "WHERE") == 0) {
    has_filter = 1;
    field_name = strtok(NULL, " "); // field name
    s = strtok(NULL, " "); // operator
    if ((relation_operator = set_relation_operator(s)) == UNDEFINED_OPERATOR) {
      print_syntax_error(s);
      return;
    }
    value = strtok(NULL, " "); // value

    if ((i_col = get_col_index_by_name(active_SQL_table, field_name)) == -1) {
      print_syntax_error(field_name);
      return;
    }
  }

  /* Trim 'value' with quotation mark if needed. */
  attr_data_type_t data_type = active_SQL_table->attribute[i_col].data_type;
  if ((data_type == CHAR) || (data_type == VARCHAR)) {
    if (check_and_trim_quotation_mark(value) == -1) {
      print_syntax_error(field_name);
      return;
    }
  }

  /* Print result. */
  Data_row_t* data = active_SQL_table->next_data;
  while (data != NULL) {
    if (has_filter == 0) {
      print_single_data_by_col_order(active_SQL_table, data,
          select_all_fields, selected_col, n_selected_field);
    }
    else {
      if (check_data_by_col_relation(data, i_col, relation_operator, value,
          data_type) == 0) {
        print_single_data_by_col_order(active_SQL_table, data,
            select_all_fields, selected_col, n_selected_field);
      }
    }
    data = data->next_data;
  }

  return;
}

void
change_SQL_table_name(SQL_table_t* table, char* name)
{
  strcpy(table->name, name);
  return;
}

void
SQL_table_constructor(SQL_table_t* table)
{
  /* Table itself. */
  strcpy(table->name, "");
  table->n_row = 0;
  table->n_col = 0;

  /* Attributes. */
  int i;
  for (i = 0; i < NUM_ATTR_SIZE; ++i) {
    strcpy(table->attribute[i].name, "");
    table->attribute[i].data_type = VOID;
    table->attribute[i].str_char_size = 0;
    table->attribute[i].str_varchar_size = 0;
    table->attribute[i].is_unique = 0;
    table->attribute[i].is_not_null = 0;
    table->attribute[i].is_primary_key = 0;
    table->attribute[i].is_check = 0;
    table->attribute[i].is_default = 0;
  }

  /* Data. */
  table->next_data = NULL;

  return;
}

void
set_primary_key(char* attr_name)
{
  int i;
  for (i = 0; i < active_SQL_table->n_col; ++i) {
    if (strcmp(active_SQL_table->attribute[i].name, attr_name) == 0) {
      active_SQL_table->attribute[i].is_unique = 1;
      active_SQL_table->attribute[i].is_primary_key = 1;
      active_SQL_table->attribute[i].is_not_null = 1;
      break;
    }
  }

  return;
}

void
print_all_table_names()
{
  int i;
  for (i = 0; i < n_SQL_table; ++i)
    fprintf(sys_out_stream, "%s\n", all_SQL_table[i]->name);
  return;
}

void
print_single_table_name_by_name(char* table_name)
{
  SQL_table_t* table = find_table_by_name(table_name);
  if (table != NULL)
    fprintf(sys_out_stream, "%s\n", table->name);
  return;
}

void
print_table_not_found_error(char* name)
{
  fprintf(stderr, "Error: no such table: %s\n", name);
  return;
}

SQL_table_t*
find_table_by_name(char* name)
{
  /* Trim quotation mark, if any. */
  int len = strlen(name);
  if ((name[0] == '\"') && (name[len - 1] == '\"')) {
    memmove(name, name+1, len-2);
    name[len-2] = '\0';
  }
  /* Find table. */
  int i;
  for (i = 0; i < n_SQL_table; ++i) {
    if (strcmp(all_SQL_table[i]->name, name) == 0)
      return all_SQL_table[i];
  }

  return NULL;
}

int
set_order_by_field_name(SQL_table_t* table, char* s, char insert_order,
    char* selected_col)
{
  int i_col;
  if ((i_col = get_col_index_by_name(table, s)) != -1) {
    selected_col[i_col] = insert_order;
    return 0;
  }

  return -1;
}

int
get_col_index_by_order(char insert_order, char* selected_col, char range)
{
  int i;
  for (i = 0; i < range; ++i) {
    if (selected_col[i] == insert_order)
      return i;
  }

  return -1;
}

Data_row_t*
create_empty_data(int n_cells)
{
  assert(n_cells >= 0);

  Data_row_t* new_data = malloc(sizeof(Data_row_t));
  new_data->cell = malloc(n_cells * sizeof(SQL_cell_t));
  int i;
  for (i = 0; i < n_cells; ++i) { // set each cell as "(null)"
    new_data->cell[i].is_null = 1;
    new_data->cell[i].str_char = NULL;
    new_data->cell[i].str_varchar = NULL;
  }
  new_data->next_data = NULL;

  return new_data;
}

int
insert_value_to_data(Data_row_t* data, char* value, int index_col,
    attr_data_type_t data_type)
{
  if (strcmp(value, "NULL") == 0) { // if input "NULL", valid
    data->cell[index_col].is_null = 1;
    return 0;
  }

  data->cell[index_col].is_null = 0;

  /* Data type verification. */
  if ((data_type == CHAR) || (data_type == VARCHAR)) {
    if (check_and_trim_quotation_mark(value) == -1)
      return -1;
  }
  else if ((data_type == INT) || (data_type == FLOAT) || (data_type == DOUBLE)){
    if (check_numeric(value) == -1)
      return -1;
  }

  /* Insert value. */
  switch (data_type) {
    case CHAR :
      data->cell[index_col].str_char = malloc(STR_CHAR_SIZE * sizeof(char));
      strcpy(data->cell[index_col].str_char, value);
      break;
    case VARCHAR :
      data->cell[index_col].str_varchar = malloc(STR_VARCHAR_SIZE*sizeof(char));
      strcpy(data->cell[index_col].str_varchar, value);
      break;
    case INT :
      data->cell[index_col].num_int = atoi(value);
      break;
    case FLOAT :
      data->cell[index_col].num_float = (float)atof(value);
      break;
    case DOUBLE :
      data->cell[index_col].num_double = atof(value);
      break;
    default :
      break;
  }

  return 0;
}

int
check_and_trim_quotation_mark(char* s)
{
  int len = strlen(s);
  if ((s[0] != '\'') || (s[len - 1] != '\''))
    return -1;

  /* Trim quotation mark. */
  memmove(s, s+1, len-2);
  s[len-2] = '\0';

  return 0;
}

int
check_numeric(char* s)
{
  /* First digit, can be digit, '.', '+', '-' */
  if ((isdigit(s[0]) == 0) && (s[0] != '.') && (s[0] != '-') && (s[0] != '+'))
    return -1;

  int has_point = 0; // can only has one point, e.g. "1.23" or ".23"
  if (s[0] == '.')
    has_point = 1;

  /* Other digits, can be digit, '.' */
  int i;
  for (i = 1; i < strlen(s); ++i) {
    if (isdigit(s[i]) == 0) {
      if (s[i] == '.') {
        if (has_point == 1) // multiple points, e.g. "2.53.56"
          return -1;
        else
          has_point = 1;
      }
      else
        return -1;
    }
  }

  return 0;
}

int
append_data_to_table(SQL_table_t* table, Data_row_t* new_data)
{
  int i;
  for (i = 0; i < table->n_col; ++i) {
    /* Check NOT NULL. */
    if (table->attribute[i].is_not_null == 1) {
      if (new_data->cell[i].is_null == 1) {
        print_data_not_null_error(table->name, table->attribute[i].name);
        return -1;
      }
    }
    /* Check UNIQUE. */
    if (table->attribute[i].is_unique == 1) {
      if (get_row_index_by_col_index(table, i, new_data, 0) != -1) {
        print_data_not_unique_error(table->attribute[i].name);
        return -1;
      }
    }
    /* Check CHAR VARCHAR size, if any. */
    if (table->attribute[i].data_type == CHAR) {
      if ((new_data->cell[i].is_null == 0) &&
          (strlen(new_data->cell[i].str_char) >
          table->attribute[i].str_char_size)) {
        print_data_str_size_exceed_error(table->attribute[i].name,
            table->attribute[i].str_char_size);
        return -1;
      }
    }
    if (table->attribute[i].data_type == VARCHAR) {
      if ((new_data->cell[i].is_null == 0) &&
          (strlen(new_data->cell[i].str_varchar) >
          table->attribute[i].str_varchar_size)) {
        print_data_str_size_exceed_error(table->attribute[i].name,
            table->attribute[i].str_varchar_size);
        return -1;
      }
    }
  }

  /* Append. */
  Data_row_t** insert_place = &(table->next_data);
  if (table->n_row > 0) {
    for (i = 0; i < table->n_row; ++i)
      insert_place = &((*insert_place)->next_data);
  }
  (*insert_place) = new_data;
  (table->n_row)++;

  return 0;
}

int
get_row_index_by_col_index(SQL_table_t* table, int i_col, Data_row_t* data,
    int i_start_row)
{
  if (i_start_row >= table->n_row)
    return -1;

  /* Search according to its data type. */
  int i_data = i_start_row;
  Data_row_t* ref_data;
  if ((ref_data = get_data_by_row_index(table, i_start_row)) == NULL)
    return -1;

  /* Search for data. */
  if (data->cell[i_col].is_null == 1) { // for null data
    /* According to ANSI standaard SQL:2003, UNIQUE allow multiple NULL
     * while (ref_data != NULL) {
     *   if (ref_data->cell[i_col].is_null == 1)
     *     return i_data;
     *   ref_data = ref_data->next_data;
     *   ++i_data;
     * }
     */
    return -1;
  }
  else { // for not null data
    while (ref_data != NULL) {
      if ((ref_data->cell[i_col].is_null == 0) && (compare_data_by_column(
          ref_data, data, i_col, table->attribute[i_col].data_type) == 0))
        return i_data;
      ref_data = ref_data->next_data;
      ++i_data;
    }
  }

  return -1;
}

int
compare_data_by_column(Data_row_t* data1, Data_row_t* data2,
    int i_col, attr_data_type_t data_type)
{
  assert(data1->cell[i_col].is_null == 0);
  assert(data2->cell[i_col].is_null == 0);

  switch (data_type) {
    case CHAR :
      return strcmp(data1->cell[i_col].str_char,
          data2->cell[i_col].str_char);
      break;
    case VARCHAR :
      return strcmp(data1->cell[i_col].str_varchar,
          data2->cell[i_col].str_varchar);
      break;
    case INT :
      if (data1->cell[i_col].num_int == data2->cell[i_col].num_int)
        return 0;
      else if (data1->cell[i_col].num_int > data2->cell[i_col].num_int)
        return 1;
      else
        return -1;
      break;
    case FLOAT :
      if (data1->cell[i_col].num_float == data2->cell[i_col].num_float)
        return 0;
      else if (data1->cell[i_col].num_float > data2->cell[i_col].num_float)
        return 1;
      else
        return -1;
      break;
    case DOUBLE :
      if (data1->cell[i_col].num_double == data2->cell[i_col].num_double)
        return 0;
      else if (data1->cell[i_col].num_double > data2->cell[i_col].num_double)
        return 1;
      else
        return -1;
      break;
    default :
      break;
  }

  return -1;
}

void
print_table(SQL_table_t* table)
{
  assert(table != NULL);
  if (table->name != NULL)
    fprintf(sys_out_stream, "%s\n", table->name);
  fprintf(sys_out_stream, "----------\n");
  Data_row_t* data = table->next_data;
  while (data != NULL) {
    print_single_data_by_col_order(table, data, 1, NULL, table->n_col);
    data = data->next_data;
  }

  return;
}

void
set_unique(char* attr_name)
{
  int i;
  for (i = 0; i < active_SQL_table->n_col; ++i) {
    if (strcmp(active_SQL_table->attribute[i].name, attr_name) == 0) {
      active_SQL_table->attribute[i].is_unique = 1;
      break;
    }
  }

  return;
}



relation_operator_t
set_relation_operator(char* s)
{
  relation_operator_t ret = UNDEFINED_OPERATOR;
  if (strcmp(s, "=") == 0)
    ret = EQUAL;
  else if (strcmp(s, "<>") == 0)
    ret = NOT_EQUAL;
  else if (strcmp(s, ">") == 0)
    ret = GREATER;
  else if (strcmp(s, "<") == 0)
    ret = LESS;
  else if (strcmp(s, ">=") == 0)
    ret = GREATER_EQUAL;
  else if (strcmp(s, "<=") == 0)
    ret = LESS_EQUAL;

  return ret;
}

Data_row_t*
get_data_by_row_index(SQL_table_t* table, int row_index)
{
  assert(table != NULL);
  if (row_index >= table->n_row)
    return NULL;

  Data_row_t* data = table->next_data;
  int i;
  for (i = 0; i < row_index; ++i)
    data = data->next_data;

  return data;
}

int
check_str_relation(char* s1, char* s2, relation_operator_t relation)
{
  assert((s1 != NULL) && (s2 != NULL));
  switch (relation) {
    case EQUAL :
      if (strcmp(s1, s2) == 0)
        return 0;
      break;
    case NOT_EQUAL :
      if (strcmp(s1, s2) != 0)
        return 0;
      break;
    case GREATER :
      if (strcmp(s1, s2) > 0)
        return 0;
      break;
    case LESS :
      if (strcmp(s1, s2) < 0)
        return 0;
      break;
    case GREATER_EQUAL :
      if (strcmp(s1, s2) >= 0)
        return 0;
      break;
    case LESS_EQUAL :
      if (strcmp(s1, s2) <= 0)
        return 0;
      break;
    default :
      break;
  }

  return -1;
}

int
check_num_relation(double n1, double n2, relation_operator_t relation)
{
  switch (relation) {
    case EQUAL :
      if (n1 == n2)
        return 0;
      break;
    case NOT_EQUAL :
      if (n1 != n2)
        return 0;
      break;
    case GREATER :
      if (n1 > n2)
        return 0;
      break;
    case LESS :
      if (n1 < n2)
        return 0;
      break;
    case GREATER_EQUAL :
      if (n1 >= n2)
        return 0;
      break;
    case LESS_EQUAL :
      if (n1 <= n2)
        return 0;
      break;
    default :
      break;
  }

  return -1;
}

void
SQL_table_destructor(SQL_table_t* table)
{
  if (table == NULL)
    return;

  int i;
  Data_row_t* data = table->next_data; // current data
  Data_row_t* temp_data; // hold the next_data of the current data
  while (data != NULL) {
    temp_data = data->next_data;
    if (data->cell != NULL) {
      for (i = 0; i < table->n_col; ++i) {
        if (data->cell[i].str_char != NULL)
          free(data->cell[i].str_char);
        if (data->cell[i].str_varchar != NULL)
          free(data->cell[i].str_varchar);
      }
    }
    free(data->cell);
    free(data);
    data = temp_data;
  }
  free(table);

  return;
}

void
copy_single_attribute(Data_col_t* dst, Data_col_t* src)
{
  assert(dst != NULL);
  assert(src != NULL);

  strcpy(dst->name, src->name);
  dst->data_type        = src->data_type;
  dst->str_char_size    = src->str_char_size;
  dst->str_varchar_size = src->str_varchar_size;
  dst->is_unique        = src->is_unique;
  dst->is_not_null      = src->is_not_null;
  dst->is_primary_key   = src->is_primary_key;
  dst->is_check         = src->is_check;
  dst->is_default       = src->is_default;

  return;
}

int
get_col_index_by_name(SQL_table_t* table, char* field_name)
{
  assert((table != NULL) && (field_name != NULL));
  int i;
  for (i = 0; i < table->n_col; ++i) {
    if (strcmp(table->attribute[i].name, field_name) == 0)
      return i;
  }

  return -1;
}

int
check_data_by_col_relation(Data_row_t* data, int i_col,
    relation_operator_t relation_operator, char* value,
    attr_data_type_t data_type)
{
  assert((data != NULL) && (value != NULL));
  char* s;
  int nn;
  float ff;
  double dd;
  if (data->cell[i_col].is_null != 1) {
    switch (data_type) {
      case CHAR :
        s = data->cell[i_col].str_char;
        return check_str_relation(s, value, relation_operator);
        break;
      case VARCHAR :
        s = data->cell[i_col].str_varchar;
        return check_str_relation(s, value, relation_operator);
        break;
      case INT :
        nn = data->cell[i_col].num_int;
        return check_num_relation(nn, atoi(value), relation_operator);
        break;
      case FLOAT :
        ff = data->cell[i_col].num_float;
        return check_num_relation(ff, atof(value), relation_operator);
        break;
      case DOUBLE :
        dd = data->cell[i_col].num_double;
        return check_num_relation(dd, atof(value), relation_operator);
        break;
      default :
        break;
    }
  }

  return -1;
}

void
print_single_data_by_col_order(SQL_table_t* table, Data_row_t* data,
    int select_all, char* selected_col, int n_selected_field)
{
  if (select_all == 1)
    n_selected_field = table->n_col;

  int i, i_col;
  for (i = 0; i < n_selected_field; ++i) {
    if (select_all == 1)
      i_col = i;
    else
      i_col = get_col_index_by_order(i+1, selected_col, table->n_col);

    print_cell(data, i_col, table->attribute[i_col].data_type);
    /* Seperator if needed. */
    if (i < (n_selected_field - 1)) {
      switch (system_output_mode) {
        case (CSV) :
          fprintf(sys_out_stream, ",");
          break;
        case (LIST) :
          fprintf(sys_out_stream, "|");
          break;
        default :
          break;
      }
    }
  }
  fprintf(sys_out_stream, "\n");

  return;
}

void
print_data_not_null_error(char* table_name, char* attribute_name)
{
  fprintf(stderr, "Error: %s.%s may not be NULL\n", table_name, attribute_name);
  return;
}

void
print_data_not_unique_error(char* attribute_name)
{
  fprintf(stderr, "Error: column %s is not unique\n", attribute_name);
  return;
}

void
print_data_str_size_exceed_error(char* attribute_name, int size)
{
  fprintf(stderr, "Error: column %s has size greater than %d\n",
      attribute_name, size);
  return;
}

void
print_cell(Data_row_t* data, int i_col, attr_data_type_t data_type)
{
  assert(data != NULL);
  if (data->cell[i_col].is_null == 0) {
    switch (data_type) {
      case CHAR :
        fprintf(sys_out_stream, "%s", data->cell[i_col].str_char);
        break;
      case VARCHAR :
        fprintf(sys_out_stream, "%s", data->cell[i_col].str_varchar);
        break;
      case INT :
        fprintf(sys_out_stream, "%d", data->cell[i_col].num_int);
        break;
      case FLOAT :
        fprintf(sys_out_stream, "%f", data->cell[i_col].num_float);
        break;
      case DOUBLE :
        fprintf(sys_out_stream, "%f", data->cell[i_col].num_double);
        break;
      default :
        break;
    }
  }
}

void
dump_all_tables()
{
  int i;
  for (i = 0; i < n_SQL_table; ++i)
    dump_table_content(all_SQL_table[i]);

  return;
}

void
dump_single_table_by_name(char* table_name)
{
  SQL_table_t* table = find_table_by_name(table_name);
  dump_table_content(table);

  return;
}

void
dump_table_content(SQL_table_t* table)
{
  if (table == NULL)
    return;

  /* First line. */
  dump_create_statement(table);

  /* Data rows. */
  int i;
  Data_row_t* data = table->next_data;
  while (data != NULL) {
    fprintf(sys_out_stream, "INSERT INTO \"%s\" VALUES(", table->name);
    for (i = 0; i < table->n_col; ++i) {
      if (i > 0)
        fprintf(sys_out_stream, ",");
      if (data->cell[i].is_null == 1) // is NULL
        fprintf(sys_out_stream, "NULL");
      else { // is not NULL
        switch (table->attribute[i].data_type) {
          case CHAR :
            fprintf(sys_out_stream, "\'%s\'", data->cell[i].str_char);
            break;
          case VARCHAR :
            fprintf(sys_out_stream, "\'%s\'", data->cell[i].str_varchar);
            break;
          case INT :
            fprintf(sys_out_stream, "%d", data->cell[i].num_int);
            break;
          case FLOAT :
            fprintf(sys_out_stream, "%f", data->cell[i].num_float);
            break;
          case DOUBLE :
            fprintf(sys_out_stream, "%f", data->cell[i].num_double);
            break;
          default :
            break;
        }
      }
    }
    fprintf(sys_out_stream, ");\n");
    data = data->next_data;
  }

  return;
}

void
print_n_insert_mismatch_error(SQL_table_t* table,
    int n_insert_col, int n_given_value)
{
  assert(table != NULL);
  if (n_insert_col == table->n_col) {
    fprintf(stderr,
        "Error: table %s has %d columns but %d values were supplied\n",
        table->name, table->n_col, n_given_value);
  }
  else {
    fprintf(stderr, "Error: %d values for %d columns\n",
        n_given_value, n_insert_col);
  }
  return;
}

void
dump_create_statement(SQL_table_t* table)
{
  if (table == NULL)
    return;

  fprintf(sys_out_stream, "CREATE TABLE %s (", table->name); // table name
  int i;
  for (i = 0; i < table->n_col; ++i) {
    if (i > 0)
      fprintf(sys_out_stream, ", ");
    fprintf(sys_out_stream, "%s ", table->attribute[i].name); // col name
    switch (table->attribute[i].data_type) { // col data type
      case CHAR :
        fprintf(sys_out_stream, "CHAR(%d)",
            table->attribute[i].str_char_size);
        break;
      case VARCHAR :
        fprintf(sys_out_stream, "VARCHAR(%d)",
            table->attribute[i].str_varchar_size);
        break;
      case INT :
        fprintf(sys_out_stream, "INT");
        break;
      case FLOAT :
        fprintf(sys_out_stream, "FLOAT");
        break;
      case DOUBLE :
        fprintf(sys_out_stream, "DOUBLE");
        break;
      default :
        break;
    }
    if (table->attribute[i].is_not_null == 1)
      fprintf(sys_out_stream, " NOT NULL");
  }
  for (i = 0; i < table->n_col; ++i) { // PRIMARY KEY
    if (table->attribute[i].is_primary_key == 1) {
      fprintf(sys_out_stream, ", PRIMARY KEY (%s)", table->attribute[i].name);
      break;
    }
  }
  char has_unique = 0; // UNIQUE (won't print if it is also PRIMARY KEY)
  char has_unique_col = 0;
  for (i = 0; i < table->n_col; ++i) {
    if ((table->attribute[i].is_primary_key == 0) &&
        (table->attribute[i].is_unique == 1)) {
      if (has_unique == 0)
        fprintf(sys_out_stream, ", UNIQUE (");
      has_unique = 1;
      if (has_unique_col == 1)
        fprintf(sys_out_stream, ", ");
      has_unique_col = 1;
      fprintf(sys_out_stream, "%s", table->attribute[i].name);
    }
  }
  if (has_unique == 1)
    fprintf(sys_out_stream, ")"); // end of UNIQUE
  fprintf(sys_out_stream, ");\n"); // end of the first line

  return;
}

void
schema_all_tables()
{
  int i;
  for (i = 0; i < n_SQL_table; ++i)
    dump_create_statement(all_SQL_table[i]);
  return;
}

void
schema_single_table_by_name(char* table_name)
{
  SQL_table_t* table = find_table_by_name(table_name);
  dump_create_statement(table);
  return;
}
