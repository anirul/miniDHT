ACLOCAL_AMFLAGS=-I m4

if MACOSX
AM_CPPFLAGS = -arch x86_64 -arch i386 -mmacosx-version-min=10.6  
AM_LDFLAGS =  -arch x86_64 -arch i386 -mmacosx-version-min=10.6 -headerpad_max_install_names
endif

bin_PROGRAMS = miniDHT_server miniDHT_test miniDHT_send miniDHT_recv 

miniDHT_server_SOURCES = server.cpp miniDHT.h
miniDHT_server_LDADD = libminiDHT-1.0.la
miniDHT_server_LDFLAGS = $(AM_LDFLAGS) -static libminiDHT-1.0.la

miniDHT_test_SOURCES = test.cpp miniDHT.h libminiDHT-1.0.la
miniDHT_test_LDADD = libminiDHT-1.0.la
miniDHT_test_LDFLAGS = $(AM_LDFLAGS) -static libminiDHT-1.0.la

miniDHT_send_CPPFLAGS = -DSEND_MAIN_TEST $(AM_CPPFLAGS)
miniDHT_send_SOURCES = send.cpp send.h miniDHT.h libminiDHT-1.0.la
miniDHT_send_LDADD = libminiDHT-1.0.la
miniDHT_send_LDFLAGS = $(AM_LDFLAGS) -static libminiDHT-1.0.la

miniDHT_recv_CPPFLAGS = -DRECV_MAIN_TEST $(AM_CPPFLAGS)
miniDHT_recv_SOURCES = recv.cpp recv.h miniDHT.h libminiDHT-1.0.la
miniDHT_recv_LDADD = libminiDHT-1.0.la
miniDHT_recv_LDFLAGS = $(AM_LDFLAGS) -static libminiDHT-1.0.la



lib_LTLIBRARIES = libminiDHT-1.0.la

libminiDHT_1_0_la_SOURCES = miniDHT.cpp miniDHT.h miniDHT_db.cpp miniDHT_db.h miniDHT_const.cpp miniDHT_const.h miniDHT_bucket.h miniDHT_search.h miniDHT_session.h  
nodist_libminiDHT_1_0_la_SOURCES = miniDHT_proto.pb.h miniDHT_proto.pb.cc
BUILT_SOURCES = miniDHT_proto.pb.h miniDHT_proto.pb.cc
CLEANFILES = miniDHT_proto.pb.h miniDHT_proto.pb.cc

miniDHT_proto.pb.h: miniDHT_proto.proto
	@PROTOC@ miniDHT_proto.proto --cpp_out=.
miniDHT_proto.pb.cc: miniDHT_proto.proto
	@PROTOC@ miniDHT_proto.proto --cpp_out=.


if USE_WX
bin_PROGRAMS += BitSmear
BitSmear_SOURCES = gui_main.cpp gui_main.h gui_network_status.h gui_network_status.cpp gui_info.h gui_info.cpp gui_frame.h gui_frame.cpp gui_connect.h gui_connect.cpp gui_dht.h gui_dht.cpp gui_list_ctrl.h gui_list_ctrl.cpp send.h send.cpp recv.h recv.cpp miniDHT.h
BitSmear_LDFLAGS = $(AM_LDFLAGS) -static libminiDHT-1.0.la
BitSmear_LDADD = libminiDHT-1.0.la

if MACOSX
bin_PROGRAMS += BitSmear_MACOSX
.PHONY: BitSmear_MACOSX

BitSmear_MACOSX: Info.plist BitSmear miniDHT_server miniDHT_send miniDHT_recv
	-mkdir -p BitSmear.app/Contents/MacOS
	-mkdir -p BitSmear.app/Contents/Frameworks
	-mkdir -p BitSmear.app/Contents/Resources/English.lproj
	cp -f Info.plist BitSmear.app/Contents
	cp -f Knob\ Download.png BitSmear.app/Contents/Resources
	cp -f Knob\ Upload.png BitSmear.app/Contents/Resources
	cp -f Knob\ Record\ On.png BitSmear.app/Contents/Resources
	cp -f Knob\ Record\ Off.png BitSmear.app/Contents/Resources
	cp -f Knob\ Info.png BitSmear.app/Contents/Resources
	cp -f Knob\ Cancel.png BitSmear.app/Contents/Resources
	cp -f Knob\ Grey.png BitSmear.app/Contents/Resources
	echo -n 'APPL????' > BitSmear.app/Contents/PkgInfo
	cp -f BitSmear BitSmear.app/Contents/MacOS/BitSmear
	cp -f miniDHT_server BitSmear.app/Contents/MacOS/
	cp -f miniDHT_send BitSmear.app/Contents/MacOS/
	cp -f miniDHT_recv BitSmear.app/Contents/MacOS/
	cp -af @BOOST_LIB_DIR@/libboost_thread-mt.dylib BitSmear.app/Contents/Frameworks/
	cp -af @BOOST_LIB_DIR@/libboost_system-mt.dylib BitSmear.app/Contents/Frameworks/
	cp -af @BOOST_LIB_DIR@/libboost_program_options-mt.dylib BitSmear.app/Contents/Frameworks/
	cp -af @BOOST_LIB_DIR@/libboost_date_time-mt.dylib BitSmear.app/Contents/Frameworks/
	cp -af @BOOST_LIB_DIR@/libboost_filesystem-mt.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libprotobuf.7.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libSDL-1.2.0.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libiconv.2.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libexpat.1.5.2.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libpng14.14.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libjpeg.8.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libtiff.3.dylib BitSmear.app/Contents/Frameworks/
	cp -af @PORT_LIB_DIR@/libwx*2.9.2.0.0.dylib BitSmear.app/Contents/Frameworks
	./embed_macosxlib.sh BitSmear
#	cp -af @BOOST_LIB_DIR@/libboost_serialization-mt.dylib BitSmear.app/Contents/Frameworks/

endif

endif

clean-local:
	-rm -rf BitSmear.app

