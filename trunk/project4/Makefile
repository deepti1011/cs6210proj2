all: basic abort multi multi-abort truncate verbose maxsegments unmapinvalid

maxsegments: librvm.a maxsegments.o
	gcc maxsegments.o librvm.a -o maxsegments

maxsegments.o: maxsegments.c
	gcc -c maxsegments.c

basic: librvm.a basic.o
	gcc basic.o librvm.a -o basic

abort: librvm.a abort.o
	gcc abort.o librvm.a -o abort

multi: librvm.a multi.o
	gcc multi.o librvm.a -o multi

multi-abort: librvm.a multi-abort.o
	gcc multi-abort.o librvm.a  -o multi-abort

truncate: librvm.a truncate.o
	gcc truncate.o librvm.a -o truncate

verbose: librvm.a verbose.o
	gcc verbose.o librvm.a -o verbose

verbose.o: verbose.c
	gcc -c verbose.c

basic.o: basic.c
	gcc -c basic.c

abort.o: abort.c
	gcc -c abort.c

multi.o: multi.c
	gcc -c multi.c

multi-abort.o: multi-abort.c
	gcc -c multi-abort.c

truncate.o: truncate.c
	gcc -c truncate.c

librvm.a: rvm.o
	ar cr librvm.a rvm.o

rvm.o: rvm.c
	gcc -c rvm.c

unmapinvalid: unmapinvalid.o
	gcc -o unmapinvalid unmapinvalid.o librvm.a

unmapinvalid.o: unmapinvalid.c
	gcc -c unmapinvalid.c

test: basic abort multi multi-abort truncate verbose maxsegments unmapinvalid
	./basic
	./abort
	./multi
	./multi-abort
	./verbose
	./maxsegments
	./unmapinvalid
	./truncate

clean:
	rm -r basic basic.o abort abort.o multi multi.o multi-abort \
	multi-abort.o truncate truncate.o librvm.a rvm.o verbose.o verbose \
	maxsegments maxsegments.o unmapinvalid unmapinvalid.o rvm_segments \