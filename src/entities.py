# -*- coding: UTF-8 -*-

from storm.locals import *

def create_store(connection_string):
	database = create_database(connection_string)
	store = Store(database)
	for type in [Phrase, Language]:
		store.execute(type.table_schema)
	return store

class Phrase(object):
	__storm_table__ = 'phrases'

	id = Int(primary=True)
	value = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS phrases
			(id INTEGER PRIMARY KEY, value VARCHAR);'''

	def __init__(self, value):
		self.value = unicode(value)

	def __repr__(self):
		return "<Phrase('%s')>" % self.value

class Language(object):
	__storm_table__ = 'languages'

	id = Int(primary=True)
	name = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS languages
			(id INTEGER PRIMARY KEY, name VARCHAR);'''

	def __init__(self, name):
		self.name = unicode(name)

	def __repr__(self):
		return "<Language('%s')>" % self.name
