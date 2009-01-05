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

int
memo_database_get_word_id(memo_database db, const char *word) {
	int retval;
	memo_database_data *results;
	const char *word_sel_templ = "SELECT (id) from words where word == \"%s\";";
	char *query;
	query = malloc(sizeof(char) * (strlen(word_sel_templ)+strlen(word)-1));
	results = memo_database_data_init();
	if (!query || !results)
		return -1;
	sprintf(query, word_sel_templ, word);
	if (memo_database_execute(db, query, results) < 0)
		return -1;
	/*
	 * KEEP IN MIND, that it works fine only if IDs are greater then 0.
	 */
	if ( results->rows < 1 )
		retval = 0;
	else
		retval = (int) results->data[0][0];
	free(query);
	memo_database_data_free(results);
	return retval;
}

int
memo_database_add_word(memo_database db, const char *key, const char *value) {
	const char *words_ins_templ = "INSERT INTO words (word) VALUES (\"%s\");";
	const char *trans_ins_templ = "INSERT INTO translations "\
			"(word_id, translation_id) VALUES (\"%i\", \"%i\");";
	char *query;
	int longer_length, key_len, value_len, key_id, value_id;

	/* TODO: Database queries in the body of this function duplicate the ones
	 * in memo_database_check_translation function. There's no need to perform
	 * them twice. Some kind of cache would be welcome. */

	/* Check whether the pair we're adding isn't in the database already. */
	if (memo_database_check_translation(db, key, value) == 0)
		return -1;

	key_len = strlen(key);
	value_len = strlen(value);
	longer_length = (key_len > value_len) ? key_len : value_len;
	/* strlen(trans_ins_templ) because it's the longest template. */
	query = malloc(sizeof(char) * (strlen(trans_ins_templ)-2+longer_length+1));
	if (!query)
		return -1;

	/* Insert a new word and it's translation to the database unless they
	 * already are there. If one of them is in the words table, insert only
	 * the one which is missing and update the translations table. */

	if ((key_id = memo_database_get_word_id(db, key)) == 0) {
		/* Insert the word. */
		sprintf(query, words_ins_templ, key);
		if (memo_database_execute(db, query, NULL) < 0)
			return -1;

		/* Get newly inserted word's ID. */
		key_id = memo_database_get_word_id(db, key);
	}
	if ((value_id = memo_database_get_word_id(db, value)) == 0) {
		/* Insert the translation. */
		sprintf(query, words_ins_templ, value);
		if (memo_database_execute(db, query, NULL) < 0)
			return -1;

		/* Get newly inserted translation's ID. */
		value_id = memo_database_get_word_id(db, value);
	}

	/* Insert the key pair to the translations table. */
	sprintf(query, trans_ins_templ, key_id, value_id);
	if (memo_database_execute(db, query, NULL) < 0)
		return -1;

	return 0;
}

int
memo_database_check_translation(memo_database db, const char *key,
		const char *value) {
	const char *trans_sel_templ = "SELECT (id) from translations where "\
			"word_id == \"%i\" AND translation_id == \"%i\";";
	char *query;
	int longer_length, key_len, value_len, key_id, value_id;
	memo_database_data *results;

	/* TODO: check return values! */

	key_len = strlen(key);
	value_len = strlen(value);
	longer_length = (key_len > value_len) ? key_len : value_len;
	/* strlen(trans_sel_templ) because it's the longest template. */
	query = malloc(sizeof(char) * (strlen(trans_sel_templ)-2+longer_length+1));

	/* Get key's and value's IDs from the database. */
	if ((key_id = memo_database_get_word_id(db, key)) == 0)
		return 1;
	else if (key_id < 0)
		return -1;
	if ( (value_id = memo_database_get_word_id(db, value)) == 0)
		return 1;
	else if (value_id < 0)
		return -1;

	/* Check whether the ID pair exists in the database. */
	results = memo_database_data_init();
	if (!results)
		return -1;

	sprintf(query, trans_sel_templ, key_id, value_id);
	memo_database_execute(db, query, results);
	if ( results->rows < 1 ) {
		/* Swap the pair and check again. */
		memo_database_data_free(results);
		results = memo_database_data_init();
		if (!results)
			return -1;
		sprintf(query, trans_sel_templ, value_id, key_id);
		memo_database_execute(db, query, results);
		if ( results->rows < 1 )
			return 1;
	}

	memo_database_data_free(results);
	return 0;
}

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
	} else
		word = NULL;
	free(query);
	memo_database_data_free(results);
	return word;
}

int
memo_word_add_translation(memo_word *w1, memo_word *w2) {
	return -1;
}

int
memo_word_delete_translation(memo_word *w1, memo_word *w2) {
	return -1;
}

int
memo_word_check_translation(memo_word *w1, memo_word *w2) {
	return -1;
}

/*
 * vim:ts=4:noet:tw=78
 */
