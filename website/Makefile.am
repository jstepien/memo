EXTRA_DIST=Memo.setup index.mdwn
all: clean
	if test ! -d dest; then mkdir dest; fi
	${IKIWIKI} -setup ${srcdir}/Memo.setup --templatedir tmpl
	ln -s ${builddir}/../../doc/memo.html dest/manual
	rm -f dest/manual/memo.html
	ln -s ${builddir}/../../doc/doxygen/html dest/techdoc
	rm -f dest/techdoc/html
clean-local:
	rm -rf dest;
