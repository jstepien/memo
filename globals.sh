#!/bin/bash

# Copyright (C) 2008, 2009 Jan Stępień
#
# This file is part of Memo.
# 
# Memo is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# Memo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Memo.  If not, see <http://www.gnu.org/licenses/>.

# Some useful default values.
if [[ $MEMO_INCLUDED_GLOBALS -ne 1 ]]
then
	WORKDIR="$HOME/.memo"
	DBFILE="$WORKDIR/db"
	TO="$USER@$HOSTNAME"
	FROM="$USER@$HOSTNAME"
	MEMO_TIMESTAMP="$( date +%s )"
	MEMO_INCLUDED_GLOBALS=1
fi

# If the first argument != empty string the function prints it to stderr.
# Returns 0 on success and other values on failure.
function error {
	if [[ -z $1 ]]
	then
		return 1
	fi
	echo $1 >&2
	return 0
}

# Checks for ~/.memo/memo.rc file (or equivalent). If it doesn't exists,
# it's created. If the parent directory doesn't exist it also gets created.
# Returns 0 on success and other values on failure.
function rc_file_check {
	if [[ -f "$MEMO_WORKDIR/memo.rc" ]]; then
		return 0
	else
		if [[ ! -d "$MEMO_WORKDIR" ]]; then
			mkdir -p "$MEMO_WORKDIR"
			if [[ $? -ne 0 ]]; then
				error "Can't create '$MEMO_WORKDIR'."
				return 1
			fi
		fi
		echo -e "MEMO_FROM=\"$FROM\"\nMEMO_TO=\"$TO\"" > \
			"$MEMO_WORKDIR/memo.rc"
		if [[ $? -ne 0 ]]; then
			error "Can't write to '$MEMO_WORKDIR/memo.rc'."
			return 1
		fi
	fi
	return 0
}
