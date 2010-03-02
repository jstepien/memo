# -*- coding: UTF-8 -*-

from entities import *

def sqlite_memory_store():
	return create_store('sqlite:///:memory:')

class TestPhrase:
	def setup(self):
		self.store = sqlite_memory_store()

	def teardown(self):
		self.store = None

	def test_adding_a_unicode_word(self):
		phrase = Phrase(u'źdźbło')
		self.store.add(phrase)
		added_phrase = self.store.find(Phrase).one()
		assert u'źdźbło' == added_phrase.value

class TestLanguage:
	def setup(self):
		self.store = sqlite_memory_store()

	def teardown(self):
		self.store = None

	def test_adding_a_unicode_language_name(self):
		language = Language(u'slovenščina')
		self.store.add(language)
		added_language = self.store.find(Language).one()
		assert u'slovenščina' == added_language.name
