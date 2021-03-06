# This specifies the exe name
TARGET=PBRTexture
# where to put the .o files
OBJECTS_DIR=obj
# core Qt Libs to use
QT+=gui opengl core quick quickwidgets
# as I want to support 4.8 and 5 this will set a flag for some of the mac stuff
# mainly in the types.h file for the setMacVisual which is native in Qt5
isEqual(QT_MAJOR_VERSION, 5) {
	cache()
	DEFINES +=QT5BUILD
}
# where to put moc auto generated files
MOC_DIR=moc
# on a mac we don't create a .app bundle file ( for ease of multiplatform use)
CONFIG-=app_bundle
# Specified included .cpps
SOURCES+= $$PWD/src/NGLScene.cpp    \
          $$PWD/src/NGLSceneMouseControls.cpp \
          $$PWD/src/main.cpp \
          $$PWD/src/TexturePack.cpp \
          $$PWD/src/FirstPersonCamera.cpp \
          $$PWD/src/Sphere.cpp
# same for the .h files
HEADERS+= $$PWD/include/NGLScene.h \
          $$PWD/include/WindowParams.h \
          $$PWD/include/TexturePack.h \
          $$PWD/include/FirstPersonCamera.h \
          $$PWD/include/scene.h \
          $$PWD/include/Sphere.h
# and add the include dir into the search path for Qt and make
INCLUDEPATH +=./include
# project root
DESTDIR=./
# adding all glsl files, json and data
OTHER_FILES+= shaders/*.glsl \
              README.md \
              textures/textures.json
# were are going to default to a console app
CONFIG += console
# note each command you add needs a ; as it will be run as a single line
# first check if we are shadow building or not easiest way is to check out against current
!equals(PWD, $${OUT_PWD}){
	copydata.commands = echo "creating destination dirs" ;
	# now make a dir
	copydata.commands += mkdir -p $$OUT_PWD/shaders ;
	copydata.commands += echo "copying files" ;
	# then copy the files
	copydata.commands += $(COPY_DIR) $$PWD/shaders/* $$OUT_PWD/shaders/ ;
	# now make sure the first target is built before copy
	first.depends = $(first) copydata
	export(first.depends)
	export(copydata.commands)
	# now add it as an extra target
	QMAKE_EXTRA_TARGETS += first copydata
}
NGLPATH=$$(NGLDIR)
isEmpty(NGLPATH){ # note brace must be here
	message("including $HOME/NGL")
	include($(HOME)/NGL/UseNGL.pri)
}
else{ # note brace must be here
	message("Using custom NGL location")
	include($(NGLDIR)/UseNGL.pri)
}



