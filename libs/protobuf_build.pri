# Paths
PROTOBUF_PATH       = $$PWD/protobuf.git/cmake
PROTOBUF_BUILD_PATH = $$PROTOBUF_PATH/build
# Windows
win32 {
	message("Compiling protobuf for Windows")
	# Fix PWD slashes
	PROTOBUF_PATH_WIN       = $$PROTOBUF_PATH
	PROTOBUF_PATH_WIN       ~= s,/,\\,g
	PROTOBUF_BUILD_PATH_WIN = $$PROTOBUF_BUILD_PATH
	PROTOBUF_BUILD_PATH_WIN ~= s,/,\\,g
	# Look for CMake
	CMAKE_BIN = $$system(where cmake)
	isEmpty(CMAKE_BIN) {
		error("CMake not found. Cannot build protobuf.")
	}
	else {
		message("CMake found.")
	}
	# Look for msbuild
	MSBUILD_BIN = $$system(where msbuild)
	isEmpty(MSBUILD_BIN) {
		error("MsBuild not found. Cannot build protobuf.")
	}
	else {
		message("MsBuild found.")
	}
	# Clean up old build if any
	exists($${PROTOBUF_BUILD_PATH}) {
		system("rmdir $${PROTOBUF_BUILD_PATH_WIN} /s /q")
	}
	# Create build
	BUILD_CREATED = FALSE
	system("mkdir $${PROTOBUF_BUILD_PATH_WIN}"): BUILD_CREATED = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("Build directory created for protobuf.")
	}
	else {
		error("Build directory could not be created for protobuf.")
	}
	# Find compatible compiler
	MSVC_VER = $$(VisualStudioVersion)
    equals(MSVC_VER, 12.0){
		message("Compiler Detected : MSVC++ 12.0 (Visual Studio 2013)")
		COMPILER = "Visual Studio 12 2013"
    }
    equals(MSVC_VER, 14.0){
		message("Compiler Detected : MSVC++ 14.0 (Visual Studio 2015)")
		COMPILER = "Visual Studio 14 2015"
    }
    equals(MSVC_VER, 15.0){
		message("Compiler Detected : MSVC++ 15.0 (Visual Studio 2017)")
		COMPILER = "Visual Studio 15 2017"
    }
    equals(MSVC_VER, 16.0){
		message("Compiler Detected : MSVC++ 16.0 (Visual Studio 2019)")
		COMPILER = "Visual Studio 16 2019"
    }
    isEmpty(COMPILER) {
		error("No compatible compiler found to generate protobuf.")
	}
    # Find platform
	contains(QT_ARCH, i386) {
		message("Platform Detected : 32 bits")
		PLATFORM = "Win32"	
	}
	contains(QT_ARCH, x86_64) {
		message("Platform Detected : 64 bits")
		PLATFORM = "x64"			
	}
	isEmpty(PLATFORM) {
		error("Non compatible platform $${QT_ARCH} to generate protobuf.")
	}
	# Generate CMake project
	PROJECT_CREATED = FALSE
	system("cmake $${PROTOBUF_PATH_WIN} -B$${PROTOBUF_BUILD_PATH_WIN} -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_MSVC_STATIC_RUNTIME=OFF -G \"$${COMPILER}\" -A $${PLATFORM}"): PROJECT_CREATED = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("CMake generate protobuf successful.")
	}
	else {
		error("CMake generate protobuf failed.")
	}
	# Build Visual Studio project
	PROJECT_BUILT = FALSE
	system("msbuild $${PROTOBUF_BUILD_PATH_WIN}\protobuf.sln"): PROJECT_BUILT = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("protobuf build successful.")
	}
	else {
		error("protobuf build failed.")
	}
}
# Linux
linux-g++ {
	message("Compiling protobuf for Linux.")
	# Look for CMake
	CMAKE_BIN = $$system(which cmake)
	isEmpty(CMAKE_BIN) {
		error("CMake not found. Cannot build protobuf.")
	}
	else {
		message("CMake found.")
	}
	# Look for make
	MAKE_BIN = $$system(which make)
	isEmpty(MAKE_BIN) {
		error("Make not found. Cannot build protobuf.")
	}
	else {
		message("Make found.")
	}
	# Clean up old build if any
	exists($${PROTOBUF_BUILD_PATH}) {
		system("rm -rf $${PROTOBUF_BUILD_PATH}")
	}
	# Create build
	BUILD_CREATED = FALSE
	system("mkdir $${PROTOBUF_BUILD_PATH}"): BUILD_CREATED = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("Build directory created for protobuf.")
	}
	else {
		error("Build directory could not be created for protobuf.")
	}
	# Generate CMake project
	PROJECT_CREATED = FALSE
	system("cmake $${PROTOBUF_PATH} -B$${PROTOBUF_BUILD_PATH} -Dprotobuf_BUILD_SHARED_LIBS=OFF"): PROJECT_CREATED = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("CMake generate protobuf successful.")
	}
	else {
		error("CMake generate protobuf failed.")
	}
    # Build project
	PROJECT_BUILT = FALSE
	system("make -C $${PROTOBUF_BUILD_PATH} all"): PROJECT_BUILT = TRUE
	equals(BUILD_CREATED, TRUE) {
		message("protobuf build successful.")
	}
	else {
		error("protobuf build failed.")
	}
}
# Mac OS
mac {
	error("Not supported.")
}	
