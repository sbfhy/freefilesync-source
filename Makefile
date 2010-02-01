CPPFLAGS=-Wall -pipe -DNDEBUG -DwxUSE_UNICODE `wx-config --cxxflags --debug=no --unicode=yes` `pkg-config --cflags gtk+-2.0` -DFFS_LINUX -DTIXML_USE_STL -DZSTRING_CHAR -O3 -pthread -c -Ishared/boost_1_x
LINKFLAGS=`wx-config --libs --debug=no --unicode=yes` shared/ossp_uuid/.libs/libuuid++.a -O3 -pthread

FILE_LIST=              #internal list of all *.cpp files needed for compilation
FILE_LIST+=structures.cpp
FILE_LIST+=algorithm.cpp
FILE_LIST+=comparison.cpp
FILE_LIST+=synchronization.cpp
FILE_LIST+=fileHierarchy.cpp
FILE_LIST+=application.cpp
FILE_LIST+=ui/guiGenerated.cpp
FILE_LIST+=ui/util.cpp
FILE_LIST+=ui/gridView.cpp
FILE_LIST+=ui/mainDialog.cpp
FILE_LIST+=ui/settingsDialog.cpp
FILE_LIST+=ui/checkVersion.cpp
FILE_LIST+=ui/batchStatusHandler.cpp
FILE_LIST+=ui/guiStatusHandler.cpp
FILE_LIST+=ui/trayIcon.cpp
FILE_LIST+=library/customGrid.cpp
FILE_LIST+=library/errorLogging.cpp
FILE_LIST+=library/statusHandler.cpp
FILE_LIST+=library/resources.cpp
FILE_LIST+=ui/smallDialogs.cpp
FILE_LIST+=library/processXml.cpp
FILE_LIST+=library/statistics.cpp
FILE_LIST+=library/filter.cpp
FILE_LIST+=shared/dragAndDrop.cpp
FILE_LIST+=shared/localization.cpp
FILE_LIST+=shared/guid.cpp
FILE_LIST+=shared/tinyxml/tinyxml.cpp
FILE_LIST+=shared/tinyxml/tinystr.cpp
FILE_LIST+=shared/tinyxml/tinyxmlerror.cpp
FILE_LIST+=shared/tinyxml/tinyxmlparser.cpp
FILE_LIST+=shared/globalFunctions.cpp
FILE_LIST+=shared/systemFunctions.cpp
FILE_LIST+=shared/customTooltip.cpp
FILE_LIST+=shared/fileHandling.cpp
FILE_LIST+=shared/fileTraverser.cpp
FILE_LIST+=shared/standardPaths.cpp
FILE_LIST+=shared/zstring.cpp
FILE_LIST+=shared/xmlBase.cpp
FILE_LIST+=shared/appMain.cpp
FILE_LIST+=shared/customButton.cpp
FILE_LIST+=shared/toggleButton.cpp
FILE_LIST+=shared/customComboBox.cpp
FILE_LIST+=shared/serialize.cpp
FILE_LIST+=shared/fileID.cpp

#list of all *.o files
OBJECT_LIST=$(foreach file, $(FILE_LIST), OBJ/$(subst .cpp,.o,$(notdir $(file))))

#build list of all dependencies
DEP_LIST=$(foreach file, $(FILE_LIST), $(subst .cpp,.dep,$(file)))


all: FreeFileSync

init: 
	if [ ! -d OBJ ]; then mkdir OBJ; fi

#remove byte ordering mark: needed by Visual C++ but an error with GCC
removeBOM: tools/removeBOM.cpp
	g++ -o removeBOM tools/removeBOM.cpp 
	./removeBOM shared/localization.cpp

osspUUID: 
	cd shared/ossp_uuid && chmod +x configure && chmod +x shtool && ./configure --with-cxx --disable-shared && make clean && make && make check

%.dep : %.cpp 
 #strip path information
	g++ $(CPPFLAGS) $< -o OBJ/$(subst .cpp,.o,$(notdir $<))

FreeFileSync: init removeBOM osspUUID $(DEP_LIST)
	g++ -o BUILD/FreeFileSync $(OBJECT_LIST) $(LINKFLAGS)

clean:
	find OBJ -type f -exec rm {} \;
