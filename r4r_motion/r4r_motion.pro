#-------------------------------------------------
#
# Project created by QtCreator 2013-02-21T16:45:43
#
#-------------------------------------------------

QMAKE_CXXFLAGS += -std=c++0x

TARGET = r4r_motion
TEMPLATE = lib

DEFINES += R4R_MOTION_LIBRARY

SOURCES += \
    tracker.cpp \
    stracker.cpp \
    ptracker.cpp \
    mtracker.cpp \
    map.cpp \
    lk.cpp \
    feature.cpp \
    descriptor.cpp \
    basic.cpp \
    tval.cpp \
    tsttrack.cpp \
    tracklet.cpp \
    dagg.cpp

HEADERS += \
    tval.h \
    tracker.h \
    stracker.h \
    ptracker.h \
    mtracker.h \
    map.h \
    lk.h \
    feature.h \
    descriptor.h \
    basic.h \
    tsttrack.h \
    tracklet.h \
    dagg.h

symbian {
    MMP_RULES += EXPORTUNFROZEN
    TARGET.UID3 = 0xED8A93FC
    TARGET.CAPABILITY = 
    TARGET.EPOCALLOWDLLDATA = 1
    addFiles.sources = r4r_motion.dll
    addFiles.path = !:/sys/bin
    DEPLOYMENT += addFiles
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
    INCLUDEPATH += ../r4r_core
    LIBS += \
         -L/usr/local/lib/\
         -lopencv_core\
         -lopencv_highgui\
         -lopencv_video\
         -lopencv_imgproc\
         -lopencv_features2d\
         -lopencv_calib3d

}

