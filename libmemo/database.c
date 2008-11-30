#include <libmemo.h>
#include <stdlib.h>
#include <string.h>

typedef struct _memo_database_data {
	int rows;
	int cols;
	void ***data;
} memo_database_data;

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
memo_database_execute(memo_database db, const char *query,
		memo_database_data *ret) {
	sqlite3_stmt *stmt;
	int rc;
	if ( sqlite3_prepare_v2(db, query, -1, &stmt, NULL) ) {
		fprintf(stderr, "Error parsing SQL query: %s\n", sqlite3_errmsg(db));
		return 1;
	}
	while (1) {
		rc = sqlite3_step(stmt);
		if ( rc == SQLITE_DONE )
			break;
		if ( rc == SQLITE_ROW && ret ) {
			/* Prepare the results */
		} else {
			fprintf(stderr, "Error executing statement: %s\n", sqlite3_errmsg(db));
			return 1;
		}
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
			"positive_answers integer DEFAULT 0, negative_answers integer "\
			"DEFAULT 0, PRIMARY KEY (id), UNIQUE (word) );";
	const char *translations_query = "CREATE TABLE translations (id integer, "\
			"word_id integer, translation_id integer, PRIMARY KEY (id) ); ";
	if ( memo_database_execute(db, words_query, NULL) ||
			memo_database_execute(db, translations_query, NULL) )
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
	const char *words_ins_templ = "INSERT INTO words (word) VALUES (\"%s\");";
	const char *words_sel_templ = "SELECT (id) from words where word == \"%s\");";
	char *query;
	int longer_length, key_len, value_len;

	key_len = strlen(key);
	value_len = strlen(value);
	longer_length = (key_len > value_len) ? key_len : value_len;
	query = malloc(sizeof(char) * (strlen(words_sel_templ)-2+longer_length+1));

	sprintf(query, words_ins_templ, key);
	memo_database_execute(db, query, NULL);
	sprintf(query, words_ins_templ, value);
	memo_database_execute(db, query, NULL);
	return 0;
}

int
memo_database_check_translation(memo_database db, const char *key,
		const char *value) {
	return -1;
}

/*
 * vim:ts=4:noet:tw=78
 */
