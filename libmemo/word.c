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

/**
 * @return 0 if the word doesn't exist in the database, negative values in
 * case of errors, positive value equal to the ID of a word if the word exists
 * in the database.
 */

#include <libmemo.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

memo_word*
memo_word_new(memo_database db) {
	memo_word *w;
	w = malloc(sizeof(memo_word));
	if (!w)
		return NULL;
	w->value = 0;
	w->db = db;
	return w;
}

int
memo_word_save(memo_word *word) {
	const char *query_template = "INSERT INTO words (word) VALUES (\"%s\");";
	char *query;
	memo_word *tmp_word;
	int word_length;

	word_length = strlen(memo_word_get_value(word));
	if (word_length < 1)
		return -1;

	/* Check if the word already exist. */

	tmp_word = memo_word_find_by_value(word->db, memo_word_get_value(word));
	if ( tmp_word != NULL ) {
		memo_word_free(tmp_word);
		return -1;
	}

	query = malloc(sizeof(char) * (strlen(query_template)+word_length-1));

	if (!query)
		return -1;

	sprintf(query, query_template, memo_word_get_value(word));
	if (memo_database_execute(memo_word_get_db(word), query, NULL) < 0)
		return -1;

	free(query);

	memo_word_reload_by_value(word);

	return 0;
}

int
memo_word_update(memo_word *word) {
	return -1;
}

int
memo_word_reload_by_value(memo_word *word) {
	memo_word *tmp;
	tmp = memo_word_find_by_value(word->db, word->value);
	if (!tmp)
		return -1;
	memcpy(word, tmp, sizeof(memo_word));
	memo_word_free(tmp);
	return 0;
}

int
memo_word_reload(memo_word *word) {
	memo_word *tmp;
	tmp = memo_word_find(word->db, word->key);
	if (!tmp)
		return -1;
	memcpy(word, tmp, sizeof(memo_word));
	memo_word_free(tmp);
	return 0;
}

int
memo_word_delete(memo_word *word) {
	const char *query_template = "DELETE FROM words WHERE id == %i;";
	char *query;
	memo_word *tmp_word;

	query = malloc(sizeof(char) * (strlen(query_template)+16));

	if (!query || memo_word_get_key(word) < 0)
		return -1;

	sprintf(query, query_template, memo_word_get_key(word));
	if (memo_database_execute(memo_word_get_db(word), query, NULL) < 0)
		return -1;

	free(query);

	return 0;
}

int
memo_word_free(memo_word *word) {
	/* TODO: Free translations' list. */
	free(word);
	return 0;
}

int
memo_word_get_key(memo_word *word) {
	return word->key;
}

const char*
memo_word_get_value(memo_word *word) {
	return word->value;
}

int
memo_word_set_value(memo_word *word, const char* value) {
	if (word->value)
		free(word->value);
	word->value = malloc(strlen(value));
	if (!word->value)
		return -1;
	strcpy(word->value, value);
	return 0;
}

memo_database
memo_word_get_db(memo_word *word) {
	return word->db;
}

/*
 * memo_word_find_by_value and memo_word_find have got much in common. Their
 * contents should be extracted to a third function used by both of them.
 * It isn't DRY now.
 */

memo_word*
memo_word_find_by_value(memo_database db, const char* value) {
	memo_word *word;
	memo_database_data *results;
	const char *word_sel_templ = "SELECT id, word FROM words WHERE word == \"%s\";";
	const char *trans_sel_templ = "SELECT word_id, translation_id from " \
			"translations where word_id == %i OR translation_id == %i;";
	char *query;

	query = malloc(sizeof(char) * (strlen(word_sel_templ)+strlen(value)-1));
	results = memo_database_data_init();

	if (!query || !results)
		return NULL;

	sprintf(query, word_sel_templ, value);
	if (memo_database_execute(db, query, results) < 0)
		return NULL;

	if ( results->rows == 1 ) {
		char *tmp;
		int len;
		word = memo_word_new(db);
		if (!word)
			return NULL;
		word->key = (int) results->data[0][0];
		len = strlen(results->data[0][1]);
		if (word->value)
			free(word->value);
		word->value = malloc((len+1)*sizeof(char));
		strcpy(word->value, results->data[0][1]);
		word->db = db;

		free(query);
		memo_database_data_free(results);
		query = malloc(sizeof(char) * (strlen(trans_sel_templ)+32));
		results = memo_database_data_init();
		if (!query || !results)
			return NULL;

		sprintf(query, trans_sel_templ, memo_word_get_key(word),
				memo_word_get_key(word));
		if (memo_database_execute(db, query, results) < 0)
			return NULL;
		else if	(results->rows == 0)
			word->translations = NULL;
		else {
			int i;
			memo_translation **last;
			last = &word->translations;
			for (i = 0; i < results->rows; i++) {
				memo_translation *t;
				t = calloc(1, sizeof(memo_translation));
				*last = t;
				if (memo_word_get_key(word) == (int) results->data[i][0])
					t->key = (int) results->data[i][1];
				else
					t->key = (int) results->data[i][0];
				last = &t->next;
			}
		}

	} else
		word = NULL;
	free(query);
	memo_database_data_free(results);
	return word;
}

