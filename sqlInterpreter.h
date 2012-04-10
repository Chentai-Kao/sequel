/*
 * sqlInterpreter.h -- Interpret the sql command
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

#ifndef _CMDINTERPRETER
#define _CMDINTERPRETER

#include "selfDefineVars.h"

/* SQL commands. */
void create_table(char* cmd);
void alter_table(char* cmd);
void truncate_table(char* cmd);
void drop_table(char* cmd);
void insert_into_table(char* cmd);
void update_table(char* cmd);
void delete_from_table(char* cmd);
void select_from_table(char* cmd);


/* Print, dump. */
void print_all_table_names();
void print_single_table_name_by_name(char* table_name);
void print_table(SQL_table_t* table);
void print_single_data_by_col_order(SQL_table_t* table, Data_row_t* data,
    int select_all, char* selected_col, int n_selected_field);
void print_cell(Data_row_t* data, int i_col, attr_data_type_t data_type);
void dump_all_tables();
// no error if table not found
void dump_single_table_by_name(char* table_name);
void dump_table_content(SQL_table_t* table);
void dump_create_statement(SQL_table_t* table);
void schema_all_tables();
void schema_single_table_by_name(char* table_name);


/* Error. */
void print_table_not_found_error(char* name);
void print_data_not_null_error(char* table_name, char* attribute_name);
void print_data_not_unique_error(char* attribute_name);
void print_data_str_size_exceed_error(char* attribute_name, int size);
void print_n_insert_mismatch_error(SQL_table_t* table,
    int n_insert_col, int n_given_value);


/* Set values. */
void change_SQL_table_name(SQL_table_t* table, char* name);
void set_primary_key(char* attr_name);
void set_unique(char* attr_name);
// return -1 if the field name is not found in table
int set_order_by_field_name(SQL_table_t* table, char* s, char insert_order,
    char* selected_col);
// return UNDEFINED_OPERATOR if 's' is not a valid operator
relation_operator_t set_relation_operator(char* s);
// copy an attribute from 'src' to 'dst'
void copy_single_attribute(Data_col_t* dst, Data_col_t* src);


/* Check, compare. */
// (for CHAR and VARCHAR), -1 if failed
int check_and_trim_quotation_mark(char* s);
// precheck before atoi(), -1 if contains non-digit
int check_numeric(char* s);
// return 0 if 's1' and 's2' matches 'relation', (s1 > s2, '>'), -1 if not match
int check_str_relation(char* s1, char* s2, relation_operator_t relation);
// return 0 if 'n1' and 'n2' matches 'relation', (n1 > n2, '>'), -1 if not match
int check_num_relation(double n1, double n2, relation_operator_t relation);
// return 0 if data->cell[i_col] and value conforms the relation, -1 if not
int check_data_by_col_relation(Data_row_t* data, int i_col,
    relation_operator_t relation_operator, char* value,
    attr_data_type_t data_type);
// compare the data according to the 'i_col' column. For any data type,
// return value: 1 if data1 > data2, 0 if =, -1 if < (the same as strcmp())
// NOTE: assume data not null
int compare_data_by_column(Data_row_t* data1, Data_row_t* data2,
    int i_col, attr_data_type_t data_type);


/* Insert, append. */
// do check data type, -1 if failed
int insert_value_to_data(Data_row_t* data, char* value, int index_col,
    attr_data_type_t data_type);
int append_data_to_table(SQL_table_t* table, Data_row_t* new_data);


/* Find, search. */
SQL_table_t* find_table_by_name(char* name);
int get_col_index_by_order(char insert_order, char* selected_col,char range);
int get_col_index_by_name(SQL_table_t* table, char* field_name);
// search from 'i_start_row', return the index of row found, -1 if not found
int get_row_index_by_col_index(SQL_table_t* table, int i_col, Data_row_t* data,
    int i_start_row);
Data_row_t* get_data_by_row_index(SQL_table_t* table, int row_index);


/* Create, construct and destruct. */
void SQL_table_constructor(SQL_table_t* table);
void SQL_table_destructor(SQL_table_t* table);
Data_row_t* create_empty_data(int n_cells);

#endif
