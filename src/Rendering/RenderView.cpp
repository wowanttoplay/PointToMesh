#include "RenderView.h"

#include <QOpenGLContext>
#include <QPainter>

RenderView::RenderView(QWidget* parent) : QOpenGLWidget(parent) {}

void RenderView::setPointCloud(PointCloudPtr cloud) {
    QMutexLocker lock(&m_mutex);
    m_cloud = std::move(cloud);
    update();
}

void RenderView::setMesh(MeshPtr mesh) {
    QMutexLocker lock(&m_mutex);
    m_mesh = std::move(mesh);
    update();
}

void RenderView::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void RenderView::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}

void RenderView::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Placeholder rendering: nothing drawn yet. Hook up VBO/VAO & shaders here.
    // For quick visual feedback, draw text with QPainter (not performant, but fine for placeholders).
    QPainter p(this);
    p.setPen(Qt::white);
    p.drawText(rect(), Qt::AlignTop | Qt::AlignLeft, "RenderView ready");
    p.end();
}


