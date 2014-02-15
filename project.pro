#-------------------------------------------------
#
# Project created by QtCreator 2012-02-24T04:40:29
#
#-------------------------------------------------

QT += core gui widgets

#TARGET = second
TEMPLATE = app

INCLUDEPATH += C:\Users\Jimmy\Documents\projects\tutorials\opencvBuilt\install\include \
		include\

#LIBS+= -LC:/Users/Jimmy/Downloads/opencv/build/x86/mingw/lib/ -lopencv_core242 -lopencv_highgui242
LIBS+= -LC:\\Users\\Jimmy\\Documents\\projects\\tutorials\\opencvBuilt\\install\\lib \
-lopencv_core246.dll  \
-lopencv_highgui246.dll \
-lopencv_video246.dll \
-lopencv_imgproc246.dll \

CONFIG += console

SOURCES += src/main.cpp \
        src/mainwindow.cpp \
		src/ImageFormat.cpp


HEADERS  += include/mainwindow.h \ 
		 include/Blob.h \
		 include/ImageFormat.h

# FORMS    += dialog.ui
