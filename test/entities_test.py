# -*- coding: UTF-8 -*-

from entities import *
from activerecord import *

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
		except NonUniqueColumnError:
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
		except NonUniqueColumnError:
			assert Phrase.find().count() == 1
		else:
			assert False, "Inserted a non-unique phrase!"

class TestPair:
	def setup(self):
		connect_to_sqlite_memory_store()
		self.l1 = Language(u'polski')
		self.l2 = Language(u'español')
		self.p1 = Phrase(u'wąż')
		self.p2 = Phrase(u'serpiente')
		for obj in [self.l1, self.l2, self.p1, self.p2]:
			obj.save()
		ActiveRecord.commit()

	def teardown(self):
		pass

	def test_adding_a_pair(self):
		Pair(self.p1, self.l1, self.p2, self.l2).save()
		pair = Pair.find().one()
		assert pair.first_language is self.l1
		assert pair.second_language is self.l2
		assert pair.first_phrase is self.p1
		assert pair.second_phrase is self.p2

	def test_adding_a_pair_twice(self):
		Pair(self.p1, self.l1, self.p2, self.l2).save()
		Pair(self.p1, self.l1, self.p2, self.l2).save()
		try:
			Pair.commit()
		except NonUniqueColumnError:
			assert Pair.find().count() == 1
		else:
			assert False, "Inserted a non-unique pair!"

	def test_adding_a_pair_twice_and_swapped(self):
		Pair(self.p1, self.l1, self.p2, self.l2).save()
		Pair(self.p2, self.l2, self.p1, self.l1).save()
		try:
			Pair.commit()
		except NonUniqueColumnError:
			assert Pair.find().count() == 1
		else:
			assert False, "Inserted a non-unique pair!"
