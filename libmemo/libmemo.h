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

#include <stdio.h>
#include <sqlite3.h>

/**
 * @defgroup memo_database memo_database
 * Database related structures and functions.
 * @{
 */

/**
 * The database.
 * Currently it's a pointer to an SQLite3 database.
 * @sa http://sqlite.org/
 */
typedef sqlite3* memo_database;

/**
 * Prepares a @ref memo_database.
 * @param db A database to be loaded.
 * @param filename A filename of a file with a database.
 */
int
memo_database_load(memo_database *db, const char *filename);

/**
 * Closes the database.
 */
int
memo_database_close(memo_database db);

/**
 * Adds a new word to the database.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_database_add_word(memo_database db, const char *key, const char *value);

/**
 * Checks whether the given translation exists in the database.
 * @return 0 if the translation in correct, 1 if it is not, negative values
 * in case of errors.
 */
int
memo_database_check_translation(memo_database db, const char *key,
		const char *value);

/**
 * @}
 */

/**
 * @defgroup memo_message memo_message
 * @ref memo_message structure and related functions.
 * @{
 */

typedef struct {
	char **headers;
	char *message;
} memo_message;

/**
 * Converts an e-mail to a @ref memo_message.
 */
int
memo_message_from_string(memo_message *msg, const char *string);

/**
 * Creates an e-mail from a @ref memo_message.
 */
int
memo_message_to_string(const memo_message *msg, char **string);

/**
 * Prepares a new tests.
 */
int
memo_message_test_prepare(const memo_message *msg, memo_database db);

/**
 * Parses a reply to a test, optionally prepares a report.
 */
int
memo_message_reply_parse(const memo_message *msg, memo_database db, memo_message *report);

/**
 * @}
 */

/*
 * vim:ts=4:noet:tw=78
 */
