#!/bin/sh

./basic_database_test ./tmpdb
if [[ $? -ne 0 ]]; then
	exit 1
fi
./basic_database_test ./tmpdir/tmpdb
if [[ $? -eq 0 ]]; then
	exit 1
fi
rm -f ./tmpdb
exit 0
