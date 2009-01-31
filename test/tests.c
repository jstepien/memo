#include <libmemo.h>
#include <check.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define DBNAME "./tmpdb"
#define ERR_LOAD "Cannot load a database in "DBNAME"."
#define ERR_CLOSE "Cannot close an opened database."
#define ERR_ADDING "Cannot add a word to the database."
#define ERR_ADDING_TR "Cannot add a translation to the database."

memo_database db;

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
	assert(memo_database_load(&db, DBNAME) == 0);
}

/*
 * Closes the database.
 */
void
database_teardown() {
	assert(memo_database_close(db) == 0);
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
			&& memo_word_get_db(a) == memo_word_get_db(b))
		return 0;
	return 1;
}

/*
 * Creates and closes a database file.
 */
START_TEST (database_openclose)
{
	memo_database db;
	fail_if(memo_database_load(&db, DBNAME) != 0, ERR_LOAD);
	fail_if(memo_database_close(db) != 0, ERR_CLOSE);
}
END_TEST

/*
 * Inserts a word to the database.
 */
START_TEST (database_word_inserting)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, "Failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	/* TODO: Should check whether the word has been saved. */
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word and checks if it can be found by it's key.
 */
START_TEST (database_word_find_by_key)
{
	memo_word *w1, *w2;
	w1 = memo_word_new(db);
	fail_if(w1 == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(w1, "Test") != 0, "Failed to set word's value.");
	fail_if(memo_word_save(w1) != 0, "Failed to save a word.");
	w2 = memo_word_find(db, memo_word_get_key(w1));
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
START_TEST (database_word_get_key)
{
	memo_word *word;
	int id;
	word = memo_word_new(db);
	fail_if(word == NULL, "can't create a new word.");
	fail_if(memo_word_set_value(word, "test") != 0, "failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "failed to save a word.");
	id = memo_word_get_key(word);
	memo_word_free(word);
	word = memo_word_find_by_value(db, "test");
	fail_if(id != memo_word_get_key(word), "Word's key does not match.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word, retrieves it and checks whether memo_word_get_db works
 * correctly.
 */
START_TEST (database_word_get_db)
{
	memo_word *word;
	memo_database d;
	int positive_answers, negative_answers;
	word = memo_word_new(db);
	fail_if(word == NULL, "can't create a new word.");
	fail_if(memo_word_set_value(word, "test") != 0, "failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "failed to save a word.");
	d = memo_word_get_db(word);
	memo_word_free(word);
	word = memo_word_find_by_value(db, "test");
	fail_if(d != memo_word_get_db(word), "DB does not match.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word, retrieves it and checks whether
 * memo_word_get_positive_answers and memo_word_get_negative_answers work
 * correctly.
 */
START_TEST (database_word_get_answers_count)
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
	word = memo_word_find_by_value(db, "test");
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
START_TEST (database_word_find_by_value)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, "Failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	memo_word_free(word);
	word = memo_word_find_by_value(db, "Test");
	fail_if(word == NULL, "Failed to find the inserted word.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word and checks whether reloading works.
 */
START_TEST (database_word_reload)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, "Failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	fail_if(memo_word_reload(word) != 0, "Failed to reload a saved word.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts a word, deletes it and checks whether it can be found or reloaded.
 */
START_TEST (database_word_delete)
{
	memo_word *word;
	word = memo_word_new(db);
	fail_if(word == NULL, "Can't create a new word.");
	fail_if(memo_word_set_value(word, "Test") != 0, "Failed to set word's value.");
	fail_if(memo_word_save(word) != 0, "Failed to save a word.");
	fail_if(memo_word_delete(word) != 0, "Failed to delete a saved word.");
	fail_if(memo_word_reload(word) == 0, "Reloaded a deleted word.");
	memo_word_free(word);
}
END_TEST

/*
 * Inserts 2 words and checks whether translation insertion works.
 */
START_TEST (database_translation_creation)
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
START_TEST (database_inserting_duplicate_translation)
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
START_TEST (database_checking_translations)
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
 * Prepares the test suite.
 */
Suite *
database_suite (void) {
	Suite *s;
	TCase *tc_openclose, *tc_basic_io;

	s = suite_create("Database");

	tc_openclose = tcase_create("Open/Close");
	tcase_add_test(tc_openclose, database_openclose);
	suite_add_tcase(s, tc_openclose);

	tc_basic_io = tcase_create("Word I/O");
	tcase_add_test(tc_basic_io, database_word_inserting);
	tcase_add_test(tc_basic_io, database_word_find_by_value);
	tcase_add_test(tc_basic_io, database_word_get_answers_count);
	tcase_add_test(tc_basic_io, database_word_get_key);
	tcase_add_test(tc_basic_io, database_word_get_db);
	tcase_add_test(tc_basic_io, database_word_reload);
	tcase_add_test(tc_basic_io, database_word_delete);
	tcase_add_test(tc_basic_io, database_translation_creation);
	tcase_add_test(tc_basic_io, database_inserting_duplicate_translation);
	tcase_add_test(tc_basic_io, database_checking_translations);
	tcase_add_checked_fixture (tc_basic_io, database_setup, database_teardown);
	suite_add_tcase(s, tc_basic_io);

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
	return (failed_count == 0) ? 0 : 1;
}
