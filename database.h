/*
 * Database class for sqlite3.
 *
 * Opens database from a file upon construction.
 * The user may execute a few SQL statements during its lifespan.
 * Closes database connection upon destruction.
 *
 * The class is based on the following tutorial:
 * https://www.tutorialspoint.com/sqlite/sqlite_c_cpp.htm
 * Further optimization is done by this stackoverflow thread:
 * https://stackoverflow.com/questions/1711631/improve-insert-per-second-performance-of-sqlite
 * SQLite amalgamation source code can be downloaded from here:
 * https://www.sqlite.org/download.html
 */

#pragma once
#include <vector>
#include <iostream>
#include "sqlite-amalgamation-3350500/sqlite3.h"

class Database {
	typedef std::vector<std::string> result_container;
	std::string filename;
	sqlite3 *db;
	result_container query;
	
	// Private copy constructor
	Database(const Database &db) {}

public:
	// Open database connection
	Database(const std::string &db_name);

	// Execute SQL statement
	void execute(const std::string &sql_statement);

	// Print last SQL query results
	void print_last_query_results(std::ostream &stream = std::cout) const;
	
	// Const pointer to the last query results
	const result_container* get_last_query_results() const;
	
	// Close database connection
	~Database();
};
