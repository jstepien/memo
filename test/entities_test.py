# -*- coding: UTF-8 -*-

from entities import *
from activerecord import NonUniqueColumnValueError

def connect_to_sqlite_memory_store():
	connect("sqlite://:memory:")

class TestPhrase:
	def setup(self):
		connect_to_sqlite_memory_store()

	def teardown(self):
		pass

	def test_adding_a_unicode_word(self):
		Phrase(u'źdźbło').save()
		phrase = Phrase.find().one()
		assert u'źdźbło' == phrase.value

	def test_adding_a_unicode_word_twice(self):
		Phrase(u'źdźbło').save()
		Phrase(u'źdźbło').save()
		try:
			Phrase.commit()
		except NonUniqueColumnValueError:
			assert Phrase.find().count() == 1
		else:
			assert False, "Inserted a non-unique phrase!"

class TestLanguage:
	def setup(self):
		connect_to_sqlite_memory_store()

	def teardown(self):
		pass

	def test_adding_a_unicode_language_name(self):
		Language(u'slovenščina').save()
		language = Language.find().one()
		assert u'slovenščina' ==language.name

	def test_adding_a_unicode_language_name_twice(self):
		Phrase(u'slovenščina').save()
		Phrase(u'slovenščina').save()
		try:
			Phrase.commit()
		except NonUniqueColumnValueError:
			assert Phrase.find().count() == 1
		else:
			assert False, "Inserted a non-unique phrase!"
