# -*- coding: UTF-8 -*-

import sys, os

subject = "Your memo test"

_mail_command = 'mailx "%%s" -s "%s"' % subject

def send_test(test, addr):
	questions = sorted(test.questions, key=_question_keygen)
	prev_key = None
	message = u"Test ID: %i\n\n" % test.id
	for q in questions:
		p = q.pair
		if prev_key != _question_keygen(q):
			prev_key = _question_keygen(q)
			if q.inverted:
				langs = (p.first_language, p.second_language)
			else:
				langs = (p.second_language, p.first_language)
			message += u"%s → %s:\n" % (langs[0].name, langs[1].name)
		phrase = p.first_phrase.value if q.inverted else p.second_phrase.value
		message += u"  %s = \n" % phrase
	_pipe_to_mail_command(message, addr)

def _question_keygen(q):
	if q.inverted:
		return (q.pair.first_language_id, q.pair.second_language_id)
	else:
		return (q.pair.second_language_id, q.pair.first_language_id)

def _pipe_to_mail_command(str, addr):
	pipe = os.popen(_mail_command % addr, 'w')
	pipe.write(str.encode('utf-8'))
	pipe.close()
