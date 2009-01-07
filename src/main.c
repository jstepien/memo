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
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "macros.h"
#include "xmalloc.h"

int
open_database(memo_database *db) {
	char *filename, memo_db_file[] = "/.memo/db", *homedir;
	int homedir_len;
	homedir = getenv("HOME");
	homedir_len = strlen(homedir);
	filename = xmalloc(sizeof(char)*(homedir_len+ARRAY_SIZE(memo_db_file)));
	strcat(filename, homedir);
	strcat(filename + homedir_len, memo_db_file);
	if (access(filename, R_OK | W_OK) == -1 && errno != ENOENT) {
		perror("Failed to open db file for r/w");
		exit(1);
	}
	if (memo_database_load(db, filename) < 0)
		exit(1);
	return 0;
}

int
main (int argc, char const* argv[]) {
	memo_database db;
	open_database(&db);
	memo_database_close(db);
	return 0;
}
