APPNAME = RealTimeSync
prefix = /usr
BINDIR = $(DESTDIR)$(prefix)/bin

CXXFLAGS = -std=c++17 -pipe -DWXINTL_NO_GETTEXT_MACRO -I../../.. -I../../../zenXml -include "zen/i18n.h" -include "zen/warn_static.h" \
           -Wall -Wfatal-errors -Wmissing-include-dirs -Wswitch-enum -Wcast-align -Wshadow -Wnon-virtual-dtor \
           -O3 -DNDEBUG `wx-config --cxxflags --debug=no` -pthread

LINKFLAGS = -s -no-pie `wx-config --libs std, aui --debug=no` -pthread

#Gtk - support "no button border"
CXXFLAGS  += `pkg-config --cflags gtk+-2.0`
LINKFLAGS += `pkg-config --libs   gtk+-2.0`

CPP_FILES=
CPP_FILES+=application.cpp
CPP_FILES+=gui_generated.cpp
CPP_FILES+=main_dlg.cpp
CPP_FILES+=tray_menu.cpp
CPP_FILES+=monitor.cpp
CPP_FILES+=xml_proc.cpp
CPP_FILES+=folder_selector2.cpp
CPP_FILES+=../base/localization.cpp
CPP_FILES+=../base/resolve_path.cpp
CPP_FILES+=../base/ffs_paths.cpp
CPP_FILES+=../../../zen/dir_watcher.cpp
CPP_FILES+=../../../zen/file_access.cpp
CPP_FILES+=../../../zen/file_io.cpp
CPP_FILES+=../../../zen/file_traverser.cpp
CPP_FILES+=../../../zen/zstring.cpp
CPP_FILES+=../../../zen/format_unit.cpp
CPP_FILES+=../../../zen/thread.cpp
CPP_FILES+=../../../wx+/file_drop.cpp
CPP_FILES+=../../../wx+/image_tools.cpp
CPP_FILES+=../../../wx+/image_resources.cpp
CPP_FILES+=../../../wx+/popup_dlg.cpp
CPP_FILES+=../../../wx+/popup_dlg_generated.cpp
CPP_FILES+=../../../xBRZ/src/xbrz.cpp

OBJ_FILES=$(CPP_FILES:%=../../Obj/RTS_GCC_Make_Release/ffs/src/rts/%.o)

all: ../../Build/Bin/$(APPNAME)

../../Build/Bin/$(APPNAME): $(OBJ_FILES)
	g++ -o $@ $^ $(LINKFLAGS)

../../Obj/RTS_GCC_Make_Release/ffs/src/rts/%.o : %
	mkdir -p $(dir $@)
	g++ $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf ../../Obj/RTS_GCC_Make_Release
	rm -f ../../Build/Bin/$(APPNAME)

install:
	mkdir -p $(BINDIR)
	cp ../../Build/Bin/$(APPNAME) $(BINDIR)
