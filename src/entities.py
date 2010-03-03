# -*- coding: UTF-8 -*-

from activerecord import ActiveRecord
from storm.locals import *

def connect(connection_string):
	ActiveRecord.connect(connection_string)

class Phrase(ActiveRecord):
	__storm_table__ = 'phrases'

	id = Int(primary=True)
	value = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS phrases
			(id INTEGER PRIMARY KEY, value VARCHAR, UNIQUE (value));'''

	def __init__(self, value):
		self.value = unicode(value)

	def __repr__(self):
		return "<Phrase('%s')>" % self.value

class Language(ActiveRecord):
	__storm_table__ = 'languages'

	id = Int(primary=True)
	name = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS languages
			(id INTEGER PRIMARY KEY, name VARCHAR, UNIQUE (name));'''

	def __init__(self, name):
		self.name = unicode(name)

	def __repr__(self):
		return "<Language('%s')>" % self.name

class Pair(ActiveRecord):
	__storm_table__ = 'pairs'

	id = Int(primary=True)
	first_phrase_id = Int()
	first_language_id = Int()
	second_phrase_id = Int()
	second_language_id = Int()

	first_phrase = Reference(first_phrase_id, Phrase.id)
	first_language = Reference(first_language_id, Language.id)
	second_phrase = Reference(second_phrase_id, Phrase.id)
	second_language = Reference(second_language_id, Language.id)

	table_schema = ['''CREATE TABLE IF NOT EXISTS pairs (id INTEGER PRIMARY KEY,
			first_phrase_id INTEGER, first_language_id INTEGER,
			second_phrase_id INTEGER, second_language_id INTEGER,
			UNIQUE (first_phrase_id, first_language_id, second_phrase_id,
			second_language_id));''',
			'''CREATE TRIGGER pairs_before_insert BEFORE INSERT ON pairs
			FOR EACH ROW WHEN EXISTS (SELECT id FROM pairs WHERE
			NEW.second_language_id = pairs.first_language_id AND
			NEW.second_phrase_id = pairs.first_phrase_id)
			BEGIN SELECT RAISE (FAIL, 'columns first_phrase_id, '''
			'''first_language_id, second_phrase_id, second_language_id are '''
			'''not unique') END; END;''']

	def __init__(self, phrase1, lang1, phrase2, lang2):
		self.first_phrase = phrase1
		self.first_language = lang1
		self.second_phrase = phrase2
		self.second_language = lang2

for cls in [Phrase, Language, Pair]:
	ActiveRecord.register_subclass(cls)
