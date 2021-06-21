/*
 * Parse files into a database table.
 *
 * Source file(s) must exist and be \t tab-separated.
 * Destination database must not have a table named after the file.
 * Creates a new table with the correct number of all text attributes.
 * header_length is used to skip the header of the file.
 *
 * insert_tsv_file_into_database:
 * Insert a single file into the database.
 *
 * batch_insert_tsv_file_into_database:
 * Insert a multiple files into the database.
 * The input is a '|'-separated string, containing filename|header_length 
 * pairs, like "filename1.tsv|0|filename2.tsv|5"
 *
 * batch_drop_tables:
 * Remove multiple tables at once. The input is the same as for batch insert.
 */

#pragma once
#include "database.h"

// Skip the header and INSERT the rest into a new table named after the file
void insert_tsv_file_into_database(const std::string &source_tsv_file, Database &destination_db, int header_length);

// Insert the contents of each file into a table of the same name
void batch_insert_tsv_file_into_database(const std::string &tables, Database &destination_db);

// Drop listed tables from Database
void batch_drop_tables(const std::string &tables, Database &destination_db);
