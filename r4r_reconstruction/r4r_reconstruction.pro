######################################################################################
#
# Copyright (c) 2013, Jonathan Balzer
#
# All rights reserved.
#
# This file is part of the R4R library.
#
# The R4R library is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The R4R library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with the R4R library. If not, see <http://www.gnu.org/licenses/>.
#
######################################################################################

QT += opengl

QMAKE_CXXFLAGS += -std=c++0x

TARGET = r4r_reconstruction
TEMPLATE = lib

DEFINES += R4R_RECONSTRUCTION_LIBRARY

SOURCES += viewer.cpp \
           trimesh.cpp

HEADERS += viewer.h \
           trimesh.h

# local inlude path
INCLUDEPATH += $$PWD/../r4r_core \
               $$PWD/../r4r_motion

# make sure that r4r_core is up to date
DEPENDPATH += $$PWD/../r4r_core \
              $$PWD/../r4r_motion

unix:!symbian|win32 {

    # create pkg file
    CONFIG += create_prl no_install_prl create_pc

    # find OpenMesh library
    OM = $$system(find /usr -name libOpenMeshCore* 2>/dev/null)
    isEmpty(OM) {
        warning("Could not resolve dependency on OpenMesh.")
    } else {

        OM = $$first(OM)
        OMLIBPATH = $$dirname(OM)
        DEFINES += HAVE_OM

        LIBS += -L$$OMLIBPATH \
                -lOpenMeshCore \
                -lOpenMeshTools

        # add this so the binary knows the location of OM in non-standard path
        QMAKE_LFLAGS += -Wl,-rpath=$$OMLIBPATH

    }

    # create install target
    headers.files = $$HEADERS
    headers.path = /usr/include/r4r/

    target.path = /usr/lib/

    INSTALLS += target \
                headers

}
