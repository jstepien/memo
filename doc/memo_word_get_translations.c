memo_word *words[3], **translations;
int count, i;
for (i = 0; i < 3; ++i) {
	words[i] = memo_word_new(db);
	memo_word_set_value(words[i], random_string[i]);
	memo_word_save(words[i]);
}
memo_word_add_translation(words[0], words[1]);
memo_word_add_translation(words[1], words[2]);
count = memo_word_get_translations(words[1], &translations);
/* count is equal to 2 and translations contains pointers to copies
 * of words[0] and words[2]. */
