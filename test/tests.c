#include <libmemo.h>
#include <check.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#define DBNAME "./tmpdb"
#define ERR_LOAD "Cannot load a database in "DBNAME"."
#define ERR_CLOSE "Cannot close an opened database."
#define ERR_ADDING "Cannot add a word to the database."
#define ERR_ADDING_TR "Cannot add a translation to the database."
#define ERR_FETCHING_TR "Cannot fetch translations from the database."
#define ERR_SET_WORD_VAL "Failed to set word's value."

memo_database *db;

void
database_remove() {
	/* Remove the database file. It's fine if it does not exist. */
	assert(unlink(DBNAME) == 0 && errno != ENOENT);
}

/*
 * Removes the database file and creates a new one.
 */
void
database_setup() {
	database_remove();
	assert(db = memo_database_open(DBNAME));
}

/*
 * Closes the database.
 */
void
database_teardown() {
	assert(memo_database_close(db) == 0);
}

/*
 * Checks whether two words' translations lists contain same keys.
 * @return 0 if they are identical, 1 otherwise.
 */
int
word_cmp_transl(memo_word *a, memo_word *b) {
	memo_translation *tr;
	int counta = 0, countb = 0, *array, i, retval;
	/*
	 * Firstly check whether both lists are of equal lenght.
	 */
	tr = a->translations;
	while (tr) {
		++counta;
		tr = tr->next;
	}
	tr = b->translations;
	while (tr) {
		++countb;
		tr = tr->next;
	}
	if (counta != countb)
		return 1;
	/*
	 * We'll fill an array with a's translations' keys and for each b's
	 * translation's key we'll try to find a field in the array with the same
	 * value. If found, it will be set to -1. If the table is filled with -1
	 * and all b's translations' keys have been found, then the translations'
	 * lists are identical.
	 *
	 * The algorithm is terribly slow but on the other hand a quicker test
	 * based on a balanced BST wouldn't be worth the time spent on
	 * implementation. Maybe one day I'll come up with something at least
	 * a bit optimised.
	 */
	array = xmalloc(counta*sizeof(int));
	tr = a->translations;
	counta = 0;
	while (tr) {
		array[counta++] = tr->key;
		tr = tr->next;
	}
	tr = b->translations;
	while (tr) {
		for (i = 0; i < counta; ++i)
			if (array[i] == tr->key) {
				array[i] = -1;
				break;
			}
		tr = tr->next;
	}
	retval = 0;
	for (i = 0; i < counta; ++i)
		if (array[i] != -1) {
			retval = 1;
			break;
		}
	free(array);
	return retval;
}

/*
 * Checks whether two words are identical.
 * @return 0 if they are identical, 1 otherwise.
 */
int
word_cmp(memo_word *a, memo_word *b) {
	if (a == b)
		return 0;
	/*
	 * TODO: compare the list of translations.
	 */
	if (strcmp(memo_word_get_value(a), memo_word_get_value(b)) == 0
			&& memo_word_get_key(a) == memo_word_get_key(b)
			&& memo_word_get_negative_answers(a) == memo_word_get_negative_answers(b)
			&& memo_word_get_positive_answers(a) == memo_word_get_positive_answers(b)
			&& memo_word_get_db(a) == memo_word_get_db(b)
			&& word_cmp_transl(a, b) == 0)
		return 0;
	return 1;
}

/*
 * Creates and closes a database file.
 */
START_TEST (database_openclose)
{
	memo_database *db;
	fail_if(!(db = memo_database_open(DBNAME)), ERR_LOAD);
	fail_if(memo_database_close(db) != 0, ERR_CLOSE);
}
END_TEST

/*
 * Inserts a word to the database.
 */
START_TEST (word_inserting)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, ERR_SET_WORD_VAL);
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	/* TODO: Should check whether the word has been saved. */
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word and checks if it can be found by it's key.
 */
