/*////////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013, Jonathan Balzer
//
// All rights reserved.
//
// This file is part of the R4R library.
//
// The R4R library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The R4R library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with the R4R library. If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////*/

#include "viewer.h"

#include <iostream>


#define NEAR_PLANE_TOL 0.8
#define FAR_PLANE_TOL 1.6

using namespace std;

namespace R4R {

CViewer::CViewer(const R4R::CView<double>& view, QWidget* parent):
    QGLWidget(parent),
    m_view(view),
    m_znear(1),
    m_zfar(10.0),
    m_last_point(),
    m_center(),
    m_show_color(true) {

    // set up window size according to resolution of camera
    const CPinholeCam& cam = dynamic_cast<const CPinholeCam&>(m_view.GetCam());
    CVector<size_t,2> sizes = cam.GetSize();
    setFixedSize(sizes.Get(0),sizes.Get(1));

    // set cursor type and window title
    setCursor(Qt::PointingHandCursor);
    setWindowTitle("OpenGL Viewer");

}

void CViewer::initializeGL(){

    glClearColor(0.1, 0.1, 0.1, 1.0);

    // this is in normalized device coordinates
    glClearDepth(1.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glDepthFunc(GL_LEQUAL);

    // access to intrinsics
    const CPinholeCam& cam = dynamic_cast<const CPinholeCam&>(m_view.GetCam());
    mat K = cam.GetProjectionMatrix();
    QSize ws = this->size();

    // set viewport size != window size
    glViewport(0,0,ws.width(),ws.height());

    // load intrinsics
    loadProjectionMatrix();

    // load extrinsics
    loadView(m_view);

    // set light positions
    GLfloat pos1[] = { 0.1,  0.1, -0.02, 0.0};
    GLfloat pos2[] = {-0.1,  0.1, -0.02, 0.0};
    GLfloat pos3[] = { 0.0,  0.0,  0.1,  0.0};
    GLfloat col1[] = { 0.7,  0.7,  0.8,  1.0};
    GLfloat col2[] = { 0.8,  0.7,  0.7,  1.0};
    GLfloat col3[] = { 0.5,  0.5,  0.5,  1.0};

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0,GL_POSITION, pos1);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,  col1);
    glLightfv(GL_LIGHT0,GL_SPECULAR, col1);

    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1,GL_POSITION, pos2);
    glLightfv(GL_LIGHT1,GL_DIFFUSE,  col2);
    glLightfv(GL_LIGHT1,GL_SPECULAR, col2);

    glEnable(GL_LIGHT2);
    glLightfv(GL_LIGHT2,GL_POSITION, pos3);
    glLightfv(GL_LIGHT2,GL_DIFFUSE,  col3);
    glLightfv(GL_LIGHT2,GL_SPECULAR, col3);

    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

}

void CViewer::paintGL(){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // paint some dummy object
    glBegin(GL_TRIANGLES);

    if(m_show_color)
        glColor3f(0.0f,0.0f,1.0f);
    else
        glColor3f(0.5f,0.5f,0.5f);

    glVertex3f(0.0f,0.0f,2.0f);

    if(m_show_color)
        glColor3f(0.0f,1.0f,0.0f);

    glVertex3f(1.0f,0.0f,2.0f);

    if(m_show_color)
        glColor3f(1.0f,0.0f,0.0f);

    glVertex3f(0.0f,1.0f,2.0f);

    glEnd();

}

void CViewer::wheelEvent(QWheelEvent* event) {

    double radius = 1;
    double d = -(double)event->delta()/120.0*0.2*radius;
    makeCurrent();

    vec3 dt = { 0, 0, 0.01f * (m_zfar - m_znear) * d };
    m_view.DifferentialTranslate(dt);

    loadView(m_view);
    updateGL();

    event->accept();

}

void CViewer::mousePressEvent(QMouseEvent* event) {

    m_last_point = event->pos();
    event->accept();

}

