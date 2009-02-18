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
#include <stdlib.h>
#include <string.h>
#include "database.h"
#include "macros.h"
#include "xmalloc.h"

memo_word*
memo_word_new(memo_database *db) {
	memo_word *w;
	w = xcalloc(1, sizeof(memo_word));
	w->key = -1;
	w->db = db;
	return w;
}

/*
 * Checks whether the database has been updated since the previous check.
 */
void
memo_word_auto_reload(memo_word *word) {
	if (memo_database_get_last_change(memo_word_get_db(word)) >
			word->db_last_change)
		memo_word_reload(word);
}

int
memo_word_save(memo_word *word) {
	const char query_template[] = "INSERT INTO words (word) VALUES (\"%s\");";
	char *query;
	memo_word *tmp_word;
	int word_length;

	word_length = strlen(memo_word_get_value(word));
	if (word_length < 1)
		return -1;

	/* Check if the word already exist. */

	tmp_word = memo_database_find_word_by_value(word->db, memo_word_get_value(word));
	if ( tmp_word != NULL ) {
		memo_word_free(tmp_word);
		return -1;
	}

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(query_template)+word_length-1));

	sprintf(query, query_template, memo_word_get_value(word));
	if (memo_database_execute(memo_word_get_db(word), query, NULL) < 0)
		return -1;

	free(query);

	return memo_word_reload_by_value(word);
}

int
memo_word_update(memo_word *word) {
	const char query_template[] = "UPDATE words "
		"SET word=\"%s\", positive_answers=%i, negative_answers=%i "
		"WHERE id==%i";
	char *query;
	int word_length;

	word_length = strlen(memo_word_get_value(word));
	if (word_length < 1)
		return -1;

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(query_template)+word_length-1));

	/* memo_word_get_* functions cannot be used here as they would force an
	 * automatical update and the value we'd want to save in the database
	 * would get overwritten with the old value from the database. */
	sprintf(query, query_template, word->value, word->positive_answers,
			word->negative_answers, word->key);
	if (memo_database_execute(memo_word_get_db(word), query, NULL) < 0)
		return -1;

	free(query);

	return 0;
}

int
memo_word_reload_by_value(memo_word *word) {
	memo_word *tmp;
	tmp = memo_database_find_word_by_value(word->db, word->value);
	if (!tmp)
		return -1;
	memo_word_copy(word, tmp);
	memo_word_free(tmp);
	return 0;
}

int
memo_word_reload(memo_word *word) {
	memo_word *tmp;
	tmp = memo_database_find_word(word->db, word->key);
	if (!tmp)
		return -1;
	memo_word_copy(word, tmp);
	memo_word_free(tmp);
	return 0;
}

int
memo_word_delete(memo_word *word) {
	const char query_template[] = "DELETE FROM words WHERE id == %i;";
	char *query;
	memo_word *tmp_word;

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(query_template)+16));

	if (memo_word_get_key(word) < 0)
		return -1;

	sprintf(query, query_template, memo_word_get_key(word));
	if (memo_database_execute(memo_word_get_db(word), query, NULL) < 0)
		return -1;

	free(query);

	return 0;
}

int
memo_word_copy(memo_word *dest, memo_word *src) {
	memo_translation **dest_tr = &dest->translations,
					 *src_tr = src->translations;
	memcpy(dest, src, sizeof(memo_word));
	while (src_tr) {
		*dest_tr = xmalloc(sizeof(memo_translation));
		(*dest_tr)->key = src_tr->key;
		dest_tr = &(*dest_tr)->next;
		src_tr = src_tr->next;
	}
	*dest_tr = NULL;
	return 0;
}

void
memo_word_free(memo_word *word) {
	memo_translation *prev, *cur = word->translations;
	while (cur) {
		prev = cur;
		cur = cur->next;
		free(prev);
	}
	free(word);
}

int
memo_word_get_negative_answers(memo_word *word) {
	memo_word_auto_reload(word);
	return word->negative_answers;
}

int
memo_word_get_positive_answers(memo_word *word) {
	memo_word_auto_reload(word);
	return word->positive_answers;
}

int
memo_word_get_key(memo_word *word) {
	/* No auto reload here as key should be constant. */
	return word->key;
}

const char*
memo_word_get_value(memo_word *word) {
	memo_word_auto_reload(word);
	return word->value;
}

int
memo_word_set_value(memo_word *word, const char* value) {
	if (word->value)
		free(word->value);
	word->value = xmalloc((strlen(value)+1)*sizeof(char));
	strcpy(word->value, value);
	return 0;
}

memo_database*
memo_word_get_db(memo_word *word) {
	/* No auto reload here as db should be constant. */
	return word->db;
}

memo_word*
memo_word_load_from_database_data(memo_database *db,
		memo_database_data *data) {
	const char trans_sel_templ[] = "SELECT word_id, translation_id from " \
			"translations where word_id == %i OR translation_id == %i;";
	char *query;
	memo_word *word;
	memo_database_data *results;
	if ( data->rows == 1 ) {
		char *tmp;
		word = memo_word_new(db);
		if (!word)
			return NULL;
		word->key = (int) data->data[0][0];
		memo_word_set_value(word, data->data[0][1]);
		word->db = db;
		word->db_last_change = memo_database_get_last_change(db);

		query = xmalloc(sizeof(char) * (ARRAY_SIZE(trans_sel_templ)+32));
		data = memo_database_data_init();
		if (!data)
			return NULL;

		sprintf(query, trans_sel_templ, memo_word_get_key(word),
				memo_word_get_key(word));
		if (memo_database_execute(db, query, data) < 0)
			return NULL;
		else if	(data->rows == 0)
			word->translations = NULL;
		else {
			int i;
			memo_translation **last;
			last = &word->translations;
			for (i = 0; i < data->rows; i++) {
				memo_translation *t;
				t = xcalloc(1, sizeof(memo_translation));
				*last = t;
				if (memo_word_get_key(word) == (int) data->data[i][0])
					t->key = (int) data->data[i][1];
				else
					t->key = (int) data->data[i][0];
				last = &t->next;
			}
		}
		memo_database_data_free(data);
		free(query);
	} else
		return NULL;
	return word;
}

int
memo_word_add_translation(memo_word *w1, memo_word *w2) {
	const char query_template[] = "INSERT INTO translations "\
			"(word_id, translation_id) VALUES (\"%i\", \"%i\");";
	char *query;
	int retval;

	/* Check whether the pair we're adding isn't in the database already. */
	if (memo_word_check_translation(w1, w2) == 0)
		return -1;

	query = xmalloc(sizeof(char) * (ARRAY_SIZE(query_template)+32));

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

int
memo_word_get_translations(memo_word *word, memo_word ***translations) {
	memo_translation *t;
	int count = 0, pos;
	if (!word || !translations)
		return -1;
	memo_word_auto_reload(word);
	if (!word->translations)
		return 0;
	t = word->translations;
	do
		++count;
	while (t = t->next);
	*translations = xmalloc(count*sizeof(memo_word*));
	t = word->translations;
	pos = 0;
	do {
		(*translations)[pos++] = memo_database_find_word(memo_word_get_db(word),
				t->key);
	} while (t = t->next);
	return count;
}

/*
 * vim:ts=4:noet:tw=78
 */
