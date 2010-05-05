# -*- coding: UTF-8 -*-

from entities import *
import messaging
from test_helpers import *

class TestReceiving(DBSetupMixIn):
	def setup(self):
		self.small_db_setup()

	def test_parsing_a_tiny_reply(self):
		f = open("test/test_parsing_a_tiny_reply.eml")
		messaging.parse_reply(f)
		questions = Question.find(test_id=2)
		base_conds = [Question.pair_id == Pair.id]
		conds_norm = base_conds + [Question.inverted == False,
				Pair.first_phrase_id == Phrase.id]
		conds_inv = base_conds + [Question.inverted == True,
				Pair.second_phrase_id == Phrase.id]
		assert questions.find(*conds_norm +
				[Phrase.value==u'cebula']).one().result == True
		assert questions.find(*conds_norm +
				[Phrase.value==u'wąż']).one().result == False
		assert questions.find(*conds_inv +
				[Phrase.value==u'cebolla']).one().result == False
		assert questions.find(*conds_inv +
				[Phrase.value==u'serpiente']).one().result == True

	def test_parsing_a_reply_with_invalid_question(self):
		f = open("test/test_parsing_a_reply_with_invalid_question.eml")
		try:
			messaging.parse_reply(f)
		except messaging.IncorrectReply:
			pass
		else:
			assert False, "messaging.IncorrectReply wasn't thrown"
