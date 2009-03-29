/*
 * Copyright (C) 2008, 2009 Jan Stępień
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
#include <string.h>
#include <stdlib.h>
#include "database.h"
#include "macros.h"
#include "xmalloc.h"

memo_database_data *
memo_database_data_init() {
	return xcalloc(1, sizeof(memo_database_data));
}

void
memo_database_data_free(memo_database_data * data) {
	int i, j;
	/*
	 * Check which columns contain strings and free their fields.
	 */
	for (j = 0; j < data->cols; j++)
		if (data->data_types[j] == STRING)
			for (i = 0; i < data->rows; i++)
				free(data->data[i][j]);
	/*
	 * Free all rows.
	 */
	for (i = 0; i < data->rows; i++)
		free(data->data[i]);
	/*
	 * Free data types array and the whole structure.
	 */
	free(data->data_types);
	free(data);
}

memo_database*
memo_database_open(const char *filename) {
	memo_database *db = xmalloc(sizeof(memo_database));
	db->last_change = 0;
	if ( sqlite3_open(filename, &(db->db)) ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db->db));
		sqlite3_close(db->db);
		return NULL;
	}
	if (memo_database_init(db)) {
		fprintf(stderr, "Error initialising database: %s\n",
				sqlite3_errmsg(db->db));
		sqlite3_close(db->db);
		return NULL;
	}
	sqlite3_update_hook(db->db, &memo_database_update_last_change, db);
	return db;
}

int
memo_database_execute(memo_database *db, const char *query,
		memo_database_data *ret) {
	sqlite3_stmt *stmt;
	int rc;
	if (sqlite3_prepare_v2(db->db, query, -1, &stmt, NULL)) {
		fprintf(stderr, "Error parsing SQL query: %s\n",
				sqlite3_errmsg(db->db));
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
			ret->data = xrealloc(ret->data, sizeof(void**)*(ret->rows+1));
			/* If we know the number of columns (i.e. it's not the first row)
			 * allocate memory for a new row. */
			if (ret->rows > 0)
				ret->data[ret->rows] = xmalloc(sizeof(void*)*ret->cols);
			else {
				/* Otherwise prepare it for realloc by zeroing it. */
				ret->data[0] = NULL;
				/* If it's the first row (i.e. ret->rows == 0) prepare
				 * ret->data_types pointer for realloc. We're going to store
				 * columns' data types in an array it will point to. */
				ret->data_types = NULL;
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
				/* If we don't know the column count yet (i.e. it's the first row
				 * of results) count columns and reallocate memory for each
				 * column we discover. Additionally enlarge the array in which
				 * columns' data types are stored. */
				if ( ret->cols == 0 ) {
					++cols;
					ret->data[ret->rows] = xrealloc(ret->data[ret->rows],
							sizeof(void*)*cols);
					ret->data_types = xrealloc(ret->data_types,
							sizeof(int)*cols);
				}
				switch (type) {
					case SQLITE_INTEGER:
						ret->data[ret->rows][i] =
							(void*) sqlite3_column_int(stmt, i);
						if (!ret->cols)
							ret->data_types[i] = INTEGER;
						break;
					case SQLITE_TEXT:
						{
							char *tmp;
							int len;
							tmp = (char*) sqlite3_column_text(stmt, i);
							len = strlen(tmp);
							ret->data[ret->rows][i] = xmalloc((len+1)*sizeof(char));
							strcpy(ret->data[ret->rows][i], tmp);
						}
						if (!ret->cols)
							ret->data_types[i] = STRING;
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
			fprintf(stderr, "Error executing statement: %s\n",
					sqlite3_errmsg(db->db));
			return -1;
		}
	}
	if ( sqlite3_finalize(stmt) ) {
		fprintf(stderr, "Error finalising statement: %s\n",
				sqlite3_errmsg(db->db));
		return -1;
	}
	return 0;
}

int
memo_database_init(memo_database *db) {
	const char *queries[] = {
		"CREATE TABLE IF NOT EXISTS words " \
			"(id integer, word text, positive_answers integer DEFAULT 0, " \
			"negative_answers integer DEFAULT 0, language_id integer, " \
			"PRIMARY KEY (id), UNIQUE (word) );",
		"CREATE TABLE IF NOT EXISTS " \
			"translations (id integer, word_id integer, translation_id " \
			"integer, PRIMARY KEY (id) );",
		"CREATE TABLE IF NOT EXISTS languages "\
			"(id integer, name text, PRIMARY KEY (id), UNIQUE (name) );"
	};
	int i;
	for (i = 0; i < ARRAY_SIZE(queries); ++i)
		if (memo_database_execute(db, queries[i], NULL) < 0 )
			return -1;
	return 0;
}

int
memo_database_close(memo_database *db) {
	sqlite3_close(db->db);
	return 0;
}

memo_word*
memo_database_find_word_by_value(memo_database *db, const char* value) {
	memo_word **words, *word;
	memo_database_data *results;
	const char word_sel_templ[] = ""
		"SELECT w.id, word, positive_answers, negative_answers, l.name "
		"FROM words w INNER JOIN languages l ON l.id = w.language_id "
		"WHERE word == \"%s\";";
	char *query;

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(word_sel_templ)+strlen(value)-1));
	results = memo_database_data_init();

	if (!results)
		return NULL;

	sprintf(query, word_sel_templ, value);
	if (memo_database_execute(db, query, results) < 0)
		return NULL;
	if (words = memo_database_load_words_from_database_data(db, results)) {
		word = words[0];
		free(words);
	}
	else
		word = NULL;
	free(query);
	memo_database_data_free(results);
	return word;
}

