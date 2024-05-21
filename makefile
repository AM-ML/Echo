all:
	clear;gcc echo.c bitboards.c  -o echo;./echo

t:
	clear;gcc test.c bitboards.c -o ftest;./ftest

git:
	clear;git add --all; git commit -am "#${a}"; git push;git log --oneline

g2:
	clear;git commit -m "#${a}"; git push; git log --oneline

gbb:
	clear; ga bitboard.c bitboard.h;gc -m "#${a}"; gp; glo

gt:
	clear; git add test.c; git commit -m "#${a}"; git push; git log --oneline

gmk:
	clear; git add makefile; git commit -m "#${a}: makefile refactoring."; git push; git log --oneline

log:
	clear;git log --oneline

gc:
	clear; gc ; git log
