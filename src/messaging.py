# -*- coding: UTF-8 -*-

import sys

def send_test(test):
	questions = sorted(test.questions, key=_question_keygen)
	prev_key = None
	for q in questions:
		p = q.pair
		if prev_key != _question_keygen(q):
			prev_key = _question_keygen(q)
			if q.inverted:
				langs = (p.first_language, p.second_language)
			else:
				langs = (p.second_language, p.first_language)
			print u"%s → %s:" % (langs[0].name, langs[1].name)
		phrase = p.first_phrase.value if q.inverted else p.second_phrase.value
		print u"  %s = " % phrase

def _question_keygen(q):
	if q.inverted:
		return (q.pair.first_language_id, q.pair.second_language_id)
	else:
		return (q.pair.second_language_id, q.pair.first_language_id)
