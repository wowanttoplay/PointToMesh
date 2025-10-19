#include "RenderView.h"

#include <QWheelEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <algorithm>
#include <QKeyEvent>

RenderView::RenderView(QWidget* parent) : QOpenGLWidget(parent) {
    // Initialize from persisted settings so colors/toggles apply at startup
    m_cfg = SettingsManager::instance().loadRenderSettings();
    // Receive keyboard focus for WASD controls
    setFocusPolicy(Qt::StrongFocus);
    // Smooth movement timer setup
    m_moveTimer.setTimerType(Qt::PreciseTimer);
    connect(&m_moveTimer, &QTimer::timeout, this, &RenderView::onMoveTick);
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
    // Ensure we receive subsequent key events
    if (!hasFocus()) setFocus(Qt::MouseFocusReason);
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

void RenderView::keyPressEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) { e->accept(); return; }
    bool prevMoving = (m_keyW || m_keyA || m_keyS || m_keyD);

    switch (e->key()) {
        case Qt::Key_W: m_keyW = true; break;
        case Qt::Key_S: m_keyS = true; break;
        case Qt::Key_A: m_keyA = true; break;
        case Qt::Key_D: m_keyD = true; break;
        case Qt::Key_Shift: m_shiftDown = true; break;
        default: QOpenGLWidget::keyPressEvent(e); return;
    }

    const bool nowMoving = (m_keyW || m_keyA || m_keyS || m_keyD);
    if (nowMoving && !prevMoving) {
        m_elapsed.restart();
        m_moveTimer.start(0); // as fast as possible
    }
    e->accept();
}

void RenderView::keyReleaseEvent(QKeyEvent* e) {
    if (e->isAutoRepeat()) { e->accept(); return; }

    switch (e->key()) {
        case Qt::Key_W: m_keyW = false; break;
        case Qt::Key_S: m_keyS = false; break;
        case Qt::Key_A: m_keyA = false; break;
        case Qt::Key_D: m_keyD = false; break;
        case Qt::Key_Shift: m_shiftDown = false; break;
        default: QOpenGLWidget::keyReleaseEvent(e); return;
    }

    if (!(m_keyW || m_keyA || m_keyS || m_keyD)) {
        m_moveTimer.stop();
        m_elapsed.invalidate();
    }
    e->accept();
}

void RenderView::onMoveTick() {
    // dt in seconds
    float dt = 0.0f;
    if (!m_elapsed.isValid()) {
        m_elapsed.restart();
        return; // wait for next tick to have dt
    }
    dt = static_cast<float>(m_elapsed.restart()) / 1000.0f;

    // Directions
    float fwd = (m_keyW ? 1.0f : 0.0f) + (m_keyS ? -1.0f : 0.0f);
    float right = (m_keyD ? 1.0f : 0.0f) + (m_keyA ? -1.0f : 0.0f);

    if (fwd == 0.0f && right == 0.0f) {
        // No movement keys held; stop timer to save CPU
        m_moveTimer.stop();
        m_elapsed.invalidate();
        return;
    }

    // Normalize diagonal speed
    if (fwd != 0.0f && right != 0.0f) {
        const float invSqrt2 = 0.70710678f;
        fwd *= invSqrt2;
        right *= invSqrt2;
    }

    // Base speed in normalized units per second (scaled again in Camera::moveHorizontal)
    float speed = 3.0f; // tweak to taste
    if (m_shiftDown) speed *= 3.0f;

    m_camera.moveHorizontal(fwd * speed * dt, right * speed * dt);
    update();
}
