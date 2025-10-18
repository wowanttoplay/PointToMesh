#include "RenderView.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QtMath>
#include <algorithm>

RenderView::RenderView(QWidget* parent) : QOpenGLWidget(parent) {
    // Reasonable defaults
    m_cfg.pointSize = 3.0f;
}

void RenderView::setPointCloud(PointCloudPtr cloud) {
    {
        QMutexLocker lock(&m_mutex);
        m_cloud = std::move(cloud);
        m_pointsDirty = true;
    }
    refitCameraToData();
    update();
}

void RenderView::setMesh(MeshPtr mesh) {
    {
        QMutexLocker lock(&m_mutex);
        m_mesh = std::move(mesh);
        m_meshDirty = true;
    }
    refitCameraToData();
    update();
}

void RenderView::initializeGL() {
    // Renderer will configure global GL state; just log actual context
    if (auto* ctx = QOpenGLContext::currentContext()) {
        const QSurfaceFormat f = ctx->format();
        qInfo("Using OpenGL %d.%d %s profile", f.majorVersion(), f.minorVersion(), f.profile() == QSurfaceFormat::CoreProfile ? "core" : "compat");
    }

    QString err;
    if (!m_renderer.initialize(m_shaders, &err)) {
        qWarning("Renderer init failed: %s", qPrintable(err));
    }
}

void RenderView::resizeGL(int w, int h) {
    Q_UNUSED(w); Q_UNUSED(h);
}

void RenderView::computeBounds(const PointCloudPtr& cloud, const MeshPtr& mesh, QVector3D& minP, QVector3D& maxP) {
    bool first = true;
    auto acc = [&](const QVector3D& v){
        if (first) { minP = maxP = v; first = false; }
        else {
            minP.setX(std::min(minP.x(), v.x())); minP.setY(std::min(minP.y(), v.y())); minP.setZ(std::min(minP.z(), v.z()));
            maxP.setX(std::max(maxP.x(), v.x())); maxP.setY(std::max(maxP.y(), v.y())); maxP.setZ(std::max(maxP.z(), v.z()));
        }
    };
    if (cloud) { for (const auto& p : cloud->points) acc(p); }
    if (mesh)  { for (const auto& v : mesh->vertices) acc(v); }
    if (first) { minP = QVector3D(-1,-1,-1); maxP = QVector3D(1,1,1); }
}

void RenderView::refitCameraToData() {
    PointCloudPtr c; MeshPtr m;
    {
        QMutexLocker lock(&m_mutex);
        c = m_cloud; m = m_mesh;
    }
    QVector3D minP, maxP; computeBounds(c, m, minP, maxP);
    const QVector3D center = 0.5f*(minP+maxP);
    const QVector3D ext = 0.5f*(maxP-minP);
    const float radius = std::max({ext.x(), ext.y(), ext.z(), 0.5f});
    m_camera.setTarget(center);
    m_camera.setDistance(std::max(3.0f*radius, 0.5f));
    m_camera.setNearFar(0.01f, std::max(1000.0f, 10.0f*radius));
}

void RenderView::paintGL() {
    // Upload on demand
    if (m_pointsDirty) {
        PointCloudPtr c; { QMutexLocker lock(&m_mutex); c = m_cloud; m_pointsDirty = false; }
        m_renderer.updatePoints(c);
    }
    if (m_meshDirty) {
        MeshPtr m; { QMutexLocker lock(&m_mutex); m = m_mesh; m_meshDirty = false; }
        m_renderer.updateMesh(m);
    }

    // Use framebuffer pixel size to account for high-DPI displays
    const qreal dpr = devicePixelRatioF();
    const QSize pixelSize(qRound(width() * dpr), qRound(height() * dpr));
    m_renderer.draw(m_camera, m_cfg, pixelSize);
}

void RenderView::mousePressEvent(QMouseEvent* e) {
    m_lastPos = e->pos();
    if (e->button() == Qt::LeftButton) m_leftDown = true;
    if (e->button() == Qt::RightButton || e->button() == Qt::MiddleButton) m_rightDown = true;
}

void RenderView::mouseMoveEvent(QMouseEvent* e) {
    const QPoint cur = e->pos();
    const QPoint delta = cur - m_lastPos;
    m_lastPos = cur;
    if (width() == 0 || height() == 0) return;

    const float ndx = float(delta.x()) / float(width());
    const float ndy = float(delta.y()) / float(height());

    if (m_leftDown) {
        // Orbit
        m_camera.orbit(ndx, -ndy);
        update();
    } else if (m_rightDown) {
        // Pan
        m_camera.pan(ndx, ndy);
        update();
    }
}

void RenderView::wheelEvent(QWheelEvent* e) {
    // Typical mouse wheel delivers 120 per notched step
    const float steps = float(e->angleDelta().y()) / 120.0f;
    if (steps != 0.0f) {
        m_camera.zoom(steps);
        update();
    }
    e->accept();
}
