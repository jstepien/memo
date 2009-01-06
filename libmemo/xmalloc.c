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

#include <stdlib.h>
#include <stdio.h>
#include "xmalloc.h"

void*
xmalloc(size_t s) {
	void *p = malloc(s);
	if (!p) {
		perror("xmalloc");
		exit(1);
	}
	return p;
}

void*
xcalloc(int n, size_t s) {
	void *p = calloc(n, s);
	if (!p) {
		perror("xcalloc");
		exit(1);
	}
	return p;
}

void*
xrealloc(void *p, size_t s) {
	p = realloc(p, s);
	if (!p) {
		perror("realloc");
		exit(1);
	}
	return p;
}

/*
 * vim:ts=4:noet:tw=78
 */
