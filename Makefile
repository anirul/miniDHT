ALL = miniDHT-server miniDHT-test miniDHT-send miniDHT-recv
CXX = clang++
FLAGS = -g -I/usr/local/include -I.. -DWITH_BDB
LIBS = -L/usr/local/lib -lboost_serialization-mt -lboost_system-mt -lboost_program_options-mt -lboost_date_time-mt -lcrypto -ldb
MINIDHT_HEADERS = miniDHT.h miniDHT_bucket.h miniDHT_const.h miniDHT_contact.h miniDHT_message.h miniDHT_search.h miniDHT_serialize.h miniDHT_db.h
BDB_HEADERS = bdb_basic_db.h bdb_btree.h bdb_hash.h bdb_iterator.h bdb_multibtree.h bdb_serialize.h

all: $(ALL)

server.o: server.cpp $(MINIDHT_HEADER)
	$(CXX) -o server.o $(FLAGS) -c server.cpp
test.o: test.cpp $(MINIDHT_HEADER)
	$(CXX) -o test.o $(FLAGS) -c test.cpp
send.o: send.cpp $(MINIDHT_HEADER)
	$(CXX) -o send.o $(FLAGS) -c send.cpp
recv.o: recv.cpp $(MINIDHT_HEADER)
	$(CXX) -o recv.o $(FLAGS) -c recv.cpp

miniDHT-server: server.o
	$(CXX) -o miniDHT-server server.o $(LIBS)
miniDHT-test: test.o
	$(CXX) -o miniDHT-test test.o $(LIBS)
miniDHT-send: send.o
	$(CXX) -o miniDHT-send send.o $(LIBS)
miniDHT-recv: recv.o
	$(CXX) -o miniDHT-recv recv.o $(LIBS)

clean:
	rm -f *.o core $(ALL)	
