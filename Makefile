ALL = miniDHT_server miniDHT_test miniDHT_send miniDHT_recv miniDHT_gui
CXX = clang++
FLAGS = -g -I/usr/local/include -I.. -DWITH_BDB
LIBS = -L/usr/local/lib -lboost_thread-mt -lboost_serialization-mt -lboost_system-mt -lboost_program_options-mt -lboost_date_time-mt -lboost_filesystem-mt -lcrypto -ldb
MINIDHT_HEADERS = miniDHT.h miniDHT_bucket.h miniDHT_const.h miniDHT_contact.h miniDHT_message.h miniDHT_search.h miniDHT_serialize.h miniDHT_db.h
BDB_HEADERS = bdb_basic_db.h bdb_btree.h bdb_hash.h bdb_iterator.h bdb_multibtree.h bdb_serialize.h

all: $(ALL)

server.o: server.cpp $(MINIDHT_HEADER)
	$(CXX) -o server.o $(FLAGS) -c server.cpp
test.o: test.cpp $(MINIDHT_HEADER)
	$(CXX) -o test.o $(FLAGS) -c test.cpp
send.o: send.cpp send.h aes_crypt.h $(MINIDHT_HEADER)
	$(CXX) -o send.o $(FLAGS) -c send.cpp
recv.o: recv.cpp recv.h aes_crypt.h $(MINIDHT_HEADER)
	$(CXX) -o recv.o $(FLAGS) -c recv.cpp
gui.o: gui.cpp gui.h $(MINIDHT_HEADER)
	$(CXX) -o gui.o $(FLAGS) `wx-config --cxxflags` -c gui.cpp

miniDHT_server: server.o
	$(CXX) -o miniDHT_server server.o $(LIBS)
miniDHT_test: test.o
	$(CXX) -o miniDHT_test test.o $(LIBS)
miniDHT_send: send.o
	$(CXX) -o miniDHT_send send.o $(LIBS)
miniDHT_recv: recv.o
	$(CXX) -o miniDHT_recv recv.o $(LIBS)
miniDHT_gui: gui.o
	$(CXX) -o miniDHT_gui gui.o $(LIBS) `wx-config --libs`

clean:
	rm -f *.o core $(ALL)	
