/*
 * TASK DESCRIPTION:
 * Compare the adverse events (co-occurence of a substance and a PT code) 
 * contained in the IMI Protect database and the SIDER database.
 * Provide a list about the common adverse events and draw a Venn diagram
 * which shows the number of adverse events in the two databases.
 *
 * SOLUTION:
 * - Common adverse event list is a list of SUBSTANCE NAME - PT NAME pairs.
 *   The list is generated via an SQLite intersection on the two datasets.
 *   SQLite does not require a database server as it operates from a file.
 * - The Venn diagram consists of 2 overlapping circles for the 2 datasets.
 *   Each of the 3 component contains the numbercount of those subsets.
 * - The C++ code uses STL, BOOST, and SQLite Amalgamation (attached in C).
 *   Files:
 *     main.cpp - Assemble solution
 *     database.h & .cpp - Manipulate SQLite DB
 *     parse.h & .cpp - Read TSV files into the DB
 *     ascii_graphics.h & .cpp - Draw minimalistic figures
 *   Compiling with GCC:
 *     1. Create sqlite3.o in subdir:
 *     gcc -c sqlite-amalgamation-3350500\sqlite3.c -o sqlite-amalgamation-3350500\sqlite3.o;
 *     2. Create a.exe executable
 *     g++ main.cpp parse.cpp database.cpp ascii_graphics.cpp sqlite-amalgamation-3350500\sqlite3.o -I C:\local\boost_1_76_0 -O3 -Wall;
 *
 * TODO:
 * - Exception handling
 * - Makefile
 * - XLS -> tsv converter (currently its done manually)
 * - Real graphics for the Venn diagram
 */

#include <string>
#include <iostream>
#include <fstream>
#include "parse.h"
#include "database.h"
#include "ascii_graphics.h"

/*
 * SQL statement HOWTO:
 *
 * In SQLite 'EXCEPT' is MINUS and lower() converts text to lowercase.
 *
 * CopyofFinalrepository_DLP30Jun2016.tsv attributes:
 * A2 = SUBSTANCE NAME
 * A9 = MEDDRA PT
 * 
 * meddra_all_se.tsv attributes:
 * A1 = SUBSTANCE ID (joined with drug_names.A1)
 * A4 = MEDDRA LEVEL (filter for 'PT' only)
 * A6 = MEDDRA LLT or PT
 * drug_names attributes:
 * A1 = SUBSTANCE ID (joined with meddra_all_se.A1)
 * A2 = SUBSTANCE NAME
 * meddra.tsv is not used at all!
 */


// Count the number of the distinct adverse effects in IMI but not in Sider
std::string imi_only(Database &db) {
	db.execute("SELECT COUNT (*) as IMI_minus_Sider_distinct \
		from ( SELECT lower(A2) as DrugName, lower(A9) as AdverseEffect \
			from 'input\\CopyofFinalrepository_DLP30Jun2016.tsv' \
			EXCEPT \
			SELECT lower(Drug.A2) as DrugName, lower(AllSe.A6) as AdverseEffect\
			from 'input\\meddra_all_se.tsv' as AllSe \
				INNER JOIN \
				'input\\drug_names.tsv' as Drug \
					where AllSe.A4 = 'PT' and Drug.A1 = AllSe.A1 )");
	std::string count_text = (db.get_last_query_results())->front();
	return count_text;
}

// Count the number of the distinct adverse effects not in IMI but in Sider
std::string sider_only(Database &db) {
	db.execute("SELECT COUNT (*) as Sider_minus_IMI_distinct \
		from (SELECT lower(Drug.A2) as DrugName, lower(AllSe.A6) as AdverseEffect\
			from 'input\\meddra_all_se.tsv' as AllSe \
				INNER JOIN \
				'input\\drug_names.tsv' as Drug \
					where AllSe.A4 = 'PT' and Drug.A1 = AllSe.A1  \
			EXCEPT \
			SELECT lower(A2) as DrugName, lower(A9) as AdverseEffect \
			from 'input\\CopyofFinalrepository_DLP30Jun2016.tsv' )");
	std::string count_text = (db.get_last_query_results())->front();
	return count_text;
}