void CViewer::mouseMoveEvent(QMouseEvent* event) {

    QPoint newpoint = event->pos();

    double dx = newpoint.x() - m_last_point.x();
    double dy = newpoint.y() - m_last_point.y();

    // enable GL context
    makeCurrent();

    if (event->buttons()==Qt::MidButton) {

        // need depth here to backproject image plane translations (take mean of clipping depths)
        vec3 dt = { -0.0005*(m_zfar-m_znear)*dx, -0.0005*(m_zfar-m_znear)*dy, 0 };
        m_view.DifferentialTranslate(dt);

    }
    else if (event->buttons() == Qt::LeftButton)
    {

        vec3 axis = { -0.05*dy, 0.05*dx, 0 };

        // transform rotation axis into world coordinates
        const CRigidMotion<double,3> Finv = m_view.GetInverseTransformation();
        axis = Finv.DifferentialTransform(axis);

        m_view.Orbit(m_center,axis);

    }
    else if (event->buttons() == Qt::RightButton)
    {

        vec3 dt;
        if(dx<0)
            dt(2) = 0.0005*(m_zfar-m_znear)*fabs(dx);
        else
            dt(2) = -0.0005*(m_zfar-m_znear)*fabs(dx);

        m_view.DifferentialTranslate(dt);

    }

    loadView(m_view);
    updateGL();

    // remember this point
    m_last_point = newpoint;

    event->accept();

}

void CViewer::keyPressEvent(QKeyEvent* event) {

    // reset to cam to world coordinates
    if(event->key() == Qt::Key_Z) {

        CRigidMotion<double,3> id;
        m_view.SetTransformation(id);
        loadView(m_view);

        updateGL();

    }

    // turn color rendering on/off
    if(event->key() == Qt::Key_C) {

        m_show_color = !m_show_color;

        updateGL();

    }
}

void CViewer::loadView(const CView<double>& view) {

    CRigidMotion<double,3> F = view.GetTransformation();
    mat mF = mat(F);

    double mv[16];

    mv[0] = mF.Data().get()[0];
    mv[1] = -mF.Data().get()[1];
    mv[2] = -mF.Data().get()[2];
    mv[3] = mF.Data().get()[3];
    mv[4] = mF.Data().get()[4];
    mv[5] = -mF.Data().get()[5];
    mv[6] = -mF.Data().get()[6];
    mv[7] = mF.Data().get()[7];
    mv[8] = mF.Data().get()[8];
    mv[9] = -mF.Data().get()[9];
    mv[10] = -mF.Data().get()[10];
    mv[11] = mF.Data().get()[11];
    mv[12] = mF.Data().get()[12];
    mv[13] = -mF.Data().get()[13];
    mv[14] = -mF.Data().get()[14];
    mv[15] = mF.Data().get()[15];

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mv[0]);

}

void CViewer::loadProjectionMatrix() {

    // get camera intrinsics
    const CPinholeCam& cam = dynamic_cast<const CPinholeCam&>(m_view.GetCam());
    mat K = cam.GetProjectionMatrix();
    QSize ws = this->size();

    double proj[16];
    fill_n(&proj[0],16,0.0);
    proj[0] = 2.0*K.Get(0,0) / double(ws.width());
    proj[8] = 2.0*(K.Get(0,2)/ double(ws.width())) - 1.0;
    proj[5] = 2.0*K.Get(1,1) /  double(ws.height());
    proj[9] = 2.0*(K.Get(1,2)/  double(ws.height())) - 1.0;
    proj[10] = (-m_zfar - m_znear)/(m_zfar - m_znear);
    proj[14] = -2*m_zfar*m_znear/(m_zfar - m_znear);
    proj[11] = -1.0;
    proj[15] = 0.0;

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(&proj[0]);

}


matf CViewer::getDepthMap(const CView<double>& view) {

    // load view and paint
    loadView(view);    
    updateClipDepth(view,1.0);
    paintGL();

    // allocate result
    QSize ws = this->size();
    matf z(ws.height(),ws.width());

    // we need buffer in order to flip the array
    float* buffer = new float[z.NElems()];

    // read depths
    glReadPixels(0, 0,z.NCols(),z.NRows(),GL_DEPTH_COMPONENT,GL_FLOAT,buffer);

    // because we have updated the depths at the beginning, we don't need to grab them from the graphics card
    float zfmzn = m_zfar - m_znear;
    float zfpzn = m_zfar + m_znear;
    float zftzn = 2.0*m_zfar*m_znear;

    for(size_t i=0; i<z.NRows(); i++) {

        for(size_t j=0; j<z.NCols(); j++) {

            // normalized depth
            float zn = 2.0*buffer[(z.NRows()-i-1)*z.NCols() + j] - 1.0;

            z(i,j) = zftzn/(zfpzn - zn*zfmzn);

        }


    }

    delete [] buffer;

    // restore view and repaint
    loadView(m_view);
    updateClipDepth(m_view,NEAR_PLANE_TOL);
    paintGL();

    return z;

}

