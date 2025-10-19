#include "RenderView.h"
#include "Threading/RenderWorker.h"

#include <QWheelEvent>
#include <QOpenGLContext>
#include <QSurfaceFormat>
#include <QOpenGLTextureBlitter>
#include <QMatrix4x4>
#include <algorithm>

RenderView::RenderView(QWidget* parent) 
    : QOpenGLWidget(parent)
    , m_renderThread(nullptr)
    , m_renderWorker(nullptr)
    , m_blitter(nullptr)
{
    // Initialize from persisted settings
    RenderSettings cfg = SettingsManager::instance().loadRenderSettings();
    m_pointSize = static_cast<float>(cfg.pointSize);
}

RenderView::~RenderView() {
    // Stop rendering and clean up worker
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "stopRendering", Qt::BlockingQueuedConnection);
        QMetaObject::invokeMethod(m_renderWorker, "shutdown", Qt::BlockingQueuedConnection);
    }
    
    if (m_renderThread) {
        m_renderThread->quit();
        m_renderThread->wait();
        delete m_renderThread;
    }
    
    if (m_blitter) {
        makeCurrent();
        delete m_blitter;
        doneCurrent();
    }
}

void RenderView::setPointCloud(PointCloudPtr cloud) {
    if (m_renderWorker) {
        refitCameraToData(cloud, nullptr);
        QMetaObject::invokeMethod(m_renderWorker, "updatePointCloud", Qt::QueuedConnection, Q_ARG(PointCloudPtr, cloud));
    }
}

void RenderView::setMesh(MeshPtr mesh) {
    if (m_renderWorker) {
        refitCameraToData(nullptr, mesh);
        QMetaObject::invokeMethod(m_renderWorker, "updateMesh", Qt::QueuedConnection, Q_ARG(MeshPtr, mesh));
    }
}

void RenderView::setShowPoints(bool on) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setShowPoints", Qt::QueuedConnection, Q_ARG(bool, on));
    }
}

void RenderView::setShowMesh(bool on) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setShowMesh", Qt::QueuedConnection, Q_ARG(bool, on));
    }
}

void RenderView::setWireframe(bool on) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setWireframe", Qt::QueuedConnection, Q_ARG(bool, on));
    }
}

void RenderView::setPointSize(float s) {
    m_pointSize = std::clamp(s, 1.0f, 20.0f);
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setPointSize", Qt::QueuedConnection, Q_ARG(float, m_pointSize));
    }
}

void RenderView::setMeshColor(const QVector3D& c) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setMeshColor", Qt::QueuedConnection, Q_ARG(QVector3D, c));
    }
}

void RenderView::setPointColor(const QVector3D& c) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setPointColor", Qt::QueuedConnection, Q_ARG(QVector3D, c));
    }
}

void RenderView::setWireColor(const QVector3D& c) {
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setWireColor", Qt::QueuedConnection, Q_ARG(QVector3D, c));
    }
}

void RenderView::initializeGL() {
    initializeOpenGLFunctions();
    
    // Log OpenGL context info
    if (auto* ctx = QOpenGLContext::currentContext()) {
        const QSurfaceFormat f = ctx->format();
        qInfo("GUI thread OpenGL %d.%d %s profile", f.majorVersion(), f.minorVersion(), 
              f.profile() == QSurfaceFormat::CoreProfile ? "core" : "compat");
    }
    
    // Create texture blitter for displaying worker's output
    m_blitter = new QOpenGLTextureBlitter();
    if (!m_blitter->create()) {
        qWarning("Failed to create texture blitter");
        delete m_blitter;
        m_blitter = nullptr;
        return;
    }
    
    // Create render thread and worker
    m_renderThread = new QThread(this);
    m_renderWorker = new RenderWorker(context());
    m_renderWorker->moveToThread(m_renderThread);
    
    // Connect worker signals
    connect(m_renderWorker, &RenderWorker::initialized, this, &RenderView::onWorkerInitialized);
    connect(m_renderWorker, &RenderWorker::errorOccurred, this, &RenderView::onWorkerError);
    
    // Start the thread and initialize the worker
    m_renderThread->start();
    QMetaObject::invokeMethod(m_renderWorker, "initialize", Qt::QueuedConnection);
}

