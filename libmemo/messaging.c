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
#include <time.h>
#include "messages.h"
#include "xmalloc.h"
#include "macros.h"
#include "messaging.h"

#define REPLY_PARSE_SCRIPT LIBDIR"/parse-reply.pl"
#define failed(x) die("%s failed at %s:%i\n", x, __FILE__, __LINE__)

int
memo_send_test(memo_database *db, unsigned count, const char *email) {
	const char command_templ[] = "mailx \"%s\" -s \"memo [%i]\"";
	FILE *mailx;
	char *command;
	memo_word **words;
	int i;
	command = xmalloc(ARRAY_SIZE(command_templ) + strlen(email) + 12);
	sprintf(command, command_templ, email, time(0));
	words = memo_database_words_to_test(db, count);
	mailx = popen(command, "w");
	for (i = 0; i < count && words[i]; ++i) {
		fprintf(mailx, "%s = \n", memo_word_get_value(words[i]));
		memo_word_free(words[i]);
	}
	free(words);
	free(command);
	if (pclose(mailx) < 0)
		return -1;
	return 0;
}

/*
 * TODO: A general refactoring would be welcome.
 */
int
memo_check_reply(FILE *reply, memo_database *db, char **report) {
	int input[2], output[2], pid;
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
		if (close(input[0]) || close(output[1]))
			failed("close");
		send_data_to_checking_script(reply, input[1]);
		if (wait(&status) < 0) {
			perror("wait");
			failed("wait");
		}
		read_data_from_checking_script(output[0], db);
	}
	return 0;
}

void
send_data_to_checking_script(FILE *data, int fd) {
	char buf[50];
	while (!feof(data)) {
		int count;
		count = fread(buf, sizeof(char), ARRAY_SIZE(buf), data);
		if (count < 1)
			continue;
		if (write(fd, buf, count) < 1)
			die("write");
	}
	if (close(fd))
		failed("close");
}

void
read_data_from_checking_script(int fd, memo_database *db) {
	FILE *parsed;
	if ((parsed = fdopen(fd, "r")) == NULL)
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
		if (question_word) {
			answer_word = memo_database_find_word_by_value(db, answer);
			if (answer_word && memo_word_check_translation(question_word,
						answer_word) == 0) {
				memo_word_inc_positive_answers(question_word);
			} else {
				memo_word_inc_negative_answers(question_word);
			}
			memo_word_update(question_word);
			memo_word_free(question_word);
			if (answer_word)
				memo_word_free(answer_word);
		}
	}
}
