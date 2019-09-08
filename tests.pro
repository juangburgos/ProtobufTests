TEMPLATE = subdirs

# NOTE : tests.pro must be in repo root because 
# qmake on Linux does not support subdirs outside $$PWD

# Projects
SUBDIRS = \
00_proto_test \
01_ws_server  \
02_ws_client  \
03_gui_client \

# Directories
00_proto_test.subdir = $$PWD/tests/00_proto_test
01_ws_server.subdir  = $$PWD/tests/01_ws_server
02_ws_client.subdir  = $$PWD/tests/02_ws_client
03_gui_client.subdir = $$PWD/tests/03_gui_client

# Dependencies
#00_proto_test.depends =
#01_ws_server.depends  =
#02_ws_client.depends  = 
#03_gui_client.depends = 