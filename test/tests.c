#include <libmemo.h>
#include <check.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>

#define DBNAME "./tmpdb"
#define ERR_LOAD "Cannot load a database in "DBNAME"."
#define ERR_CLOSE "Cannot close an opened database."
#define ERR_ADDING "Cannot add a word to the database."

memo_database db;

void
database_remove() {
	/* Remove the database file. It's fine if it does not exist. */
	assert(unlink(DBNAME) == 0 && errno != ENOENT);
}

void
database_setup() {
	database_remove();
	assert(memo_database_load(&db, DBNAME) == 0);
	assert(memo_database_init(db) == 0);
}

void
database_teardown() {
	assert(memo_database_close(db) == 0);
}

START_TEST (database_openclose)
{
	memo_database db;
	fail_if(memo_database_load(&db, DBNAME) != 0, ERR_LOAD);
	fail_if(memo_database_close(db) != 0, ERR_CLOSE);
}
END_TEST

START_TEST (database_insertion)
{
	fail_if(memo_database_add_word(db, "Test", "tset") != 0, ERR_ADDING);
	/* Should check whether the pair is really there. */
}
END_TEST

START_TEST (database_checking_translations)
{
	fail_if(memo_database_add_word(db, "Test", "tset") != 0, ERR_ADDING);
	fail_if(memo_database_check_translation(db, "Test", "doesn't exist") != 1,
		   "Found an item which does not exist in the database");
	fail_if(memo_database_check_translation(db, "foobar", "tset") != 1,
		   "Found an item which does not exist in the database");
	fail_if(memo_database_check_translation(db, "Test", "tset") != 0,
		   "Cannot find a word which exists in the database.");
	fail_if(memo_database_check_translation(db, "tset", "Test") != 0,
		   "Cannot find a pair which exists in the database "\
		   "(in reverse order).");
}
END_TEST

Suite *
database_suite (void) {
	Suite *s;
	TCase *tc_openclose, *tc_basic_io;

	s = suite_create("Database");

	tc_openclose = tcase_create("Open/Close");
	tcase_add_test(tc_openclose, database_openclose);
	suite_add_tcase(s, tc_openclose);

	tc_basic_io = tcase_create("Basic I/O");
	tcase_add_test(tc_basic_io, database_insertion);
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
