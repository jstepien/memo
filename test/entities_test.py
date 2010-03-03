# -*- coding: UTF-8 -*-

from datetime import datetime
from entities import *
from activerecord import *

def connect_to_sqlite_memory_store():
	connect("sqlite://:memory:")

def should_raise_on_save(object, exclass):
	try:
		object.save()
		ActiveRecord.commit()
	except exclass:
		pass
	else:
		assert False, exclass + " wasn't raised"

class SmallDBSetupMixIn():
	'''A mixin which adds a method which connects to the database and adds two
	languages, four words and two pairs.'''
	def small_db_setup(self):
		connect_to_sqlite_memory_store()
		self.langs = [Language(x) for x in (u'polski', u'español')]
		self.phrases = [Phrase(x) for x in
				(u'wąż', u'serpiente', u'komputer', u'ordenador')]
		self._create_pairs()
		for obj in self.langs + self.phrases + self.pairs:
			obj.save()
		ActiveRecord.commit()

	def _create_pairs(self):
		l, p = self.langs, self.phrases
		self.pairs = [Pair(arr[0], arr[1], arr[2], arr[3]) for arr in
				( [p[0], l[0], p[1], l[1]], [p[2], l[0], p[3], l[1]] ) ]

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
		should_raise_on_save(Phrase(u'źdźbło'), NonUniqueColumnError)
		assert Phrase.find().count() == 1

class TestLanguage:
	def setup(self):
		connect_to_sqlite_memory_store()

	def teardown(self):
		pass

	def test_adding_a_unicode_language_name(self):
		Language(u'slovenščina').save()
		language = Language.find().one()
		assert u'slovenščina' == language.name

	def test_adding_a_unicode_language_name_twice(self):
		Language(u'slovenščina').save()
		should_raise_on_save(Language(u'slovenščina'), NonUniqueColumnError)
		assert Language.find().count() == 1

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
		should_raise_on_save(Pair(self.p1, self.l1, self.p2, self.l2),
				NonUniqueColumnError)
		assert Pair.find().count() == 1

	def test_adding_a_pair_twice_and_swapped(self):
		Pair(self.p1, self.l1, self.p2, self.l2).save()
		should_raise_on_save(Pair(self.p2, self.l2, self.p1, self.l1),
				NonUniqueColumnError)
		assert Pair.find().count() == 1

	def test_adding_a_pair_with_the_same_phrase(self):
		Pair(self.p1, self.l1, self.p1, self.l2).save()
		assert Pair.find().count() == 1

	def test_adding_a_pair_with_the_same_language(self):
		Pair(self.p1, self.l1, self.p2, self.l1).save()
		assert Pair.find().count() == 1

class TestTest(SmallDBSetupMixIn):
	def setup(self):
		self.small_db_setup()

	def test_adding(self):
		then = datetime.now()
		Test().save()
		assert Test.find().count() == 1
		assert then <= Test.find().one().ctime <= datetime.now()

	def test_adding_with_questions(self):
		t = Test()
		t.save()
		for p in self.pairs:
			Question(t, p).save()
		assert Test.find().one().questions.count() == len(self.pairs)

class TestQuestion(SmallDBSetupMixIn):
	def setup(self):
		self.small_db_setup()
		self.tests = [Test(), Test()]

	def test_adding_a_question_without_a_test(self):
		should_raise_on_save(Question(None, self.pairs[0]),
				NullException)
		assert Question.find().count() == 0

	def test_adding_a_question(self):
		Question(self.tests[0], self.pairs[0])
		assert Question.find().count() == 1
		q = Question.find().one()
		assert q.test is self.tests[0]
		assert q.pair is self.pairs[0]
		assert q.answer is None

	def test_adding_a_question_inverted(self):
		Question(self.tests[0], self.pairs[0], inverted=True)
		assert Question.find().count() == 1

	def test_adding_a_question_multiple_times(self):
		Question(self.tests[0], self.pairs[0]).save()
		should_raise_on_save(Question(self.tests[0], self.pairs[0]),
				NonUniqueColumnError)
		assert Question.find().count() == 1
