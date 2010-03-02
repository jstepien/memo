# -*- coding: UTF-8 -*-

from activerecord import ActiveRecord
from storm.locals import *

def connect(connection_string):
	ActiveRecord.connect(connection_string)

class Phrase(object, ActiveRecord):
	__storm_table__ = 'phrases'

	id = Int(primary=True)
	value = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS phrases
			(id INTEGER PRIMARY KEY, value VARCHAR, UNIQUE (value));'''

	def __init__(self, value):
		self.value = unicode(value)

	def __repr__(self):
		return "<Phrase('%s')>" % self.value

class Language(object, ActiveRecord):
	__storm_table__ = 'languages'

	id = Int(primary=True)
	name = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS languages
			(id INTEGER PRIMARY KEY, name VARCHAR, UNIQUE (name));'''

	def __init__(self, name):
		self.name = unicode(name)

	def __repr__(self):
		return "<Language('%s')>" % self.name

for cls in [Phrase, Language]:
	ActiveRecord.register_subclass(cls)
