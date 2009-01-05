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
#include "database.h"

memo_database_data *
memo_database_data_init() {
	return calloc(1, sizeof(memo_database_data));
}

void
memo_database_data_free(memo_database_data * data) {
	int i;
	/* TODO: free a string, if a column contains strings */
	for (i = 0; i < data->rows; i++)
		free(data->data[i]);
	free(data);
}

int
memo_database_load(memo_database *db, const char *filename) {
	if ( sqlite3_open(filename, db) ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		return -1;
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
		return -1;
	}
	while (1) {
		/* Execute the query. */
		rc = sqlite3_step(stmt);
		/* If everything's fine we can leave the loop. */
		if ( rc == SQLITE_DONE )
			break;
		/* Reallocate the returned data table, as we need memory for a pointer
		 * to the next row. */
		if (ret) {
			ret->data = realloc(ret->data, sizeof(void**)*(ret->rows+1));
			/* If we know the number of columns (i.e. it's not the first row)
			 * allocate memory for a new row. */
			if (ret->rows > 0)
				ret->data[ret->rows] = malloc(sizeof(void*)*ret->cols);
			else
				ret->data[0] = NULL;
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
				/* If we don't know the column count yet (i.e. it's the first row
				 * of results) count columns and reallocate memory for each
				 * column we discover. */
				if ( ret->cols == 0 ) {
					ret->data[ret->rows] = realloc(ret->data[ret->rows],
							sizeof(void*)*(++cols));
				}
				switch (type) {
					case SQLITE_INTEGER:
						ret->data[ret->rows][i] = (void*) sqlite3_column_int(stmt, i);
						break;
					case SQLITE_TEXT:
						{
							char *tmp;
							int len;
							tmp = sqlite3_column_text(stmt, i);
							len = strlen(tmp);
							ret->data[ret->rows][i] = malloc((len+1)*sizeof(char));
							strcpy(ret->data[ret->rows][i], tmp);
						}
						break;
					default:
						/* TODO: maybe an error message...? */
						return -1;
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
			return -1;
		}
	}
	if ( sqlite3_finalize(stmt) ) {
		fprintf(stderr, "Error finalising statement: %s\n", sqlite3_errmsg(db));
		return -1;
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
	if ( memo_database_execute(db, words_query, NULL) < 0 ||
			memo_database_execute(db, translations_query, NULL) < 0 )
		return -1;
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
