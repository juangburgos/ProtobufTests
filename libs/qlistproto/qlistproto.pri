include($$PWD/../protobuf.pri)

INCLUDEPATH += $$PWD/

!build_pass:{
	# Windows
	win32 {
		PROTOC_EXE        = $$PWD/../protobuf.git/cmake/build/Debug/protoc.exe
		PROTOC_EXE_WIN    = $$PROTOC_EXE
		PROTOC_EXE_WIN    ~= s,/,\\,g
		PROTOC_TARGET     = $$PWD
		PROTOC_TARGET_WIN = $$PROTOC_TARGET
		PROTOC_TARGET_WIN ~= s,/,\\,g
		# Compile proto if not exists
		!exists($${PROTOC_EXE}) {
			include($$PWD/../../libs/protobuf_build.pri)
		}
		# Run protoc
		PROTO_COMPILED = FALSE
		system("$${PROTOC_EXE_WIN} --cpp_out=$${PROTOC_TARGET_WIN} qlistproto.proto"): PROTO_COMPILED = TRUE
		equals(PROTO_COMPILED, TRUE) {
			message("Proto compile successful.")
		}
		else {
			error("Proto compile failed.")
		}
	}
	# Linux
	linux-g++ {
		PROTOC_EXE    = $$PWD/../protobuf.git/cmake/build/protoc
		PROTOC_TARGET = $$PWD
		# Compile proto if not exists
		!exists($${PROTOC_EXE}) {
			include($$PWD/../../libs/protobuf_build.pri)
		}
		# Run protoc
		PROTO_COMPILED = FALSE
		system("$${PROTOC_EXE} --cpp_out=$${PROTOC_TARGET} qlistproto.proto"): PROTO_COMPILED = TRUE
		equals(PROTO_COMPILED, TRUE) {
			message("Proto compile successful.")
		}
		else {
			error("Proto compile failed.")
		}
	}
	# Mac OS
	mac {
		error("Not supported.")
	}	
}

SOURCES  += \
$$PWD/qlistproto.pb.cc \
$$PWD/qlistelement.cpp \
$$PWD/qlistrequest.cpp \
$$PWD/qlistresponse.cpp

HEADERS  += \
$$PWD/qlistproto.pb.h \
$$PWD/qlistelement.h \
$$PWD/qlistrequest.h \
$$PWD/qlistresponse.h
