# -*- coding: UTF-8 -*-

from entities import *

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

class TestLanguage:
	def setup(self):
		connect_to_sqlite_memory_store()

	def teardown(self):
		pass

	def test_adding_a_unicode_language_name(self):
		Language(u'slovenščina').save()
		language = Language.find().one()
		assert u'slovenščina' ==language.name
