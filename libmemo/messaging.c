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

#include <stdio.h>
#include <sys/wait.h>
#include <libmemo.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include "messages.h"
#include "xmalloc.h"
#include "macros.h"

#define REPLY_PARSE_SCRIPT LIBDIR"/parse-reply.pl"
#define failed(x) die("%s failed at %s:%i\n", x, __FILE__, __LINE__)

int
memo_send_test(memo_database *db, unsigned words, const char *email) {
	return -1;
}

/*
 * TODO: split this function into smaller ones. It's huge.
 * A general refactoring would also be welcome.
 */
int
memo_check_reply(FILE *reply, memo_database *db, char **report) {
	int input[2], output[2], pid;
	char buf[50];
	if (pipe(input) || pipe(output))
		failed("pipe");
	if ((pid = fork()) < 0) {
		failed("fork");
	} else if (pid == 0) {
		/* We're the child. */
		if (dup2(output[1], 1) < 0)
			failed("dup2");
		if (dup2(input[0], 0) < 0)
			failed("dup2");
		close(input[1]);
		close(input[0]);
		execv(REPLY_PARSE_SCRIPT, NULL);
		failed("execv");
	} else {
		/* We're the parent. */
		int status;
		FILE *parsed;
		if (close(input[0]) || close(output[1]))
			failed("close");
		while (!feof(reply)) {
			int count;
			count = fread(buf, sizeof(char), ARRAY_SIZE(buf), reply);
			if (count < 1)
				continue;
			if (write(input[1], buf, count) < 1)
				die("write");
		}
		if (close(input[1]))
			failed("close");
		if (wait(&status) < 0) {
			perror("wait");
			failed("wait");
		}
		if ((parsed = fdopen(output[0],"r")) == NULL)
			failed("fdopen");
		while (!feof(parsed)) {
			/* TODO: That's not a very portable way of reading lines. */
			int count, match;
			static int qn = 0, an = 0;
			static char *question = NULL, *answer = NULL;
			memo_word *question_word, *answer_word;
			count = getline(&question, &qn, parsed);
			if (count < 1)
				continue;
			question[count-1] = '\0';
			count = getline(&answer, &an, parsed);
			if (count < 1)
				continue;
			answer[count-1] = '\0';
			question_word = memo_database_find_word_by_value(db, question);
			answer_word = memo_database_find_word_by_value(db, answer);
			if (question_word) {
				if (answer_word && memo_word_check_translation(question_word,
							answer_word) == 0) {
					memo_word_inc_positive_answers(question_word);
				} else {
					memo_word_inc_negative_answers(question_word);
				}
				memo_word_update(question_word);
			}
			if (question_word)
				memo_word_free(question_word);
			if (answer_word)
				memo_word_free(answer_word);
		}
	}
	return 0;
}
