# -*- coding: UTF-8 -*-

import sys, os, re
from entities import *
from storm.locals import Select # remove this dependency

subject = "Your memo test"

_mail_command = 'mailx "%%s" -s "%s"' % subject

def send_test(test, addr):
	questions = sorted(test.questions, key=_question_keygen)
	prev_key = None
	message = u"Test ID: %i\n" % test.id
	for q in questions:
		p = q.pair
		if prev_key != _question_keygen(q):
			prev_key = _question_keygen(q)
			if q.inverted:
				langs = (p.second_language, p.first_language)
			else:
				langs = (p.first_language, p.second_language)
			message += u"%s → %s:\n" % (langs[0].name, langs[1].name)
		phrase = p.second_phrase.value if q.inverted else p.first_phrase.value
		message += u"  %s = \n" % phrase
	_pipe_to_mail_command(message, addr)

def parse_reply(file):
	langs = test = questions_pairs = None
	for line in file:
		line = line.decode('utf-8')
		match = re.match(u'^>\s*Test ID: (\d+)', line)
		if match:
			test = Test.find(id=int(match.groups()[0])).one()
			questions_pairs = Pair.find(Pair.id.is_in(
				Select(Question.pair_id, Question.test_id==test.id)))
			continue
		match = re.match(u'^>\s*(.+) → (.+):$', line)
		if match:
			langs = [Language.find(name=match.groups()[i]).one()
					for i in (0, 1)]
			continue
		match = re.match(u'^>\s*(.+) =\s*(.+)', line)
		if match:
			phrases = [Phrase.find(value=match.groups()[i]).one()
					for i in (0, 1)]
			if phrases[0] is None:
				raise IncorrectReply("Found a phrase which isn't in the " +
						"database")
			pair = questions_pairs.find(Pair.first_phrase == phrases[0],
					Pair.first_language == langs[0],
					Pair.second_language == langs[1]).one()
			if pair is not None:
				_check_answer(test.questions.find(Question.inverted == False,
					Question.pair == pair, ).one(), phrases[1])
			else:
				pair = questions_pairs.find(Pair.second_phrase==phrases[0],
						Pair.first_language == langs[1],
						Pair.second_language == langs[0]).one()
				if pair is None:
					raise IncorrectReply("Found a phrase which wasn't in the " +
							"test number %i" % test.id)
				_check_answer(test.questions.find(Question.inverted == True,
					Question.pair == pair, ).one(), phrases[1])
			continue
		if test != None and re.match(u"^>*\s*$", line):
			break
		if test != None: raise Exception("no match: '''%s'''" %
				line.encode('utf-8').rstrip())

def _check_answer(question, answer):
	p = question.pair
	correct_answer = p.first_phrase if question.inverted else p.second_phrase
	question.result = True if answer is correct_answer else False
	question.save()

def _question_keygen(q):
	if q.inverted:
		return (q.pair.first_language_id, q.pair.second_language_id)
	else:
		return (q.pair.second_language_id, q.pair.first_language_id)

def _pipe_to_mail_command(str, addr):
	pipe = os.popen(_mail_command % addr, 'w')
	pipe.write(str.encode('utf-8'))
	pipe.close()

class IncorrectReply(Exception):
	pass
