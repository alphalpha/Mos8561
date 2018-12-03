LIB_DIR =../lib
CATCH_DIR =../third_party
CC=g++
CPPFLAGS=-I$(CATCH_DIR) -I$(LIB_DIR) -std=c++11 -Wall
TESTBIN=tst_mos8561


$(TESTBIN): tests.cpp
	$(CC) -o $@ $< $(CPPFLAGS)

.PHONY: clean

clean:
	rm -f $(TESTBIN)

