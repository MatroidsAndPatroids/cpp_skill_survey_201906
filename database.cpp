#include <string>
#include <vector>
#include <ostream>
#include "sqlite-amalgamation-3350500/sqlite3.h"
#include "database.h"

// C-like callback function for sqlite3
// It appends the result of the last SQL query at the end of a vector
static int callback(void *query, int argc, char **argv, char **azColName) {
	// C-like function expects vector<string>* passed as void*
	typedef std::vector<std::string> t_container;
	t_container *p = reinterpret_cast< t_container* >(query);
	
	// collect all attributes as a tab-separated string
	std::string line = (argc > 0 ? (argv[0] ? std::string(argv[0]) : "NULL") : "--empty line--");
	for(int i = 1; i < argc; i++) {
		line += '\t' + (argv[i] ? std::string(argv[i]) : "NULL");
	}
	p->push_back(line);
	return 0;
}

Database::Database(const std::string &db_name) {
	filename = db_name;
	int rc = sqlite3_open(filename.c_str(), &db);
	if( rc ) {
		std::cout << "Can't open " << filename << ": " \
			<< sqlite3_errmsg(db) << std::endl;
	} else {
		std::cout << "Opened " << filename << " successfully\n";
		char *zErrMsg = 0;
		// TRANSACTION BEGIN and END improves runtime from 30min to 6sec
		sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, &zErrMsg);
	}
}

void Database::execute(const std::string &sql_statement) {
	query.clear(); // erase stored query texts
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_exec(db, sql_statement.c_str(), callback, &query, &zErrMsg);

	if( rc != SQLITE_OK ){
		std::cout << "SQL error: " << zErrMsg << std::endl;
		sqlite3_free(zErrMsg);
	} else {
		//std::cout << "Operation completed successfully\n";
	}
}

void Database::print_last_query_results(std::ostream &stream) const {
	for (size_t i = 0; i < query.size(); ++i) {
		stream << query[i] << std::endl;
	}
}

const Database::result_container* Database::get_last_query_results() const {
	return &query;
}

Database::~Database() {
	char *zErrMsg = 0;
	sqlite3_exec(db, "END TRANSACTION", NULL, NULL, &zErrMsg);
	sqlite3_close(db);
	std::cout << "Closed " << filename << " successfully\n";
}

void test_database() {
	Database db("test.db");
	std::string sql;

	sql = "CREATE TABLE COMPANY("  \
		"ID INT PRIMARY KEY     NOT NULL," \
		"NAME           TEXT    NOT NULL," \
		"AGE            INT     NOT NULL," \
		"ADDRESS        CHAR(50)," \
		"SALARY         REAL );";
	db.execute(sql);

	sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
         "VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
         "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
         "VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";
	db.execute(sql);

	sql = "SELECT * from COMPANY"; 
	db.execute(sql);

	sql = "DROP TABLE COMPANY"; 
	db.execute(sql);
}