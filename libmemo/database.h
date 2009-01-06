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

#ifndef LIBMEMO_DATABASE_H_
#define LIBMEMO_DATABASE_H_

enum {
	INTEGER,
	STRING
};

typedef struct {
	int rows;
	int cols;
	void ***data;
	int *data_types;
} memo_database_data;

memo_database_data *
memo_database_data_init();

void
memo_database_data_free(memo_database_data * data);

int
memo_database_execute(memo_database db, const char *query,
		memo_database_data *ret);

#endif /* LIBMEMO_DATABASE_H_ */

/*
 * vim:ts=4:noet:tw=78
 */