memo_word*
memo_database_find_word(memo_database *db, int id) {
	memo_word *word, **words;
	memo_database_data *results;
	const char word_sel_templ[] = ""
		"SELECT w.id, word, positive_answers, negative_answers, l.name "
		"FROM words w INNER JOIN languages l ON l.id = w.language_id "
		"WHERE w.id == \"%i\";";
	char *query;

	/* In the following malloc call we're hoping that log10(id) < 16 .
	 * It can be verified by counting the actual logarithm, although it seems
	 * to expensive, doesn't it? */
	query = xmalloc(sizeof(char) * (ARRAY_SIZE(word_sel_templ)+16));
	results = memo_database_data_init();

	if (!results)
		return NULL;

	sprintf(query, word_sel_templ, id);
	if (memo_database_execute(db, query, results) < 0)
		return NULL;
	if (words = memo_database_load_words_from_database_data(db, results)) {
		word = words[0];
		free(words);
	}
	else
		word = NULL;
	free(query);
	memo_database_data_free(results);
	return word;
}

void
memo_database_update_last_change(void *data, int action, char const *dbname,
		char const *rowname, sqlite3_int64 rowid) {
	((memo_database*) data)->last_change++;
}

unsigned long
memo_database_get_last_change(memo_database *db) {
	return db->last_change;
}

memo_word**
memo_database_load_words_from_database_data(memo_database *db,
		memo_database_data *data) {
	const char trans_sel_templ[] = "SELECT word_id, translation_id from "
		"translations where word_id == %i OR translation_id == %i;";
	char query[ARRAY_SIZE(trans_sel_templ)+32];
	memo_word **words;
	memo_database_data *transl_data;
	int i;
	if (data->rows < 1)
		return NULL;
	words = xcalloc(data->rows, sizeof(memo_word*));
	for (i = 0; i < data->rows; ++i) {
		words[i] = memo_word_new(db);
		if (!words[i])
			return NULL;
		words[i]->key = (int) data->data[i][0];
		memo_word_set_value(words[i], data->data[i][1]);
		memo_word_set_positive_answers(words[i], (int) data->data[i][2]);
		memo_word_set_negative_answers(words[i], (int) data->data[i][3]);
		memo_word_set_language(words[i], data->data[i][4]);
		words[i]->db = db;
		words[i]->db_last_change = memo_database_get_last_change(db);

		sprintf(query, trans_sel_templ, memo_word_get_key(words[i]),
				memo_word_get_key(words[i]));
		transl_data = memo_database_data_init();
		if (memo_database_execute(db, query, transl_data) < 0)
			return NULL;
		else if	(transl_data->rows == 0)
			words[i]->translations = NULL;
		else {
			int j;
			memo_translation **last;
			last = &words[i]->translations;
			for (j = 0; j < transl_data->rows; j++) {
				memo_translation *t;
				t = xcalloc(1, sizeof(memo_translation));
				*last = t;
				if (memo_word_get_key(words[i]) ==
						(int) transl_data->data[j][0])
					t->key = (int) transl_data->data[j][1];
				else
					t->key = (int) transl_data->data[j][0];
				last = &t->next;
			}
		}
		memo_database_data_free(transl_data);
	}
	return words;
}

memo_word**
memo_database_words_to_test(memo_database *db, int count) {
	memo_word **words;
	memo_database_data *results;
	const char word_sel_templ[] = ""
		"SELECT w.id, word, positive_answers, negative_answers, l.name "
		"FROM words w INNER JOIN languages l ON l.id = w.language_id "
		"ORDER BY (negative_answers / positive_answers) DESC LIMIT %i;";
	char *query;

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(word_sel_templ)+16));
	results = memo_database_data_init();

	if (!results)
		return NULL;

	sprintf(query, word_sel_templ, count);
	if (memo_database_execute(db, query, results) < 0)
		return NULL;
	words = memo_database_load_words_from_database_data(db, results);
	free(query);
	memo_database_data_free(results);
	return words;
}

int
memo_database_get_language_key(memo_database *db, const char *lang) {
	const char insert_template[] = ""
		"INSERT INTO languages (name) VALUES (\"%s\");";
	const char select_template[] = ""
		"SELECT id FROM languages where name = \"%s\";";
	char *select_query, *insert_query;
	memo_database_data *results;
	int len = strlen(lang), retval;

	results = memo_database_data_init();
	select_query = xmalloc(sizeof(char) * (ARRAY_SIZE(select_template)+len));
	sprintf(select_query, select_template, lang);
	if (memo_database_execute(db, select_query, results) < 0) {
		memo_database_data_free(results);
		free(select_query);
		return -1;
	} else if (results->rows > 0) {
		retval = results->data[0][0];
		memo_database_data_free(results);
		free(select_query);
		return retval;
	}

	/* If there's no such language, insert it. */

	insert_query = xmalloc(sizeof(char) * (ARRAY_SIZE(insert_template)+len));
	sprintf(insert_query, insert_template, lang);
	if (memo_database_execute(db, insert_query, NULL) < 0) {
		memo_database_data_free(results);
		free(insert_query);
		free(select_query);
		return -1;
	}
	free(insert_query);

	if (memo_database_execute(db, select_query, results) == 0 &&
			results->rows > 0) {
		retval = results->data[0][0];
		memo_database_data_free(results);
		free(select_query);
		return retval;
	}
	memo_database_data_free(results);
	free(select_query);
	return -1;
}

/*
 * vim:ts=4:noet:tw=78
 */