START_TEST (word_find_by_key)
{
	memo_word *w1, *w2;
	w1 = memo_word_new(db);
	fail_if(w1 == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(w1, "Test") != 0, ERR_SET_WORD_VAL);
	fail_if(memo_word_save(w1) != 0, "Failed to save a word.");
	w2 = memo_database_find_word(db, memo_word_get_key(w1));
	fail_if(w2 == NULL ||
			strcmp(memo_word_get_value(w1), memo_word_get_value(w2)) != 0,
			"Failed to find the inserted word.");
	memo_word_free(w1);
	memo_word_free(w2);
}
END_TEST

/*
 * Inserts a word, retrieves it and checks whether memo_word_get_key works
 * correctly.
 */
START_TEST (word_get_key)
{
	memo_word *word;
	int id;
	word = memo_word_new(db);
	fail_if(word == NULL, "can't create a new word.");
	fail_if(memo_word_set_value(word, "test") != 0, "failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "failed to save a word.");
	id = memo_word_get_key(word);
	memo_word_free(word);
	word = memo_database_find_word_by_value(db, "test");
	fail_if(id != memo_word_get_key(word), "Word's key does not match.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word, retrieves it and checks whether memo_word_get_db works
 * correctly.
 */
START_TEST (word_get_db)
{
	memo_word *word;
	memo_database *tmpdb;
	int positive_answers, negative_answers;
	word = memo_word_new(db);
	fail_if(word == NULL, "can't create a new word.");
	fail_if(memo_word_set_value(word, "test") != 0, "failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "failed to save a word.");
	tmpdb = memo_word_get_db(word);
	memo_word_free(word);
	word = memo_database_find_word_by_value(db, "test");
	fail_if(tmpdb != memo_word_get_db(word), "DB does not match.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word, retrieves it and checks whether
 * memo_word_get_positive_answers and memo_word_get_negative_answers work
 * correctly.
 */
START_TEST (word_get_answers_count)
{
	memo_word *word;
	int positive_answers, negative_answers;
	word = memo_word_new(db);
	fail_if(word == NULL, "can't create a new word.");
	fail_if(memo_word_set_value(word, "test") != 0, "failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "failed to save a word.");
	positive_answers = memo_word_get_positive_answers(word);
	negative_answers = memo_word_get_negative_answers(word);
	memo_word_free(word);
	word = memo_database_find_word_by_value(db, "test");
	fail_if(negative_answers != memo_word_get_negative_answers(word),
			"negative answers count does not match.");
	fail_if(positive_answers != memo_word_get_positive_answers(word),
			"positive answers count does not match.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word and checks whether it can be found by it's value.
 */
START_TEST (word_find_by_value)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, ERR_SET_WORD_VAL);
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	memo_word_free(word);
	word = memo_database_find_word_by_value(db, "Test");
	fail_if(word == NULL, "Failed to find the inserted word.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word and checks whether reloading works.
 */
START_TEST (word_reload)
{
	memo_word *w1, *w2;
	w1 = memo_word_new(db);
	w2 = (memo_word*) xmalloc(sizeof(memo_word));
	fail_if(!w1 || !w2, "Can't create a new word.");
	fail_if(memo_word_set_value(w1, "Test") != 0, ERR_SET_WORD_VAL);
	fail_if(memo_word_save(w1) != 0, "Failed to save a word.");
	w2->key = memo_word_get_key(w1);
	w2->db = memo_word_get_db(w1);
	fail_if(memo_word_reload(w2) != 0, "Failed to reload a saved word.");
	fail_if(word_cmp(w1, w2) != 0, "Reloaded word isn't similar to the original.");
	memo_word_free(w1);
	memo_word_free(w2);
}
END_TEST

/*
 * Inserts a word, deletes it and checks whether it can be found or reloaded.
 */
START_TEST (word_delete)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, ERR_SET_WORD_VAL);
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	fail_if(memo_word_delete(word) != 0, "Failed to delete a saved word.");
	fail_if(memo_word_reload(word) == 0, "Reloaded a deleted word.");
	memo_word_free(word);
}
END_TEST

/*
 * Checks if memo_word_update updates word's row in the database properly.
 * Another word is created after the first word is saved but before it's updated
 * so we're also checking if the function handles such cases properly (i.e.
 * doesn't automatically reload first word's contents as the database's last
 * change number has changed).
 */
START_TEST (word_update)
{
	memo_word *w1, *w2;
	const char str1[] = "qwe", str2[] = "asd", str3[] = "zxc";
	w1 = memo_word_new(db);
	memo_word_set_value(w1, str1);
	memo_word_save(w1);

	w2 = memo_word_new(db);
	memo_word_set_value(w2, str1);

	memo_word_set_value(w1, str3);
	fail_if(memo_word_update(w1) != 0);

	memo_word_save(w2);

	fail_if(strcmp(memo_word_get_value(w1), str3) != 0);

	memo_word_free(w1);
	memo_word_free(w2);

	fail_if((w2 = memo_database_find_word_by_value(db, str3)) == NULL);

	memo_word_free(w2);
}
END_TEST

/*
 * Inserts a word, copies it and checks whether it has been copied properly.
 */
START_TEST (word_copy)
{
	memo_word *word, *copy, *tmp_word;
	word = memo_word_new(db);
	copy = memo_word_new(db);
	memo_word_set_value(word, "Test");
	memo_word_save(word);
	fail_if(memo_word_copy(copy, word) != 0, 0);

	tmp_word = memo_database_find_word_by_value(db, memo_word_get_value(copy));
	fail_if(word_cmp(tmp_word, word) != 0, 0);
	memo_word_free(tmp_word);

	tmp_word = memo_database_find_word(db, memo_word_get_key(copy));
	fail_if(word_cmp(tmp_word, word) != 0, 0);
	memo_word_free(tmp_word);

	memo_word_free(word);
	memo_word_free(copy);
}
END_TEST

/*
 * Inserts 2 words and check whether auto reload works.
 */
START_TEST (word_auto_reload)
{
	memo_word *w1, *w2;
	const char str1[] = "correct", str3[] = "incorrect", str2[] = "foobar";

	w1 = memo_word_new(db);
	memo_word_set_value(w1, str1);
	memo_word_save(w1);
	memo_word_set_value(w1, str3);
	fail_if(strcmp(str3, memo_word_get_value(w1)) != 0);

	w2 = memo_word_new(db);
	memo_word_set_value(w2, str2);
	memo_word_save(w2);

	/*
	 * memo_word_get_value should automatically update w1 as w2 has been
	 * inserted to the database.
	 */
	fail_if(strcmp(str1, memo_word_get_value(w1)) != 0);

	memo_word_free(w1);
	memo_word_free(w2);
}
END_TEST

/*
 * Check if pos/neg answers count changing functions work.
 */
START_TEST (word_answers_count_changing)
{
	const char str[] = "memo";
	const int pos = 15, neg =  21;
	memo_word *word;
	word = memo_word_new(db);
	memo_word_set_value(word, str);
	memo_word_set_positive_answers(word, pos);
	memo_word_set_negative_answers(word, neg);
	memo_word_save(word);
	memo_word_free(word);

	word = memo_database_find_word_by_value(db, str);
	fail_if(memo_word_get_positive_answers(word) != pos);
	fail_if(memo_word_get_negative_answers(word) != neg);
	memo_word_inc_positive_answers(word);
	memo_word_inc_negative_answers(word);
	memo_word_inc_negative_answers(word);
	fail_if(memo_word_update(word) != 0);
	memo_word_free(word);

	word = memo_database_find_word_by_value(db, str);
	fail_if(memo_word_get_positive_answers(word) != pos+1);
	fail_if(memo_word_get_negative_answers(word) != neg+2);
	memo_word_free(word);
}
END_TEST

/*
 * Inserts 2 words and checks whether translation insertion works.
 */
START_TEST (translation_creation)
{
	memo_word *w1, *w2;
	w1 = memo_word_new(db);
	w2 = memo_word_new(db);
	memo_word_set_value(w1, "Test");
	memo_word_set_value(w2, "tset");
	memo_word_save(w1);
	memo_word_save(w2);
	/* All functions used above should have been checked by now in a previous
	 * test. */
	fail_if(memo_word_add_translation(w1, w2) != 0, ERR_ADDING_TR);
	memo_word_free(w1);
	memo_word_free(w2);
	/* TODO: Should check whether the pair is really there. */
}
END_TEST

/*
 * Inserts 4 words and checks whether translation insertion allows duplicates.
 */
START_TEST (translation_inserting_duplicate)
{
	memo_word *w[4];
	int i;
	for (i = 0; i < 4; i++)
		w[i] = memo_word_new(db);
	memo_word_set_value(w[0], "one");
	memo_word_set_value(w[1], "two");
	memo_word_set_value(w[2], "three");
	memo_word_set_value(w[3], "four");
	for (i = 0; i < 4; i++)
		memo_word_save(w[i]);
	fail_if(memo_word_add_translation(w[0], w[1]) != 0, ERR_ADDING_TR);
	fail_if(memo_word_add_translation(w[1], w[2]) != 0, ERR_ADDING_TR);
	fail_if(memo_word_add_translation(w[0], w[3]) != 0, ERR_ADDING_TR);
	fail_if(memo_word_add_translation(w[0], w[3]) == 0, "Successfully "\
			"added a translation already existing in the database.");
	fail_if(memo_word_add_translation(w[2], w[1]) == 0, "Successfully "\
			"added a translation already existing in the database.");
	for (i = 0; i < 4; i++)
		memo_word_free(w[i]);
}
END_TEST

/*
 * Inserts 2 words and a translation and checks whether translation checking
 * words.
 */
START_TEST (translation_checking)
{
	memo_word *w1, *w2, *w_exists, *w_doesnt_exist;
	w1 = memo_word_new(db);
	w2 = memo_word_new(db);
	w_doesnt_exist = memo_word_new(db);
	w_exists = memo_word_new(db);
	memo_word_set_value(w1, "Test");
	memo_word_set_value(w2, "tset");
	memo_word_set_value(w_exists, "Exists in the database.");
	memo_word_set_value(w_doesnt_exist, "Does not exist in the db.");
	memo_word_save(w1);
	memo_word_save(w2);
	memo_word_save(w_exists);

	fail_if(memo_word_add_translation(w1, w2) != 0, ERR_ADDING_TR);

	fail_if(memo_word_check_translation(w1, w_exists) != 1,
		   "Found an pair which does not exist in the database");
	fail_if(memo_word_check_translation(w1, w_doesnt_exist) != 1,
		   "Found an item which does not exist in the database");

	fail_if(memo_word_check_translation(w1, w2) != 0,
		   "Cannot find a word which exists in the database.");
	fail_if(memo_word_check_translation(w2, w1) != 0,
		   "Cannot find a pair which exists in the database "\
		   "(in reverse order).");

	memo_word_free(w1);
	memo_word_free(w2);
}
END_TEST

/*
 * Inserts 3 words and a translation and checks whether list of translations
 * returned by memo_word_get_translations is correct.
 */
START_TEST (translation_fetching)
{
	memo_word *w1, *w2, *w3, **translations = 0;
	w1 = memo_word_new(db);
	w2 = memo_word_new(db);
	w3 = memo_word_new(db);
	memo_word_set_value(w1, "foo");
	memo_word_set_value(w2, "bar");
	memo_word_set_value(w3, "lol");
	memo_word_save(w1);
	memo_word_save(w2);
	memo_word_save(w3);

	fail_if(memo_word_add_translation(w1, w2) != 0, ERR_ADDING_TR);
	fail_if(memo_word_add_translation(w2, w3) != 0, ERR_ADDING_TR);

	fail_if(memo_word_get_translations(w2, &translations) != 2,
		   ERR_FETCHING_TR);
	fail_unless((!word_cmp(translations[0], w1) && !word_cmp(translations[1], w3)) ||
			(!word_cmp(translations[1], w1) && !word_cmp(translations[0], w3)),
			ERR_FETCHING_TR);

	memo_word_free(translations[0]);
	memo_word_free(translations[1]);
	free(translations);

	fail_if(memo_word_get_translations(w1, &translations) != 1,
		   ERR_FETCHING_TR);
	fail_unless(!word_cmp(translations[0], w2), ERR_FETCHING_TR);

	memo_word_free(translations[0]);
	free(translations);

	memo_word_free(w1);
	memo_word_free(w2);
	memo_word_free(w3);
}
END_TEST

/*
 * Inserts 3 words and a translation and check whether auto reload works.
 */
START_TEST (translation_auto_reload)
{
	memo_word *w1, *w2, *w3, *w1_copy, **transl;
	memo_translation *tr;
	const char str1[] = "correct", str3[] = "incorrect", str2[] = "foobar",
		  str4[] = "memo";

	w1 = memo_word_new(db);
	memo_word_set_value(w1, str1);
	memo_word_save(w1);

	w2 = memo_word_new(db);
	memo_word_set_value(w2, str2);
	memo_word_save(w2);

	/* Ensure that w1->db_last_change is up to date. */
	memo_word_reload(w1);
	memo_word_set_value(w1, str4);
	fail_if(strcmp(str4, memo_word_get_value(w1)) != 0);

	memo_word_add_translation(w1, w2);

	/*
	 * memo_word_get_value should automatically update w1 as a new translation
	 * has been inserted to the database.
	 */
	fail_if(strcmp(str1, memo_word_get_value(w1)) != 0);

	/*
	 * Remove all w1 translations from the memory. We expect that the next
	 * auto reload will get them back.
	 */
	w1_copy = memo_word_new(db);
	memo_word_copy(w1_copy, w1);
	tr = w1->translations;
	while (tr) {
		memo_translation *tmp = tr;
		tr = tr->next;
		free(tmp);
	}
	w1->translations = NULL;

	w3 = memo_word_new(db);
	memo_word_set_value(w3, str3);
	memo_word_save(w3);

	/*
	 * memo_word_get_translations should automatically update w1 as w3 has
	 * been inserted to the database.
	 */
	fail_if(memo_word_get_translations(w1, &transl) != 1, 0);
	free(transl);
	fail_if(word_cmp_transl(w1_copy, w1) != 0, 0);
	fail_if(word_cmp(w1_copy, w1) != 0, 0);

	memo_word_free(w1);
	memo_word_free(w2);
	memo_word_free(w3);
	memo_word_free(w1_copy);
}
END_TEST

/*
 * Checks a test. Firstly it adds 10 words to the database and creates 5
 * translations. Afterwards it opens a file with a reply to a fictional test
 * and checks the answers. Eventually we're checking whether words in the
 * database have been updated properly.
 */
START_TEST (messaging_checking_test)
{
	const int words = 10;
	int i, answers[words][2];
	memo_word *w[words];
	char value[11];
	FILE *f;

	for (i = 0; i < words; ++i) {
		w[i] = memo_word_new(db);
		sprintf(value, (i < words/2) ? "question%i" : "answer%i",
				i%(words/2));
		w[i]->positive_answers = answers[i][0] = i+10;
		w[i]->negative_answers = answers[i][1] = 20-i;
		fail_if(memo_word_set_value(w[i], value), 0);
		fail_if(memo_word_save(w[i]), 0);
	}
	for (i = 0; i < words/2; ++i)
		fail_if(memo_word_add_translation(w[i], w[i+(words/2)]), 0);

	fail_unless(f = fopen(TOP_SRCDIR"/test/reply1.eml", "r"));
	fail_if(memo_check_reply(f, db, 0));
	fclose(f);

	/* question0..4 */
	fail_if(memo_word_get_positive_answers(w[0]) != answers[0][0], 0);
	fail_if(memo_word_get_negative_answers(w[0]) != answers[0][1], 0);
	fail_if(memo_word_get_positive_answers(w[1]) != answers[1][0]+1, 0);
	fail_if(memo_word_get_negative_answers(w[1]) != answers[1][1], 0);
	fail_if(memo_word_get_positive_answers(w[2]) != answers[2][0], 0);
	fail_if(memo_word_get_negative_answers(w[2]) != answers[2][1]+1, 0);
	fail_if(memo_word_get_positive_answers(w[3]) != answers[3][0], 0);
	fail_if(memo_word_get_negative_answers(w[3]) != answers[3][1]+1, 0);
	fail_if(memo_word_get_positive_answers(w[4]) != answers[4][0], 0);
	fail_if(memo_word_get_negative_answers(w[4]) != answers[4][1], 0);
	/* answer0..4 */
	fail_if(memo_word_get_positive_answers(w[5]) != answers[5][0]+1, 0);
	fail_if(memo_word_get_negative_answers(w[5]) != answers[5][1], 0);
	fail_if(memo_word_get_positive_answers(w[6]) != answers[6][0], 0);
	fail_if(memo_word_get_negative_answers(w[6]) != answers[6][1], 0);
	fail_if(memo_word_get_positive_answers(w[7]) != answers[7][0], 0);
	fail_if(memo_word_get_negative_answers(w[7]) != answers[7][1], 0);
	fail_if(memo_word_get_positive_answers(w[8]) != answers[8][0], 0);
	fail_if(memo_word_get_negative_answers(w[8]) != answers[8][1]+1, 0);
	fail_if(memo_word_get_positive_answers(w[9]) != answers[9][0], 0);
	fail_if(memo_word_get_negative_answers(w[9]) != answers[9][1], 0);

	for (i = 0; i < words; ++i)
		memo_word_free(w[i]);
}
END_TEST

/*
 * Prepares the test suite.
 */
Suite *
database_suite (void) {
	Suite *s;
	TCase *tc_database, *tc_words, *tc_translations, *tc_messaging;

	s = suite_create("libmemo");

	tc_database = tcase_create("Database");
	tcase_add_test(tc_database, database_openclose);
	suite_add_tcase(s, tc_database);

	tc_words = tcase_create("Words");
	tcase_add_test(tc_words, word_inserting);
	tcase_add_test(tc_words, word_find_by_value);
	tcase_add_test(tc_words, word_get_answers_count);
	tcase_add_test(tc_words, word_get_key);
	tcase_add_test(tc_words, word_get_db);
	tcase_add_test(tc_words, word_reload);
	tcase_add_test(tc_words, word_delete);
	tcase_add_test(tc_words, word_update);
	tcase_add_test(tc_words, word_copy);
	tcase_add_test(tc_words, word_auto_reload);
	tcase_add_test(tc_words, word_answers_count_changing);
	tcase_add_checked_fixture (tc_words, database_setup, database_teardown);
	suite_add_tcase(s, tc_words);

	tc_translations = tcase_create("Translations");
	tcase_add_test(tc_translations, translation_creation);
	tcase_add_test(tc_translations, translation_inserting_duplicate);
	tcase_add_test(tc_translations, translation_checking);
	tcase_add_test(tc_translations, translation_fetching);
	tcase_add_test(tc_translations, translation_auto_reload);
	tcase_add_checked_fixture (tc_translations, database_setup,
			database_teardown);
	suite_add_tcase(s, tc_translations);

	tc_messaging = tcase_create("Messaging");
	tcase_add_test(tc_messaging, messaging_checking_test);
	tcase_add_checked_fixture (tc_messaging, database_setup,
			database_teardown);
	suite_add_tcase(s, tc_messaging);

	return s;
}

int
main (void) {
	int failed_count;
	Suite *s = database_suite();
	SRunner *sr = srunner_create(s);
	srunner_run_all(sr, CK_NORMAL);
	failed_count = srunner_ntests_failed(sr);
	srunner_free(sr);
	/* Remove the temporary database file if the tests have passed. */
	if (!failed_count)
		database_remove();
	return (failed_count == 0) ? 0 : 1;
}
