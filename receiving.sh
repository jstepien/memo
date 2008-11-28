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

# Reads an e-mail contents from stdin, analyses the answers updating the
# database and sends a reply with a report.
function receive_email {
	# Contents of the report.
	reply=""
	# The file in which the report will be stored.
	tmpfile="$MEMO_WORKDIR/tmp-$MEMO_TIMESTAMP-$$"
	# Read from stdin until EOF, one line per one iteration.
	while read line
	do
		# If the line looks like a reply to a test Memo has sent, i.e.
		#
		# > something = something else
		#
		# begin parsing.
		if [[ $( echo $line | grep "^ *> *.*=.*$" -c ) -eq 1  ]]
		then
			# Remove " > " including spaces around it, substitute " = " with
			# "|", spaces with underscores and afterwards "|" with spaces.
			# In other words, prepare the line for awk's "print $#" command.
			tmp=$( echo $line | sed -e "s/^\ *>\ *//" -e "s/\ *=\ */|/" \
				-e "s/\ /_/g" -e "s/|/\ /g" )
			# The word before the equation character.
			word=$( echo $tmp | awk '{ print $1 }' | sed -e "s/_/\ /g" )
			# The word after the equation character.
			ans=$( echo $tmp | awk '{ print $2 }' | sed -e "s/_/\ /g" )
			# Get the correct answer's line from the database.
			tmp=$( grep "^$word|" $MEMO_DBFILE \
				| sed -e "s/\ /_/g" -e "s/|/\ /g" )
			# Get the correct answer.
			correct=$( echo $tmp | awk '{ print $2 }' | sed -e "s/_/\ /g" )
			# Get correct answer's positive and negative results count.
			positive=$( echo $tmp | awk '{ print $3 }' | sed -e "s/_/\ /g" )
			negative=$( echo $tmp | awk '{ print $4 }' | sed -e "s/_/\ /g" )
			# Initialise new positive and negative results counting variables.
			newpositive="$positive"
			newnegative="$negative"
			# Add to the reply a line containing a report about what was the
			# answer and - in case of incorrect answer - what has been expected.
			reply="${reply}Q: '${word}' A: '${ans}'"
			if [[ $( echo "$correct" | grep "$ans" -c ) -eq 1 ]]
			then
				let newpositive="$positive+1"
				reply="${reply}, correct."
			else
				let newnegative="$negative+1"
				reply="${reply}, incorrect, expected '${correct}'"
			fi
			reply="${reply}\n"
			# Update the database with new positive and negative results
			# count.
			sed -i $MEMO_DBFILE -e "s/^\($word|[^|]*\)|$positive|$negative$/\1|$newpositive|$newnegative/"
		fi
	done
	# Save the value of the reply in the temporary file, send it and remove
	# the file.
	echo -e ${reply} > "$tmpfile" || exit 1
	send_email "$tmpfile" || exit 1
	rm "$tmpfile" || exit 1
	# Done.
	return 0
}
