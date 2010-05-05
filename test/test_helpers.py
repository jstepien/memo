# -*- coding: UTF-8 -*-

from entities import *
from activerecord import *
import tutor

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

class DBSetupMixIn():
	'''A mixin which adds a methods which connect to the database and add some
	valuaes for tests.'''

	def tiny_db_setup(self):
		'''Adds two languages, four words and two pairs.'''
		connect_to_sqlite_memory_store()
		self.langs = [Language(x) for x in (u'polski', u'español')]
		self.phrases = [Phrase(x) for x in
				(u'wąż', u'serpiente', u'komputer', u'ordenador')]
		self._create_pairs()
		for obj in self.langs + self.phrases + self.pairs:
			obj.save()
		ActiveRecord.commit()

	def small_db_setup(self):
		'''Adds two languages, eight words, and two tests four pairs, of which
		one has been completed.'''
		connect_to_sqlite_memory_store()
		self.langs = [Language(x) for x in (u'polski', u'español')]
		self.phrases = [Phrase(x) for x in (
			u'wąż', u'serpiente', u'komputer', u'ordenador',
			u'cebula', u'cebolla', u'iść', u'ir')]
		self._create_pairs()
		tutor.QuestionsPerTest = 8
		for obj in self.langs + self.phrases + self.pairs:
			obj.save()
		self.tests = [self._create_completed_test(),
			self._create_uncompleted_test()]
		ActiveRecord.commit()

	def _create_pairs(self):
		self.pairs = []
		for i in range(len(self.phrases) / 2):
			self.pairs.append(Pair(self.phrases[2*i], self.langs[0],
				self.phrases[2*i+1], self.langs[1]))

	def _create_completed_test(self):
		t = tutor.prepare_test()
		for q in t.questions:
			q.result = 1
		t.save()

	def _create_uncompleted_test(self):
		t = tutor.prepare_test()