memo_word*
memo_word_find(memo_database db, int id) {
	memo_word *word;
	memo_database_data *results;
	const char *word_sel_templ = "SELECT id, word FROM words WHERE id == %i;";
	const char *trans_sel_templ = "SELECT (id) from translations where "\
			"word_id == %i OR translation_id == %i;";
	char *query;

	/* In the following malloc call we're hoping that log10(id) < 16 .
	 * It can be verified by counting the actual logarithm, although it seems
	 * to expensive, doesn't it? */
	query = malloc(sizeof(char) * (strlen(word_sel_templ)+16));
	results = memo_database_data_init();

	if (!query || !results)
		return NULL;

	sprintf(query, word_sel_templ, id);
	if (memo_database_execute(db, query, results) < 0)
		return NULL;

	if ( results->rows == 1 ) {
		char *tmp;
		int len;
		word = memo_word_new(db);
		if (!word)
			return NULL;
		word->key = (int) results->data[0][0];
		len = strlen(results->data[0][1]);
		if (word->value)
			free(word->value);
		word->value = malloc((len+1)*sizeof(char));
		strcpy(word->value, results->data[0][1]);
		word->db = db;

		free(query);
		memo_database_data_free(results);
		query = malloc(sizeof(char) * (strlen(trans_sel_templ)+32));
		results = memo_database_data_init();
		if (!query || !results)
			return NULL;

		sprintf(query, trans_sel_templ, memo_word_get_key(word),
				memo_word_get_key(word));
		if (memo_database_execute(db, query, results) < 0)
			return NULL;
		else if	(results->rows == 0)
			word->translations = NULL;
		else {
			int i;
			memo_translation **last;
			last = &word->translations;
			for (i = 0; i < results->rows; i++) {
				memo_translation *t;
				t = calloc(1, sizeof(memo_translation));
				*last = t;
				t->key = (int) results->data[i][0];
				last = &t->next;
			}
		}
	} else
		word = NULL;
	free(query);
	memo_database_data_free(results);
	return word;
}

int
memo_word_add_translation(memo_word *w1, memo_word *w2) {
	const char *query_template = "INSERT INTO translations "\
			"(word_id, translation_id) VALUES (\"%i\", \"%i\");";
	char *query;
	int retval;

	/* Check whether the pair we're adding isn't in the database already. */
	if (memo_word_check_translation(w1, w2) == 0)
		return -1;

	query = malloc(sizeof(char) * (strlen(query_template)+32));

	/* Insert the key pair to the translations table. */
	sprintf(query, query_template, memo_word_get_key(w1),
			memo_word_get_key(w2));
	if (memo_database_execute(w1->db, query, NULL) < 0)
		retval = -1;
	else
		retval = 0;

	/*
	 * TODO: Full reload isn't necessary. Only translations' lists should be
	 * reloaded. Replace it with memo_word_reload, as it's lighter than
	 * searching by value.
	 */
	memo_word_reload_by_value(w1);
	memo_word_reload_by_value(w2);

	free(query);
	return retval;
}

int
memo_word_delete_translation(memo_word *w1, memo_word *w2) {
	return -1;
}

int
memo_word_check_translation(memo_word *w1, memo_word *w2) {
	memo_translation *t;
	if (!w1 || !w2)
		return -1;
	t = w1->translations;
	while (t) {
		if (t->key == w2->key)
			return 0;
		t = t->next;
	}
	return 1;
}

/*
 * vim:ts=4:noet:tw=78
 */
