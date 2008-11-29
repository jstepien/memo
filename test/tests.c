#include <libmemo.h>
#include <check.h>
#include <assert.h>

#define DBNAME "./tmpdb"
#define ERR_LOAD "Cannot load a database in "DBNAME"."
#define ERR_CLOSE "Cannot close an opened database."
#define ERR_ADDING "Cannot add a word to the database."

memo_database db;

void
database_setup() {
	assert(memo_database_load(&db, DBNAME) == 0);
	/* TODO: truncate the database. */
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

START_TEST (database_basic_io_add_get)
{
	fail_if(memo_database_add_word(db, "Test", "tset") != 0, ERR_ADDING);
	fail_if(memo_database_check_translation(db, "Test", "doesn't exist") != 1,
		   "Found an item which does not exist in the database");
	fail_if(memo_database_check_translation(db, "Test", "tset") != 0,
		   "Cannot find a word which exists in the database.");
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
	tcase_add_test(tc_basic_io, database_basic_io_add_get);
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
