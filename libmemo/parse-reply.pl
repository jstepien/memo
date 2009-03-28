#!/usr/bin/env perl
# Copyright (C) 2009 Jan Stępień
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

use warnings;
use strict;
use encoding 'utf8';
while (<STDIN>) {
	if (/^> ([\d\w ]+)\s+=\s+([\d\w ]*)$/) {
		print "$1\n$2\n";
	}
}

# vim:ts=4:noet:tw=78
