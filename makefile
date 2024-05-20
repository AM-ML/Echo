all:
	clear;gcc echo.c  -o echo;./echo

test:
	clear;gcc test.c -o test;./test

git:
	clear;git add --all; git commit -am "#${a}"; git push;git log --oneline

log:
	clear;git log --oneline
