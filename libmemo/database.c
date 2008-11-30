#include <libmemo.h>

int
memo_database_load(memo_database *db, const char *filename) {
	if( sqlite3_open(filename, db) ){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		return 1;
	}
	return 0;
}

int
memo_database_init(memo_database db) {
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
