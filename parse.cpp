#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include<boost/tokenizer.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include "parse.h"
#include "database.h"

// sql prepared statement
std::string prepare(const std::string &sql_statement) {
	std::string prepared = sql_statement;
	boost::replace_all(prepared, "'", "''");
	return "'" + prepared + "'";
}

// skip the first few lines of an input stream
void skip_lines(std::istream &input, int number_of_lines) {
	std::string line;
	for (int i = 0; i < number_of_lines && getline(input, line); ++i) {}
}

// assemble CREATE statement with the correct number of TEXT attributes
std::string sql_create_statement(
	const std::string &source_file, 
	const std::string &destination_table, 
	int header_length) {
	// open file
	std::ifstream file_stream(source_file.c_str());
	
	// skip the header
	skip_lines(file_stream, header_length);
	
	// get the number of attributes from the first line
	std::string line;
	assert( getline(file_stream, line) ); // does file even exist???
	int number_of_attributes = std::count(line.begin(), line.end(), '\t') + 1;
	
	// attribute names are A1, A2, ...., An
	std::string create_statement = "CREATE TABLE " + prepare(destination_table) + " (";
	for (int i = 1; i <= number_of_attributes; ++i) {
		const std::string index = boost::lexical_cast<std::string>(i);
		create_statement += "A" + index + " TEXT NOT NULL, ";
	}
	// correct the ending of the statement
	create_statement.replace(create_statement.size() - 2, 2, ");");
	
	return create_statement;
}

// line by line convert the file into a long list of INSERT statements
std::string sql_bulk_insert_statements(
	const std::string &source_file, 
	const std::string &destination_table, 
	int header_length) {
	// open file
	std::ifstream file_stream(source_file.c_str());
	
	// skip the header
	skip_lines(file_stream, header_length);
	
	const std::string prefix = "INSERT INTO " + prepare(destination_table) + " VALUES (";
	std::string insert_statements; // long list of insert statements
	
	// line by line bulk create all sql statements
	for (std::string line; getline(file_stream, line); ) {
		// protect from ' found in the text
		line = prepare(line);
		// wrap the rest around with ' ' guards
		boost::replace_all(line, "\t", "', '");
		// append new statement at the end of the list
		insert_statements += prefix + line + ");";
	}
	
	return insert_statements;
}

// Skip the header and INSERT the rest into a new table named after the file
// Also used as an embedded function for process_table_string, see below
void insert_tsv_file_into_database(
	const std::string &source_file, 
	Database &destination_db, 
	int header_length) {
	// trace message
	std::cout << "Reading " << source_file << std::endl;
	
	// table is named after the file
	const std::string destination_table = source_file;
	
	// bulk create all sql statements from the file
	const std::string create_statement = sql_create_statement(source_file, destination_table, header_length);
	//std::cout << create_statement << "\n";
	destination_db.execute(create_statement);
	
	// bulk create all sql statements from the file
	//std::cout << "Preparing INSERT statements\n";
	const std::string insert_statement = sql_bulk_insert_statements(source_file, destination_table, header_length);
	
	// execute sql INSERT statements all at once
	//std::cout << "Executing INSERT statements\n";
	destination_db.execute(insert_statement);
}

// Quick test displays the count and the first few inserted elements.
void test_insert_tsv_file_into_database(
	const std::string &source_file, 
	const std::string &destination_db_file, 
	int header_length) {
	Database db(destination_db_file);
	insert_tsv_file_into_database(source_file, db, header_length);
	db.execute("SELECT * from " + prepare(source_file) + " LIMIT 5;");
	db.execute("SELECT COUNT(*) from " + prepare(source_file) + ";");
	db.execute("DROP TABLE " + prepare(source_file) + ";");
}

// Helper function that breaks a '|'-separated input string to tokens,
// and executes given function f(string, Database, int) on each pair.
void process_table_string(const std::string &tables, Database &destination_db, void (*f)(const std::string&, Database&, int)) {
	// initialize boost tokenizer
	boost::char_separator<char> sep("|");
	typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer;
	t_tokenizer tok(tables, sep);
	
	// process tokens in pairs of two
	for (t_tokenizer::iterator it=tok.begin(); it != tok.end(); ++it) {
		const std::string source_file = *it;
		if (++it == tok.end()) {
			// missing token pair
			break;
		}
		int header_length = atoi((*it).c_str());
		
		(*f)(source_file, destination_db, header_length);
	}
}

// Insert the contents of each file into a table of the same name
void batch_insert_tsv_file_into_database(
	const std::string &tables, 
	Database &destination_db) {
	process_table_string(tables, destination_db, 
		insert_tsv_file_into_database);
}

// Embedded function for process_table_string, see above
void sql_drop_table(
	const std::string &table_name, 
	Database &destination_db, 
	int not_used) {
	destination_db.execute("DROP TABLE " + prepare(table_name) + ";");
}

// Drop listed tables from Database
void batch_drop_tables(const std::string &tables, Database &destination_db) {
	
	process_table_string(tables, destination_db, sql_drop_table);
}