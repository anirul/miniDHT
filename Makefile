# hand made Makefile sorry I m lazy...

ALL = \
	miniDHT_proto.pb.h miniDHT_proto.pb.cc \
	libminiDHT.a \
	db_key_value \
	db_multi_key_data \
	session \
	miniDHT_server \
	miniDHT_test \
	miniDHT_send miniDHT_recv \
	BitSmear 

ifeq ($(OSTYPE), darwin)
ALL += BitSmear.app
endif

AR = ar
RANLIB = ranlib
CXX = clang++
PROTOC = protoc
FLAGS = -g -I/usr/include
LIBS = \
	-L/usr/lib \
	-L/usr/local/lib \
	-lboost_thread-mt \
	-lboost_system-mt \
	-lboost_program_options-mt \
	-lboost_date_time-mt \
	-lboost_filesystem-mt \
	-lprotobuf \
	-lcrypto \
	-lpthread \
	-lsqlite3 \
	-lm

all: $(ALL)

miniDHT_proto.pb.h: miniDHT_proto.proto
	$(PROTOC) miniDHT_proto.proto --cpp_out=.
miniDHT_proto.pb.cc: miniDHT_proto.proto
	$(PROTOC) miniDHT_proto.proto --cpp_out=.
miniDHT_proto.pb.o: miniDHT_proto.pb.cc miniDHT_proto.pb.h
	$(CXX) -o miniDHT_proto.pb.o $(FLAGS) -c miniDHT_proto.pb.cc
miniDHT_db.o: miniDHT_db.cpp miniDHT_db.h miniDHT_proto.pb.h
	$(CXX) -o miniDHT_db.o $(FLAGS) -c miniDHT_db.cpp
miniDHT_const.o: miniDHT_const.cpp miniDHT_const.h miniDHT_proto.pb.h
	$(CXX) -o miniDHT_const.o $(FLAGS) -c miniDHT_const.cpp
miniDHT.o: miniDHT.cpp miniDHT.h miniDHT_db.h miniDHT_const.h miniDHT_bucket.h miniDHT_proto.pb.h miniDHT_search.h miniDHT_session.h
	$(CXX) -o miniDHT.o $(FLAGS) -c miniDHT.cpp
libminiDHT.a: miniDHT.o miniDHT_proto.pb.o miniDHT_db.o miniDHT_const.o
	$(AR) cru libminiDHT.a miniDHT.o miniDHT_proto.pb.o miniDHT_db.o miniDHT_const.o
	$(RANLIB) libminiDHT.a

db_key_value.o: db_key_value.cpp miniDHT_db.h 
	$(CXX) -o db_key_value.o $(FLAGS) -c db_key_value.cpp
db_multi_key_data.o: db_multi_key_data.cpp miniDHT_db.h 
	$(CXX) -o db_multi_key_data.o $(FLAGS) -c db_multi_key_data.cpp
session.o: session.cpp miniDHT_session.h
	$(CXX) -o session.o $(FLAGS) -c session.cpp
server.o: server.cpp miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h miniDHT_session.h
	$(CXX) -o server.o $(FLAGS) -c server.cpp
test.o: test.cpp  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o test.o $(FLAGS) -c test.cpp
send.o: send.cpp send.h aes_crypt.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o send.o $(FLAGS) -c send.cpp -DSEND_MAIN_TEST
recv.o: recv.cpp recv.h aes_crypt.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o recv.o $(FLAGS) -c recv.cpp -DRECV_MAIN_TEST

gui_main.o: gui_main.cpp gui_main.h gui_network_status.h gui_connect.h gui_info.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_main.o $(FLAGS) `wx-config --cxxflags` -c gui_main.cpp
gui_connect.o: gui_connect.cpp gui_connect.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_connect.o $(FLAGS) `wx-config --cxxflags` -c gui_connect.cpp
gui_info.o: gui_info.cpp gui_info.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_info.o $(FLAGS) `wx-config --cxxflags` -c gui_info.cpp
gui_network_status.o: gui_network_status.cpp gui_network_status.h gui_dht.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_network_status.o $(FLAGS) `wx-config --cxxflags` -c gui_network_status.cpp
gui_dht.o: gui_dht.cpp gui_dht.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_dht.o $(FLAGS) `wx-config --cxxflags` -c gui_dht.cpp
gui_send.o: send.cpp send.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_send.o $(FLAGS) -c send.cpp
gui_recv.o: recv.cpp recv.h  miniDHT.h miniDHT_bucket.h miniDHT_const.h  miniDHT_search.h miniDHT_db.h miniDHT_session.h
	$(CXX) -o gui_recv.o $(FLAGS) -c recv.cpp
gui_list_ctrl.o: gui_list_ctrl.cpp gui_list_ctrl.h 
	$(CXX) -o gui_list_ctrl.o $(FLAGS) `wx-config --cxxflags` -c gui_list_ctrl.cpp

db_key_value: db_key_value.o libminiDHT.a
	$(CXX) -o db_key_value db_key_value.o libminiDHT.a $(LIBS)
db_multi_key_data: db_multi_key_data.o libminiDHT.a
	$(CXX) -o db_multi_key_data db_multi_key_data.o libminiDHT.a $(LIBS)
session: session.o libminiDHT.a
	$(CXX) -o session session.o libminiDHT.a $(LIBS)
miniDHT_server: server.o libminiDHT.a
	$(CXX) -o miniDHT_server server.o libminiDHT.a $(LIBS)
miniDHT_test: test.o libminiDHT.a
	$(CXX) -o miniDHT_test test.o libminiDHT.a $(LIBS)
miniDHT_send: send.o libminiDHT.a
	$(CXX) -o miniDHT_send send.o libminiDHT.a $(LIBS)
miniDHT_recv: recv.o libminiDHT.a
	$(CXX) -o miniDHT_recv recv.o libminiDHT.a $(LIBS)
BitSmear: gui_main.o gui_connect.o gui_info.o gui_network_status.o gui_dht.o gui_send.o gui_recv.o gui_list_ctrl.o libminiDHT.a
	$(CXX) -o BitSmear \
		gui_main.o \
		gui_connect.o \
		gui_info.o \
		gui_network_status.o \
		gui_dht.o \
		gui_send.o \
		gui_recv.o \
		gui_list_ctrl.o \
		libminiDHT.a \
	$(LIBS) `wx-config --libs`

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
	cp miniDHT_server BitSmear.app/Contents/MacOS/
	cp miniDHT_send BitSmear.app/Contents/MacOS/
	cp miniDHT_recv BitSmear.app/Contents/MacOS/
endif

clean:
	rm -rf *.o core $(ALL)	

