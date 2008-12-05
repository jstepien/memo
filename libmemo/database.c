/*
 * Copyright (C) 2008 Jan Stępień
 *
 * This file is part of Memo.
 *
 * Memo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Memo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Memo.  If not, see <http://www.gnu.org/licenses/>.
 */

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
		/* Execute the query. */
		rc = sqlite3_step(stmt);
		/* If everything's fine we can leave the loop. */
		if ( rc == SQLITE_DONE )
			break;
		/* TODO: allocate it more sensibly when refactoring. */
		if (ret) {
			int i;
			ret->data = malloc(sizeof(void**)*10);
			for(  i = 0; i < 10; i += 1) {
				ret->data[i] = malloc(sizeof(void*)*10);
			}
		}
		/* If the database has returned some data and the user provided a
		 * place to store it. */
		if ( rc == SQLITE_ROW && ret ) {
			int i, type, cols;
			/* If column count isn't defined yet, it should be counted from
			 * zero. */
			if ( ret->cols == 0 )
				cols = 0;
			i = 0;
			/* While there are columns. */
			while ( ( type = sqlite3_column_type(stmt, i)) != SQLITE_NULL ) {
				if ( ret->cols == 0 )
					cols++;
				switch (type) {
					case SQLITE_INTEGER:
						ret->data[ret->rows][i] = (void*) sqlite3_column_int(stmt, i);
						break;
					case SQLITE_TEXT:
						/* TODO: copy the string and add it's address to
						 * ret->data. */
						break;
					default:
						/* TODO: maybe an error message...? */
						return 1;
				}
				i++;
			}
			/* Define the column count unless it's already defined. */
			if ( ret->cols == 0 )
				ret->cols = cols;
			/* A row has been added. */
			ret->rows++;
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
	const char *words_sel_templ = "SELECT (id) from words where word == \"%s\";";
	const char *trans_ins_templ = "INSERT INTO translations "\
			"(word_id, translation_id) VALUES (\"%i\", \"%i\");";
	char *query;
	int longer_length, key_len, value_len, key_id, value_id;
	memo_database_data *results;

	/* check return values! */

	key_len = strlen(key);
	value_len = strlen(value);
	longer_length = (key_len > value_len) ? key_len : value_len;
	/* strlen(trans_ins_templ) because it's the longest template. */
	query = malloc(sizeof(char) * (strlen(trans_ins_templ)-2+longer_length+1));

	/* Check whether a word already exists in the database before
	 * inserting. */
	sprintf(query, words_ins_templ, key);
	memo_database_execute(db, query, NULL);
	sprintf(query, words_ins_templ, value);
	memo_database_execute(db, query, NULL);

	/* TODO: There should be an initialising function */
	results = calloc(1, sizeof(memo_database_data));

	sprintf(query, words_sel_templ, key);
	memo_database_execute(db, query, results);
	key_id = (int) results->data[0][0];

	/* TODO: That's not the best solution. */
	free(results);
	results = calloc(1, sizeof(memo_database_data));

	sprintf(query, words_sel_templ, value);
	memo_database_execute(db, query, results);
	value_id = (int) results->data[0][0];

	sprintf(query, trans_ins_templ, key_id, value_id);
	memo_database_execute(db, query, NULL);

	/* TODO: There should be a freeing function */
	free(results);
	return 0;
}

int
memo_database_check_translation(memo_database db, const char *key,
		const char *value) {
	const char *words_sel_templ = "SELECT (id) from words where word == \"%s\";";
	const char *trans_sel_templ = "SELECT (id) from translations where "\
			"word_id == \"%i\" AND translation_id == \"%i\";";
	char *query;
	int longer_length, key_len, value_len, key_id, value_id;
	memo_database_data *results;

	/* check return values! */

	key_len = strlen(key);
	value_len = strlen(value);
	longer_length = (key_len > value_len) ? key_len : value_len;
	/* strlen(trans_sel_templ) because it's the longest template. */
	query = malloc(sizeof(char) * (strlen(trans_sel_templ)-2+longer_length+1));

	/* TODO: There should be an initialising function */
	results = calloc(1, sizeof(memo_database_data));

	sprintf(query, words_sel_templ, key);
	memo_database_execute(db, query, results);
	if ( results->rows < 1 ) {
		printf("no key\n");
		return 1;
	}
	key_id = (int) results->data[0][0];

	free(results);
	results = calloc(1, sizeof(memo_database_data));

	sprintf(query, words_sel_templ, value);
	memo_database_execute(db, query, results);
	if ( results->rows < 1 ) {
		printf("no trans\n");
		return 1;
	}
	value_id = (int) results->data[0][0];

	free(results);
	results = calloc(1, sizeof(memo_database_data));

	sprintf(query, trans_sel_templ, key_id, value_id);
	memo_database_execute(db, query, results);
	if ( results->rows < 1 ) {
		printf("no match\n");
		return 1;
	}
	printf("match\n");
	free(results);
	return 0;
}

/*
 * vim:ts=4:noet:tw=78
 */
