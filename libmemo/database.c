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
memo_database_close(memo_database db) {
	sqlite3_close(db);
	return 0;
}

/*
 * vim:ts=4:noet:tw=78
 */
