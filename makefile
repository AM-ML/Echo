all:
	clear;gcc echo.c bitboards.c  -o echo;./echo

test:
	clear;gcc test.c bitboards.c -o ftest;./ftest

git:
	clear;git add --all; git commit -am "#${a}"; git push;git log --oneline

log:
	clear;git log --oneline