void RenderView::resizeGL(int w, int h) {
    if (m_renderWorker) {
        // Account for high-DPI displays
        const qreal dpr = devicePixelRatioF();
        const QSize pixelSize(qRound(w * dpr), qRound(h * dpr));
        QMetaObject::invokeMethod(m_renderWorker, "resize", Qt::QueuedConnection, Q_ARG(QSize, pixelSize));
    }
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

void RenderView::refitCameraToData(const PointCloudPtr& cloud, const MeshPtr& mesh) {
    QVector3D minP, maxP; 
    computeBounds(cloud, mesh, minP, maxP);
    
    const QVector3D center = 0.5f*(minP+maxP);
    const QVector3D ext = 0.5f*(maxP-minP);
    const float radius = std::max({ext.x(), ext.y(), ext.z(), 0.5f});
    
    // Send camera updates to worker
    if (m_renderWorker) {
        QMetaObject::invokeMethod(m_renderWorker, "setCameraTarget", Qt::QueuedConnection, Q_ARG(QVector3D, center));
        QMetaObject::invokeMethod(m_renderWorker, "setCameraDistance", Qt::QueuedConnection, Q_ARG(float, std::max(3.0f*radius, 0.5f)));
        QMetaObject::invokeMethod(m_renderWorker, "setCameraNearFar", Qt::QueuedConnection, Q_ARG(float, 0.01f), Q_ARG(float, std::max(1000.0f, 10.0f*radius)));
    }
}

void RenderView::paintGL() {
    if (!m_blitter || !m_renderWorker) {
        return;
    }
    
    // Get the current color texture ID from the worker (thread-safe atomic read)
    GLuint textureId = m_renderWorker->currentColorTextureId();
    
    if (textureId == 0) {
        // Worker hasn't produced a texture yet, clear to background color
        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }
    
    // Blit the worker's texture to the default framebuffer
    const QRect targetRect(QPoint(0, 0), size());
    const QSize texSize = size();  // Texture size matches window size
    const QMatrix4x4 target = QOpenGLTextureBlitter::targetTransform(targetRect, QRect(QPoint(0, 0), texSize));
    
    m_blitter->bind(GL_TEXTURE_2D);
    m_blitter->blit(textureId, target, QOpenGLTextureBlitter::OriginBottomLeft);
    m_blitter->release();
    
    // Request another frame
    update();
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
    if (width() == 0 || height() == 0 || !m_renderWorker) return;

    const float ndx = float(delta.x()) / float(width());
    const float ndy = float(delta.y()) / float(height());

    if (m_leftDown) {
        // Orbit
        QMetaObject::invokeMethod(m_renderWorker, "orbit", Qt::QueuedConnection, 
                                 Q_ARG(float, ndx), Q_ARG(float, -ndy));
    } else if (m_rightDown) {
        // Pan
        QMetaObject::invokeMethod(m_renderWorker, "pan", Qt::QueuedConnection, 
                                 Q_ARG(float, ndx), Q_ARG(float, ndy));
    }
}

void RenderView::wheelEvent(QWheelEvent* e) {
    if (!m_renderWorker) return;
    
    // Typical mouse wheel delivers 120 per notched step
    const float steps = float(e->angleDelta().y()) / 120.0f;
    if (steps != 0.0f) {
        QMetaObject::invokeMethod(m_renderWorker, "zoom", Qt::QueuedConnection, Q_ARG(float, steps));
    }
    e->accept();
}

void RenderView::onWorkerInitialized() {
    qInfo("RenderWorker initialized successfully");
    
    // Start rendering
    QMetaObject::invokeMethod(m_renderWorker, "startRendering", Qt::QueuedConnection);
    
    // Initialize with saved settings
    RenderSettings cfg = SettingsManager::instance().loadRenderSettings();
    setShowPoints(cfg.showPoints);
    setShowMesh(cfg.showMesh);
    setWireframe(cfg.wireframe);
    setPointSize(static_cast<float>(cfg.pointSize));
    setMeshColor(cfg.meshColor);
    setPointColor(cfg.pointColor);
    setWireColor(cfg.wireColor);
}

void RenderView::onWorkerError(const QString& message) {
    qWarning("RenderWorker error: %s", qPrintable(message));
}
