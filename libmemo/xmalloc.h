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

#ifndef LIBMEMO_XMALLOC_H_
#define LIBMEMO_XMALLOC_H_

void*
xmalloc(size_t s);

void*
xcalloc(int n, size_t s);

void*
xrealloc(void *p, size_t s);

#endif /* LIBMEMO_XMALLOC_H_ */

/*
 * vim:ts=4:noet:tw=78
 */
