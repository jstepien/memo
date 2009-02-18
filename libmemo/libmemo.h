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

typedef struct memo_word memo_word;

/**
 * @defgroup memo_database memo_database
 * Database related structures and functions.
 * @{
 */

typedef struct memo_database memo_database;

/**
 * Prepares a @ref memo_database.
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
 * Fetches a word with a given value from the database.
 * @param db the database to search.
 * @param value the string we're looking for.
 * @return pointer to the found word or @c NULL if it wasn't found or errors
 * have occured.
 */
memo_word*
memo_database_find_word_by_value(memo_database *db, const char* value);

/**
 * Fetches a word with a given key from the database.
 * @param db the database to search.
 * @param key the key we're looking for.
 * @return pointer to the found word or @c NULL if it wasn't found or errors
 * have occured.
 */
memo_word*
memo_database_find_word(memo_database *db, int key);

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
struct memo_word {
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
	/**
	 * Positive answers count.
	 */
	unsigned positive_answers;
	/**
	 * Negative answers count.
	 */
	unsigned negative_answers;
	/**
	 * Last seen value of @ref memo_database::last_change.
	 * It gets updated after each call to functions that update the word's row
	 * in the database.
	 */
	unsigned long db_last_change;
	/**
	 * Linked list of translations' keys.
	 */
	memo_translation *translations;
};

/**
 * Create a new, empty word.
 * @return a pointer to a new word, @c NULL in case of errors.
 */
memo_word*
memo_word_new(memo_database *db);

/**
 * Inserts the given word to the database.
 * This function is not updating an already inserted word - if such behaviour
 * is necessary use @ref memo_word_update.
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

/**
 * Deletes a word from the database.
 */
int
memo_word_delete(memo_word *word);

/**
 * Fetches positive answers count for a given word.
 * @return positive answers count.
 */
int
memo_word_get_positive_answers(memo_word *word);

/**
 * Fetches negative answers count for a given word.
 * @return negative answers count.
 */
int
memo_word_get_negative_answers(memo_word *word);

/**
 * Set's words positive answers count to the given value.
 */
void
memo_word_set_positive_answers(memo_word *word, unsigned value);

/**
 * Set's words negative answers count to the given value.
 */
void
memo_word_set_negative_answers(memo_word *word, unsigned value);

/**
 * Increments words positive answers count by one.
 */
void
memo_word_inc_positive_answers(memo_word *word);

/**
 * Increments words negative answers count by one.
 */
void
memo_word_inc_negative_answers(memo_word *word);

/**
 * Frees the memory occupied by a given @c memo_word.
 */
void
memo_word_free(memo_word *word);

/**
 * Returns word's database unique key.
 * @return word's key.
 */
int
memo_word_get_key(memo_word *word);

/**
 * Returns word's value.
 * @return word's value.
 */
const char*
memo_word_get_value(memo_word *word);

/**
 * Sets word's value.
 * @param word a word.
 * @param value a string to set the word's value to.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_set_value(memo_word *word, const char* value);

/**
 * Returns a pointer to a database the word's in.
 * @return the database's address.
 */
memo_database*
memo_word_get_db(memo_word *word);

/**
 * Copies a word.
 * The result is just a copy of the @ref memo_word structure. The word cannot
 * be copied inside of the database due to value's uniqueness.
 * @param dest the destination (has to be allocated using @ref memo_word_new).
 * @param src the source word.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_copy(memo_word *dest, memo_word *src);

/**
 * Inserts a new translation to the database.
 * Adds a (@c w1, @c w2) pair to the translations table.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_add_translation(memo_word *w1, memo_word *w2);

/**
 * Deletes an existing translation from the database.
 * Finds a (@c w1, @c w2) pair in the translations table and removes it.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_word_delete_translation(memo_word *w1, memo_word *w2);

/**
 * Returns word's translations.
 * @param word the word.
 * @param translations a pointer to an array of pointers to @ref memo_word
 * structures.
 * @return non-negative integer equal to the number of the given word's
 * translations count or negative values in case of errors.
 *
 * <strong>Example of usage:</strong>
 * @include memo_word_get_translations.c
 */
int
memo_word_get_translations(memo_word *word, memo_word ***translations);

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

/**
 * @defgroup memo_messaging Messaging
 * Functions preparing tests and reports and parsing replies.
 * @{
 */

/**
 * Checks a reply to a test.
 * Loads the reply from the given file, updates answers numbers in the
 * database and prepares an optional report.
 * @param reply the file with the reply to check.
 * @param db the database to update.
 * @param report a pointer to a string where the report will be saved. If it's
 * @c NULL no report be prepared.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_check_reply(FILE *reply, memo_database *db, char **report);

/**
 * Sends a test.
 * Picks words from the database and sends a test to a given e-mail address.
 * @param db the database.
 * @param words number of words to include in the test.
 * @param email the address to send the test to.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_send_test(memo_database *db, unsigned words, const char *email);

/**
 * @}
 */

#endif /* LIBMEMO_LIBMEMO_H_ */

/*
 * vim:ts=4:noet:tw=78
 */
