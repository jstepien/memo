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

#ifndef LIBMEMO_DATABASE_H_
#define LIBMEMO_DATABASE_H_

/**
 * @defgroup memo_database memo_database
 * @{
 */

enum memo_database_data_types {
	INTEGER,
	STRING
};

/**
 * Used to retrieve data from the database.
 * It represents a table.
 */
typedef struct {
	/**
	 * Number of rows.
	 */
	int rows;
	/**
	 * Number of columns.
	 */
	int cols;
	/**
	 * A two dimensional array with the data.
	 * It contains either integers or char* converted to void*.
	 */
	void ***data;
	/**
	 * An array containing information about columns' data types.
	 * It's values belong to @ref memo_database_data_types. Each field of this
	 * array tells what kind of data is stored in the respective column of
	 * @ref data table.
	 */
	int *data_types;
} memo_database_data;

/**
 * Prepares a new memo_database_data structure.
 */
memo_database_data *
memo_database_data_init();


/**
 * Frees a memo_database_data structure.
 */
void
memo_database_data_free(memo_database_data * data);

/**
 * Executes a given SQL query.
 * @param db the database to which the query will be sent.
 * @param query the query.
 * @param ret pointer to a structure where the results will be stored. If it's
 * @c NULL the results will not be saved.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_database_execute(memo_database db, const char *query,
		memo_database_data *ret);

/**
 * Initialise the database.
 * Create the structure of tables, unless they already exist. Internal use
 * only.
 * @return 0 in case of success, negative values in case of errors.
 */
int
memo_database_init(memo_database db);

/**
 * @}
 */

#endif /* LIBMEMO_DATABASE_H_ */

/*
 * vim:ts=4:noet:tw=78
 */
