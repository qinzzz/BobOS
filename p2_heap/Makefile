TEST_EXTS = .cc .ok
ID=$(shell pwd | sed -e 's/.*_//')

MY_TESTS = ${addprefix ${ID},${TEST_EXTS}}
SIG = ${firstword ${shell (ls ${MY_TESTS} > /dev/null 2>&1  && (cat ${MY_TESTS} | md5sum)) || echo "?" }}

POSSIBLE_TESTS = ${basename ${wildcard *${firstword ${TEST_EXTS}}}}
TESTS = ${POSSIBLE_TESTS}
TEST_OKS = ${addsuffix .ok,${TESTS}}
TEST_RESULTS = ${addsuffix .result,${TESTS}}
TEST_OUTS = ${addsuffix .out,${TESTS}}
TEST_RAWS = ${addsuffix .raw,${TESTS}}
TEST_DIFFS = ${addsuffix .diff,${TESTS}}

KERNELS = ${addsuffix .kernel,${TESTS}}

all : t0.kernel;

$(KERNELS) : %.kernel :
	(cd kernel; rm -f kernel.cc)
	(cd kernel; ln -s ../$*.cc kernel.cc)
	(cd kernel; make clean)
	(cd kernel; make)

clean:
	rm -rf *.diff *.raw *.out *.result
	(make -C kernel clean)

clean2::
	rm -rf *.diff

${TEST_RAWS} : %.raw : Makefile %.kernel
	timeout 20 qemu-system-i386 -no-reboot -nographic -smp 4 --monitor none --serial file:$*.raw -drive file=kernel/kernel.img,index=0,media=disk,format=raw -device isa-debug-exit,iobase=0xf4,iosize=0x04 2> /dev/null || true

${TEST_OUTS} : %.out : Makefile %.raw
	-egrep '^\*\*\*' $*.raw > $*.out 2> /dev/null

${TEST_DIFFS} : %.diff : Makefile %.out %.ok
	-diff -wBb $*.out $*.ok > $*.diff 2> /dev/null

${TEST_RESULTS} : %.result : Makefile %.diff
	(test -z "`cat $*.diff`" && echo "pass" > $*.result) || echo "fail" > $*.result

test: Makefile ${TEST_RESULTS}
	for i in ${TEST_RESULTS}; do echo "`basename -s .result $$i` ... `cat $$i`"; done
	
% :
	(make -C kernel $@)

###################### Test #################

sig:
	@echo ${SIG}


copy_student_tests:
ifeq ($(SIG),?)
	@echo "no tests"
else
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp $(ID)$$i $(TEST_DIR)/$(SIG)$$i; \
	done
endif
endif

copy_given_tests:
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp t?$$i $(TEST_DIR); \
	done
endif

remove_tests:
	@for i in $(TEST_EXTS); do \
	    rm -rf *$$i; \
	done
