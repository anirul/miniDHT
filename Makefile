# hand made Makefile sorry I m lazy...

ALL = miniDHT_server miniDHT_test \
	miniDHT_send miniDHT_recv BitSmear 

ifeq ($(OSTYPE), darwin)
ALL += BitSmear.app
endif

CXX = clang++
FLAGS = -g -I/usr/local/include -I.. -DWITH_BDB
LIBS = \
	-L/usr/local/lib \
	-lboost_thread-mt \
	-lboost_serialization-mt \
	-lboost_system-mt \
	-lboost_program_options-mt \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lcrypto \
	-ldb
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
gui_main.o: gui_main.cpp gui_main.h gui_network_status.h gui_connect.h gui_info.h $(MINIDHT_HEADER)
	$(CXX) -o gui_main.o $(FLAGS) `wx-config --cxxflags` -c gui_main.cpp
gui_connect.o: gui_connect.cpp gui_connect.h $(MINIDHT_HEADER)
	$(CXX) -o gui_connect.o $(FLAGS) `wx-config --cxxflags` -c gui_connect.cpp
gui_info.o: gui_info.cpp gui_info.h $(MINIDHT_HEADER)
	$(CXX) -o gui_info.o $(FLAGS) `wx-config --cxxflags` -c gui_info.cpp
gui_network_status.o: gui_network_status.cpp gui_network_status.h $(MINIDHT_HEADER)
	$(CXX) -o gui_network_status.o $(FLAGS) `wx-config --cxxflags` -c gui_network_status.cpp

miniDHT_server: server.o
	$(CXX) -o miniDHT_server server.o $(LIBS)
miniDHT_test: test.o
	$(CXX) -o miniDHT_test test.o $(LIBS)
miniDHT_send: send.o
	$(CXX) -o miniDHT_send send.o $(LIBS)
miniDHT_recv: recv.o
	$(CXX) -o miniDHT_recv recv.o $(LIBS)
BitSmear: gui_main.o gui_connect.o gui_info.o gui_network_status.o
	$(CXX) -o BitSmear gui_main.o gui_connect.o gui_info.o gui_network_status.o $(LIBS) `wx-config --libs`

ifeq ($(OSTYPE), darwin)
BitSmear.app: Info.plist BitSmear
	-mkdir BitSmear.app
	-mkdir BitSmear.app/Contents
	-mkdir BitSmear.app/Contents/MacOS
	-mkdir BitSmear.app/Contents/Resources
	-mkdir BitSmear.app/Contents/Resources/English.lproj
	cp Info.plist BitSmear.app/Contents
	cp Knob\ Download.png BitSmear.app/Contents/Resources
	cp Knob\ Upload.png BitSmear.app/Contents/Resources
	cp Knob\ Record\ On.png BitSmear.app/Contents/Resources
	cp Knob\ Record\ Off.png BitSmear.app/Contents/Resources
	cp Knob\ Info.png BitSmear.app/Contents/Resources
	cp Knob\ Cancel.png BitSmear.app/Contents/Resources
	cp Knob\ Grey.png BitSmear.app/Contents/Resources
	echo -n 'APPL????' > BitSmear.app/Contents/PkgInfo
	cp BitSmear BitSmear.app/Contents/MacOS/BitSmear
endif

clean:
	rm -rf *.o core $(ALL)	