CTriMeshViewer::CTriMeshViewer(const CView<double>& view):
    R4R::CViewer(view),
    m_mesh(nullptr),
    m_bbox() {

}

void CTriMeshViewer::setMesh(CTriangleMesh* mesh) {

    m_mesh = mesh;

    // update bounding box
    m_bbox = mesh->BoundingBox();

    // update clip depths
    updateClipDepth(m_view,NEAR_PLANE_TOL);

}

void CTriMeshViewer::updateBoundingBox() {

    // recompute bounding box
    m_bbox = m_mesh->BoundingBox();

    // update clip depths
    updateClipDepth(m_view,NEAR_PLANE_TOL);

}

void CTriMeshViewer::updateClipDepth(const CView<double>& view, double tolerance) {

    vector<vec3f> corners = m_bbox.Corners();

    float minz = std::numeric_limits<float>::max();
    float maxz = -std::numeric_limits<float>::max();

    CRigidMotion<double,3> F = view.GetTransformation();

    for(u_int i=0; i<corners.size(); i++) {

        vec3f lc = F.Transform(corners.at(i));

        if(lc.Get(2)<minz)
            minz = lc.Get(2);

        if(lc.Get(2)>maxz)
            maxz = lc.Get(2);

    }

    double ntol, ftol;

    if(tolerance==0)
        ntol = 1.0;
    else
        ntol = tolerance;

    ftol = 1.0/ntol;

    m_znear = ntol*minz;
    m_zfar = ftol*maxz;

    // send new projection matrix to graphics card
    loadProjectionMatrix();

}

void CTriMeshViewer::mouseReleaseEvent(QMouseEvent *event) {

    // update clip depths
    this->updateClipDepth(m_view,NEAR_PLANE_TOL);

    updateGL();

    event->accept();

}

void CTriMeshViewer::paintGL() {

    if(m_mesh==nullptr)
        return;

    // set projection and modelview matrices from m_view
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // load mesh
    TriangleMesh::FaceIter f_it;
    TriangleMesh::FaceVertexIter fv_it;

    glBegin(GL_TRIANGLES);

    for (f_it=m_mesh->faces_begin(); f_it!=m_mesh->faces_end(); ++f_it) {

        // load normal
        glNormal3fv(&m_mesh->normal(f_it)[0]);

        for (fv_it = m_mesh->fv_iter(f_it.handle()); fv_it; ++fv_it) {

            // load color
            if(m_show_color) {

                OpenMesh::Vec3uc color = m_mesh->color(fv_it);
                glColor3ub(color[0],color[1],color[2]);

            }
            else
                glColor3ub(200,200,200);


            // load vertex
            glVertex3fv(&m_mesh->point(fv_it)[0]);

        }

    }

    glEnd();

}

void CTriMeshViewer::loadView(const CView<double>& view) {

    // load extrinsics
    CRigidMotion<double,3> F = view.GetTransformation();
    mat mF = mat(F);

    double mv[16];
    mv[0] = mF.Data().get()[0];
    mv[1] = -mF.Data().get()[1];
    mv[2] = -mF.Data().get()[2];
    mv[3] = mF.Data().get()[3];
    mv[4] = mF.Data().get()[4];
    mv[5] = -mF.Data().get()[5];
    mv[6] = -mF.Data().get()[6];
    mv[7] = mF.Data().get()[7];
    mv[8] = mF.Data().get()[8];
    mv[9] = -mF.Data().get()[9];
    mv[10] = -mF.Data().get()[10];
    mv[11] = mF.Data().get()[11];
    mv[12] = mF.Data().get()[12];
    mv[13] = -mF.Data().get()[13];
    mv[14] = -mF.Data().get()[14];
    mv[15] = mF.Data().get()[15];

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(&mv[0]);

}

}