#!/bin/bash

# Copyright (C) 2008 Jan Stępień
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

USAGE="$0: usage: $0 [-s|-r]"

function usage {
	echo "$USAGE"
}

function send {
	MEMO_TMPFILE="$MEMO_WORKDIR/tmp-$MEMO_TIMESTAMP-$$"
	prepare_email "$MEMO_TMPFILE" || exit 1
	send_email "$MEMO_TMPFILE" || exit 1
	rm "$MEMO_TMPFILE" || exit 1
	exit 0
}

function receive {
	receive_email || exit 1
}

function main {
	if [[ $# -ne 1 ]]
	then
		echo $USAGE
		return 1
	fi
	case $1 in
		"-s")	send;;
		"-r")	receive;;
		*)	usage; exit 1;;
	esac
}
