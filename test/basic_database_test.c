#include <libmemo.h>
#include <stdio.h>
#include <string.h>

int
main (int argc, char const* argv[])
{
	memo_database db;
	if ( argc != 2 || strlen(argv[1]) < 1 ) {
		fprintf(stderr, "Usage: %s filename\n", argv[0]);
		return 1;
	}
	if ( memo_database_load(&db, argv[1]) != 0 )
		return 1;
	if ( memo_database_close(db) != 0 )
		return 1;
	return 0;
}
