#include <libmemo.h>

int
memo_database_load(memo_database *db, const char *filename) {
	if ( sqlite3_open(filename, db) ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		return 1;
	}
	return 0;
}

int
memo_database_execute(memo_database db, const char *query) {
	sqlite3_stmt *stmt;
	if ( sqlite3_prepare_v2(db, query, -1, &stmt, NULL) ) {
		fprintf(stderr, "Error parsing SQL query: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	if ( sqlite3_step(stmt) != SQLITE_DONE ) {
		fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	if ( sqlite3_finalize(stmt) ) {
		fprintf(stderr, "Error finalising statement: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	return 0;
}

int
memo_database_init(memo_database db) {
	const char *words_query = "CREATE TABLE words (id integer, word text, "\
			"positive_answers integer, negative_answers integer, "\
			"PRIMARY KEY (id), UNIQUE (id, word) );";
	const char *translations_query = "CREATE TABLE translations (id integer, "\
			"word_id integer, translation_id integer, PRIMARY KEY (id), "\
			"UNIQUE (id) );";
	if ( memo_database_execute(db, words_query) ||
			memo_database_execute(db, translations_query) )
		return 1;
	return 0;
}

int
memo_database_close(memo_database db) {
	sqlite3_close(db);
	return 0;
}

int
memo_database_add_word(memo_database db, const char *key, const char *value) {
	return -1;
}

int
memo_database_check_translation(memo_database db, const char *key,
		const char *value) {
	return -1;
}

/*
 * vim:ts=4:noet:tw=78
 */
