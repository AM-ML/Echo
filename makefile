gcc_warning_flags = -Wunused-variable -Wshadow -Wconversion -Wuninitialized -Wfloat-equal
gcc_optimization_flags = -oFast -O3 -march=native -mtune=native -funroll-loops -finline-functions -flto -fomit-frame-pointer -fprefetch-loop-arrays -ffast-math
release_gcc = gcc $(gcc_warning_flags) $(gcc_optimization_flags) -fopenmp src/*.c -o bin/echo -lm
normal_gcc = gcc $(gcc_warning_flags) src/*.c -o bin/echo -fopenmp -lm

default_linux:
	clear;
	@start=`date +%s%N`; \
	mv bin/echo bin/echo_prev; \
	${normal_gcc}; \
	end=`date +%s%N`; \
	runtime=$$((end - start)); \
	runtime_ms=$$((runtime / 1000000)); \
	start2=`date +%s%N`; \
	./bin/echo; \
	end2=`date +%s%N`; \
	runtime2=$$((end2 - start2)); \
	runtime_ms2=$$((runtime2 / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;96mCompilation: \033[1;93m$${runtime_ms}ms\033[0;0m..."; \
	echo "\033[1;94m----\033[1;96mRuntime: \033[1;93m$${runtime_ms2}ms\033[0;0m..."; \
	echo "-------------------------"


rel:
	clear;
	@start=`date +%s%N`; \
	mv bin/echo bin/echo_prev; \
	${release_gcc}; \
	end=`date +%s%N`; \
	runtime=$$((end - start)); \
	runtime_ms=$$((runtime / 1000000)); \
	start2=`date +%s%N`; \
	./bin/echo; \
	end2=`date +%s%N`; \
	runtime2=$$((end2 - start2)); \
	runtime_ms2=$$((runtime2 / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;96mCompilation: \033[1;93m$${runtime_ms}ms\033[0;0m..."; \
	echo "\033[1;94m----\033[1;96mRuntime: \033[1;93m$${runtime_ms2}ms\033[0;0m..."; \
	echo "-------------------------"


l:
	clear;
	@start=`date +%s%N`; \
	${normal_gcc} ${a}; \
	end=`date +%s%N`; \
	runtime=$$((end - start)); \
	runtime_ms=$$((runtime / 1000000)); \
	start2=`date +%s%N`; \
	./bin/echo; \
	end2=`date +%s%N`; \
	runtime2=$$((end2 - start2)); \
	runtime_ms2=$$((runtime2 / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;96mCompilation: \033[1;93m$${runtime_ms}ms\033[0;0m..."; \
	echo "\033[1;94m----\033[1;96mRuntime: \033[1;93m$${runtime_ms2}ms\033[0;0m..."; \
	echo "-------------------------"

r:
	start2=`date +%s%N`; \
	clear; ./bin/echo; \
	end2=`date +%s%N`; \
	runtime2=$$((end2 - start2)); \
	runtime_ms2=$$((runtime2 / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;94m----\033[1;96mRuntime: \033[1;93m$${runtime_ms2}ms\033[0;0m..."; \
	echo "-------------------------"

win:
	clear; x86_64-w64-mingw32-gcc $(gcc_warning_flags) $(gcc_optimization_flags) -fopenmp src/*.c -o bin/echo_win64.exe -lm

compile:
	clear;
	@start=`date +%s%N`; \
	${normal_gcc}; \
	end=`date +%s%N`; \
	runtime=$$((end - start)); \
	runtime_ms=$$((runtime / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;96mCompilation: \033[1;93m$${runtime_ms}ms\033[0;0m..."; \
	echo "-------------------------"


math:
	clear;
	@start=`date +%s%N`; \
	${normal_gcc} -lm; \
	end=`date +%s%N`; \
	runtime=$$((end - start)); \
	runtime_ms=$$((runtime / 1000000)); \
	start2=`date +%s%N`; \
	./bin/echo; \
	end2=`date +%s%N`; \
	runtime2=$$((end2 - start2)); \
	runtime_ms2=$$((runtime2 / 1000000)); \
	echo "-------------------------"; \
	echo "\033[1;96mCompilation: \033[1;93m$${runtime_ms}ms\033[0;0m..."; \
	echo "\033[1;94m----\033[1;96mRuntime: \033[1;93m$${runtime_ms2}ms\033[0;0m..."; \
	echo "-------------------------"

update:
		cp "src/" "/mnt/c/Users/Suzy M. Najdy/Desktop/echo/" -rf
