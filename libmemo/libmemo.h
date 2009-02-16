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

#ifndef LIBMEMO_LIBMEMO_H_
#define LIBMEMO_LIBMEMO_H_

#include <stdio.h>
#include <sqlite3.h>

/**
 * @defgroup memo_database memo_database
 * Database related structures and functions.
 * @{
 */

/**
 * The database.
 * Currently it's an SQLite3 database.
 * @sa http://sqlite.org/
 */
typedef sqlite3 memo_database;

/**
 * Prepares a @ref memo_database.
 * @param db A database to be loaded.
 * @param filename A filename of a file with a database.
 * @return 0 in case of success, negative values in case of errors.
 */
memo_database *
memo_database_open(const char *filename);

/**
 * Closes the database.
 * @return 0.
 */
int
memo_database_close(memo_database *db);

/**
 * @}
 */

/**
 * @defgroup memo_word memo_word
 * Word related structures and functions.
 * @{
 */

/**
 * A node in a translations' list.
 * If next is @c NULL the given node is the last translation.
 */
typedef struct _memo_translation {
	/**
	 * Translation's unique key.
	 */
	int key;
	/**
	 * Pointer to a next translation.
	 */
	struct _memo_translation *next;
} memo_translation;

/**
 * Represents a word.
 * Follows an active record pattern.
 */
typedef struct {
	/**
	 * A database which contains the word.
	 */
	memo_database *db;
	/**
	 * Word's value.
	 */
	char *value;
	/**
	 * Word's unique key.
	 */
	int key;
	unsigned positive_answers;
	unsigned negative_answers;
	/**
	 * Linked list of translations' keys.
	 */
	memo_translation *translations;
} memo_word;

/**
 * Create a new, empty word.
 * @return a pointer to a new word, @c NULL in case of errors.
 */
memo_word*
memo_word_new(memo_database *db);

/**
 * Inserts the given word to the database.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_save(memo_word *word);

/**
 * Updates given word's row in the database.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_update(memo_word *word);

/**
 * Reloads the given word from the database.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_reload(memo_word *word);

/**
 * Reloads the given word from the database using it's value.
 * Dedicated to internal usage. Used in @ref memo_word_save to get word's id
 * after saving it.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_reload_by_value(memo_word *word);

/**
 * Reloads the given word from the database.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_reload(memo_word *word);

int
memo_word_delete(memo_word *word);

int
memo_word_get_positive_answers(memo_word *word);

int
memo_word_get_negative_answers(memo_word *word);

int
memo_word_free(memo_word *word);

int
memo_word_get_key(memo_word *word);

const char*
memo_word_get_value(memo_word *word);

int
memo_word_set_value(memo_word *word, const char* value);

memo_database*
memo_word_get_db(memo_word *word);

memo_word*
memo_word_find_by_value(memo_database *db, const char* value);

memo_word*
memo_word_find(memo_database *db, int id);

int
memo_word_copy(memo_word *dest, memo_word *src);

int
memo_word_add_translation(memo_word *w1, memo_word *w2);

int
memo_word_delete_translation(memo_word *w1, memo_word *w2);

/**
 * Checks whether the given translation exists in the database.
 * @return 0 if the translation in correct, 1 if it is not, negative values
 * in case of errors.
 */
int
memo_word_check_translation(memo_word *w1, memo_word *w2);

/**
 * @}
 */

#endif /* LIBMEMO_LIBMEMO_H_ */

/*
 * vim:ts=4:noet:tw=78
 */
