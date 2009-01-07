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

type mailx 2>&1 > /dev/null
if [[ $? -ne 0 ]]
then
	error "$0: can't find mailx"
	exit 1
fi

FOOTER="--
memo $MEMO_VERSION
"

function prepare_email {
	touch $1
	if [[ $? -ne 0 ]]
	then
		error "$0: prepare_email: cant write to tmp file $1"
		return 1
	fi
	cat $MEMO_DBFILE \
		| grep -v "^#.*" \
		| sed -e "s/\ /_/g" -e "s/|/\ /g" \
		| awk '{ print $3" "$1}' \
		| sort -g \
		| head -n 20 \
		| awk '{ print $2 }' \
		| sed -e "s/_/\ /g" -e "s/$/\ =\ /g" \
		> $1
	echo "$FOOTER" >> $1
	return 0
}

function send_email {
	if [[ ! -f $1 ]]
	then
		error "$0: prepare_email: cant write to tmp file $1"
		return 1
	fi
	cat $1 | mailx "$MEMO_TO" \
		-s "memo ($MEMO_TIMESTAMP)" \
		-a "X-Mailer: memo $MEMO_VERSION" \
		-a "Content-Type: text/plain; charset=UTF-8" \
		-a "Content-Transfer-Encoding: 8bit" \
		 || return 1
	return 0
}
