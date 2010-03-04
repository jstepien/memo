# -*- coding: UTF-8 -*-

from datetime import datetime
from activerecord import ActiveRecord
from storm.locals import *

def connect(connection_string):
	ActiveRecord.connect(connection_string)

class Phrase(ActiveRecord):
	__storm_table__ = 'phrases'

	id = Int(primary=True)
	value = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS phrases
			(id INTEGER PRIMARY KEY, value VARCHAR NOT NULL, UNIQUE (value));'''

	def __init__(self, value):
		self.value = unicode(value)

	def __repr__(self):
		return "<Phrase('%s')>" % self.value.encode('utf-8')

class Language(ActiveRecord):
	__storm_table__ = 'languages'

	id = Int(primary=True)
	name = Unicode()

	table_schema = '''CREATE TABLE IF NOT EXISTS languages
			(id INTEGER PRIMARY KEY, name VARCHAR NOT NULL, UNIQUE (name));'''

	def __init__(self, name):
		self.name = unicode(name)

	def __repr__(self):
		return "<Language('%s')>" % self.name.encode('utf-8')

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
			first_phrase_id INTEGER NOT NULL, first_language_id INTEGER NOT
			NULL, second_phrase_id INTEGER NOT NULL, second_language_id INTEGER
			NOT NULL, UNIQUE (first_phrase_id, first_language_id,
			second_phrase_id, second_language_id));''',
			'''CREATE TRIGGER IF NOT EXISTS pairs_before_insert
			BEFORE INSERT ON pairs FOR EACH ROW WHEN EXISTS (SELECT id
			FROM pairs WHERE NEW.second_language_id = pairs.first_language_id
			AND NEW.second_phrase_id = pairs.first_phrase_id)
			BEGIN SELECT RAISE (FAIL, 'columns first_phrase_id, '''
			'''first_language_id, second_phrase_id, second_language_id are '''
			'''not unique') END; END;''']

	def __init__(self, phrase1, lang1, phrase2, lang2):
		self.first_phrase = phrase1
		self.first_language = lang1
		self.second_phrase = phrase2
		self.second_language = lang2

	def __repr__(self):
		return "<Pair(%s in %s, %s in %s)>" % (self.first_phrase,
				self.first_language, self.second_phrase, self.second_language)

class Test(ActiveRecord):
	__storm_table__ = 'tests'

	id = Int(primary=True)
	ctime = DateTime()

	table_schema = '''CREATE TABLE IF NOT EXISTS tests
			(id INTEGER PRIMARY KEY, ctime DATETIME NOT NULL);'''

	def __init__(self):
		self.ctime = datetime.now()

	def __repr__(self):
		return "<Test('%s')>" % self.id

class Question(ActiveRecord):
	__storm_table__ = 'questions'

	id = Int(primary=True)
	pair_id = Int()
	test_id = Int()
	inverted = Bool()
	# True = correct, False = incorrect, None = not yet given.
	result = Bool()

	pair = Reference(pair_id, Pair.id)
	test = Reference(test_id, Test.id)

	table_schema = '''CREATE TABLE IF NOT EXISTS questions
			(id INTEGER PRIMARY KEY, pair_id INTEGER, test_id INTEGER NOT NULL,
			inverted BOOL NOT NULL, result BOOL,
			UNIQUE (pair_id, test_id, inverted));'''

	def __init__(self, test, pair, inverted=False):
		self.test = test
		self.pair = pair
		self.inverted = inverted

	def __repr__(self):
		return "<Question(%s, %s, %s)>" % (self.test, self.pair, self.inverted)

	def question(self):
		return self.pair.first_phrase if not self.inverted else \
			self.pair.second_phrase

	def answer(self):
		return self.pair.first_phrase if self.inverted else \
			self.pair.second_phrase

Test.questions = ReferenceSet(Test.id, Question.test_id)

for cls in [Phrase, Language, Pair, Test, Question]:
	ActiveRecord.register_subclass(cls)