// Count the number of the distinct adverse effects both in IMI and in Sider
std::string intersection(Database &db) {
	db.execute("SELECT COUNT (*) as IMI_intersect_Sider_distinct \
		from ( SELECT lower(A2) as DrugName, lower(A9) as AdverseEffect \
			from 'input\\CopyofFinalrepository_DLP30Jun2016.tsv' \
			INTERSECT \
			SELECT lower(Drug.A2) as DrugName, lower(AllSe.A6) as AdverseEffect\
			from 'input\\meddra_all_se.tsv' as AllSe \
				INNER JOIN \
				'input\\drug_names.tsv' as Drug \
					where AllSe.A4 = 'PT' and Drug.A1 = AllSe.A1 )");
	std::string count_text = (db.get_last_query_results())->front();
	return count_text;
}

// Write result of the mutual adverse effect SQL query to a file.
void write_adverse_effect_list(
	Database &db, 
	const std::string &destination_file) {
	std::ofstream out_file(destination_file.c_str());
	db.execute("SELECT lower(A2) as DrugName, lower(A9) as AdverseEffect \
		from 'input\\CopyofFinalrepository_DLP30Jun2016.tsv' \
		INTERSECT \
		SELECT lower(Drug.A2) as DrugName, lower(AllSe.A6) as AdverseEffect\
		from 'input\\meddra_all_se.tsv' as AllSe \
			INNER JOIN \
			'input\\drug_names.tsv' as Drug \
				where AllSe.A4 = 'PT' and Drug.A1 = AllSe.A1");
	db.print_last_query_results(out_file);
}

// Write ASCII Venn diagram to a file. Input 3 texts for each component.
void write_imi_sider_venn_diagram(
	const std::string &destination_file, 
	std::string imi_only, 
	std::string intersection, 
	std::string sider_only) {
	std::ofstream out_file(destination_file.c_str());
	const int cols = 48;
	const int rows = 24;
	
	Ascii_graphics canvas(rows, cols);
	canvas.draw_circle(9, 12, 12);
	canvas.draw_circle(9, 21, 12);
	canvas.draw_text(imi_only, 5, 12);
	canvas.draw_text(intersection, 14, 12);
	canvas.draw_text(sider_only, 23, 12);
	canvas.draw_text("IMI", 5, 22);
	canvas.draw_text("Sider", 23, 22);
	canvas.print(out_file);
}

// Calculate requested numbers and the list via SQL queries,
// and write the list and the Venn diagram to files
void solution(Database &db) {
	const std::string tsv_filename = "output\\imi_sider_mutual_adverse_effects.tsv";
	std::cout << "Writing " << tsv_filename << std::endl;
	write_adverse_effect_list(db, tsv_filename);
	
	const std::string txt_filename = "output\\modest_venn_diagram.txt";
	std::cout << "Writing " << txt_filename << std::endl;
	const std::string imi = imi_only(db);
	const std::string sider = sider_only(db);
	const std::string intersect = intersection(db);
	write_imi_sider_venn_diagram(txt_filename, imi, intersect, sider);
}

// Prepare environment for the solution and run it
int main(int argc, char* argv[]) {
	// |-separated filename - header_length pairs
	const std::string tables = "input\\meddra.tsv|0|input\\drug_names.tsv|0|input\\meddra_all_se.tsv|0|input\\CopyofFinalrepository_DLP30Jun2016.tsv|9";
	
	// Open database file (created if doesn't exist)
	Database db("output\\survey.db");
	
	// Read each file into a table of the same name
	batch_insert_tsv_file_into_database(tables, db);
	
	// Solve the task
	solution(db);
	
	// Restore original state of the database
	batch_drop_tables(tables, db);
	
	return 0;
}
