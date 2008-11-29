#include <libmemo.h>
#include <check.h>

START_TEST (database_open_close)
{
	memo_database db;
	fail_if(memo_database_load(&db, "./tmpdb") != 0,
			"Can't create a database in ./tmpdb");
	fail_if(memo_database_close(db) != 0,
			"Can't close an opened database");
}
END_TEST

Suite *
database_suite (void) {
	Suite *s = suite_create("Database");

	TCase *tc_database = tcase_create("Basic operations");
	tcase_add_test(tc_database, database_open_close);
	suite_add_tcase(s, tc_database);

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
