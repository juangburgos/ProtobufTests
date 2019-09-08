# Windows
win32 {
	# include pre-compiled library as a dependency
	CONFIG(debug, debug|release) {
		LIBS += -L$$PWD/protobuf.git/cmake/build/Debug
	    LIBS += -lgmock -lgmock_main -llibprotobufd -llibprotobuf-lited -llibprotocd
	} else {
	    LIBS += -L$$PWD/protobuf.git/cmake/build/Release
	    LIBS += -lgmock -lgmock_main -llibprotobuf -llibprotobuf-lite -llibprotoc 
	}	
}
# Linux
linux-g++ {
	LIBS += -L$$PWD/protobuf.git/cmake/build
    LIBS += -lgmock -lgmock_main -lprotobuf -lprotobuf-lite -lprotoc
}
# Mac OS
mac {
	error("Not supported.")
}	
# include header directory
INCLUDEPATH += $$PWD/protobuf.git/src/

# NOTE : use -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF
# https://github.com/protocolbuffers/protobuf/blob/master/cmake/README.md
# https://stackoverflow.com/questions/35116437/errors-when-linking-to-protobuf-3-on-ms-visual-c