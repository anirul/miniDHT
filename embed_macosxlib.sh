#!/bin/bash

LIBPOSTFIX=lib*.dylib
APP=$1
     
PORTLIBDIR=/opt/local/lib
BINDIR=$1.app/Contents/MacOS
LIBDIR=$1.app/Contents/Frameworks
   

echo "Changing directory to " $BINDIR " ..."

# patch all wx dynlibs and Saracon executable
for file in `ls  $LIBDIR/$LIBPOSTFIX | xargs basename`
	do
	# patch all library internal cross references
	echo "Patching " $file "..."
	for fileother in `ls $LIBDIR/$LIBPOSTFIX | xargs basename`
	do
		# library
		echo "  Patching " $fileother " with " $file "..."
		install_name_tool  -change $PORTLIBDIR/$file @executable_path/../Frameworks/$file  $LIBDIR/$fileother
	done
	
	# we need to add a few manual correction to make it work
	install_name_tool -change $PORTLIBDIR/libz.1.dylib /usr/lib/libz.1.dylib $LIBDIR/$file
	install_name_tool -change $PORTLIBDIR/libexpat.1.dylib @executable_path/../Frameworks/libexpat.1.5.2.dylib $LIBDIR/$file
	 

	# patch current library itself
	install_name_tool  -id  @executable_path/../Frameworks/$file  $LIBDIR/$file
	# patch executable
	for execs in `ls $BINDIR/* | xargs basename`
	do
		echo "  Patching " $BINDIR/$execs " with " $file "..."
		install_name_tool  -change $PORTLIBDIR/$file @executable_path/../Frameworks/$file $BINDIR/$execs
		install_name_tool -change $PORTLIBDIR/libboost_system.dylib @executable_path/../Frameworks/libboost_system-mt.dylib $BINDIR/$execs
	done
done
