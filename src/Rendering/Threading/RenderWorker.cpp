#include "RenderWorker.h"
#include "../Renderer.h"
#include "../ShaderLibrary.h"
#include "../Camera.h"
#include <QOpenGLFramebufferObjectFormat>
#include <QThread>
#include <algorithm>

RenderWorker::RenderWorker(QOpenGLContext* shareContext, QObject* parent)
    : QObject(parent)
    , m_shareContext(shareContext)
    , m_context(nullptr)
    , m_surface(nullptr)
    , m_writeFboIndex(0)
    , m_currentColorTextureId(0)
    , m_renderTimer(nullptr)
    , m_pixelSize(800, 600)
    , m_pointsDirty(false)
    , m_meshDirty(false)
{
    // Load initial render settings
    m_settings = SettingsManager::instance().loadRenderSettings();
}

RenderWorker::~RenderWorker() {
    // Ensure cleanup happens in the worker thread
    if (m_context) {
        shutdown();
    }
}

void RenderWorker::initialize() {
    qInfo("RenderWorker::initialize() starting in thread %p", QThread::currentThread());
    
    // Create offscreen surface
    m_surface = new QOffscreenSurface();
    m_surface->setFormat(m_shareContext->format());
    m_surface->create();
    
    if (!m_surface->isValid()) {
        emit errorOccurred("Failed to create offscreen surface");
        return;
    }
    
    // Create shared OpenGL context for this worker
    m_context = new QOpenGLContext();
    m_context->setFormat(m_shareContext->format());
    m_context->setShareContext(m_shareContext);
    
    if (!m_context->create()) {
        emit errorOccurred("Failed to create shared OpenGL context");
        delete m_surface;
        m_surface = nullptr;
        return;
    }
    
    // Make context current
    if (!m_context->makeCurrent(m_surface)) {
        emit errorOccurred("Failed to make context current");
        delete m_context;
        m_context = nullptr;
        delete m_surface;
        m_surface = nullptr;
        return;
    }
    
    // Initialize rendering components
    m_renderer = std::make_unique<Renderer>();
    m_shaders = std::make_unique<ShaderLibrary>();
    m_camera = std::make_unique<Camera>();
    
    QString error;
    if (!m_renderer->initialize(*m_shaders, &error)) {
        emit errorOccurred(QString("Renderer initialization failed: %1").arg(error));
        m_context->doneCurrent();
        return;
    }
    
    // Create FBOs
    createFBOs(m_pixelSize);
    
    // Create render timer (60 FPS)
    m_renderTimer = new QTimer(this);
    m_renderTimer->setInterval(1000 / 60);  // 16.67ms for 60 FPS
    connect(m_renderTimer, &QTimer::timeout, this, &RenderWorker::renderFrame);
    
    m_context->doneCurrent();
    
    qInfo("RenderWorker::initialize() completed successfully");
    emit initialized();
}

void RenderWorker::shutdown() {
    qInfo("RenderWorker::shutdown() starting");
    
    if (m_renderTimer) {
        m_renderTimer->stop();
        delete m_renderTimer;
        m_renderTimer = nullptr;
    }
    
    if (m_context) {
        m_context->makeCurrent(m_surface);
        
        // Clean up FBOs
        m_fbo[0].reset();
        m_fbo[1].reset();
        
        // Clean up rendering components
        m_renderer.reset();
        m_shaders.reset();
        m_camera.reset();
        
        m_context->doneCurrent();
        delete m_context;
        m_context = nullptr;
    }
    
    if (m_surface) {
        delete m_surface;
        m_surface = nullptr;
    }
    
    m_currentColorTextureId.store(0, std::memory_order_release);
    
    qInfo("RenderWorker::shutdown() completed");
}

void RenderWorker::resize(const QSize& pixelSize) {
    if (pixelSize.width() <= 0 || pixelSize.height() <= 0) {
        return;
    }
    
    m_pixelSize = pixelSize;
    
    if (m_context) {
        m_context->makeCurrent(m_surface);
        createFBOs(m_pixelSize);
        m_context->doneCurrent();
    }
}

void RenderWorker::startRendering() {
    if (m_renderTimer) {
        qInfo("RenderWorker::startRendering()");
        m_renderTimer->start();
    }
}

void RenderWorker::stopRendering() {
    if (m_renderTimer) {
        qInfo("RenderWorker::stopRendering()");
        m_renderTimer->stop();
    }
}

void RenderWorker::updatePointCloud(PointCloudPtr cloud) {
    m_pointCloud = cloud;
    m_pointsDirty = true;
}

void RenderWorker::updateMesh(MeshPtr mesh) {
    m_mesh = mesh;
    m_meshDirty = true;
}

void RenderWorker::orbit(float dx, float dy) {
    if (m_camera) {
        m_camera->orbit(dx, dy);
    }
}

void RenderWorker::pan(float dx, float dy) {
    if (m_camera) {
        m_camera->pan(dx, dy);
    }
}

void RenderWorker::zoom(float delta) {
    if (m_camera) {
        m_camera->zoom(delta);
    }
}

void RenderWorker::setCameraTarget(const QVector3D& target) {
    if (m_camera) {
        m_camera->setTarget(target);
    }
}

void RenderWorker::setCameraDistance(float distance) {
    if (m_camera) {
        m_camera->setDistance(distance);
    }
}

void RenderWorker::setCameraNearFar(float near, float far) {
    if (m_camera) {
        m_camera->setNearFar(near, far);
    }
}

void RenderWorker::setShowPoints(bool on) {
    m_settings.showPoints = on;
}

void RenderWorker::setShowMesh(bool on) {
    m_settings.showMesh = on;
}

void RenderWorker::setWireframe(bool on) {
    m_settings.wireframe = on;
}

void RenderWorker::setPointSize(float size) {
    m_settings.pointSize = static_cast<int>(std::clamp(size, 1.0f, 20.0f));
}

void RenderWorker::setMeshColor(const QVector3D& color) {
    m_settings.meshColor = color;
}

void RenderWorker::setPointColor(const QVector3D& color) {
    m_settings.pointColor = color;
}

void RenderWorker::setWireColor(const QVector3D& color) {
    m_settings.wireColor = color;
}

void RenderWorker::renderFrame() {
    if (!m_context || !m_renderer || !m_fbo[0] || !m_fbo[1]) {
        return;
    }
    
    m_context->makeCurrent(m_surface);
    
    // Upload data if dirty
    if (m_pointsDirty) {
        m_renderer->updatePoints(m_pointCloud);
        m_pointsDirty = false;
    }
    
    if (m_meshDirty) {
        m_renderer->updateMesh(m_mesh);
        m_meshDirty = false;
    }
    
    // Bind the write FBO and render to it
    QOpenGLFramebufferObject* writeFbo = m_fbo[m_writeFboIndex].get();
    if (writeFbo->bind()) {
        m_renderer->draw(*m_camera, m_settings, m_pixelSize);
        writeFbo->release();
        
        // Swap buffers and publish the new texture ID
        swapBuffers();
    }
    
    m_context->doneCurrent();
}

void RenderWorker::createFBOs(const QSize& size) {
    // Configure FBO format with color and depth attachments
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(0);  // No MSAA for now, can be added later
    
    // Create two FBOs for double-buffering
    m_fbo[0] = std::make_unique<QOpenGLFramebufferObject>(size, format);
    m_fbo[1] = std::make_unique<QOpenGLFramebufferObject>(size, format);
    
    if (!m_fbo[0]->isValid() || !m_fbo[1]->isValid()) {
        qWarning("Failed to create valid FBOs");
        m_fbo[0].reset();
        m_fbo[1].reset();
        return;
    }
    
    // Initialize the write index
    m_writeFboIndex = 0;
    
    // Publish the initial read FBO's texture
    m_currentColorTextureId.store(m_fbo[1]->texture(), std::memory_order_release);
    
    qInfo("Created FBOs: %dx%d", size.width(), size.height());
}

void RenderWorker::swapBuffers() {
    // Swap write/read indices
    m_writeFboIndex = 1 - m_writeFboIndex;
    
    // The FBO we just finished writing to is now the read FBO
    int readFboIndex = 1 - m_writeFboIndex;
    
    // Atomically publish the new texture ID for the GUI thread to read
    GLuint textureId = m_fbo[readFboIndex]->texture();
    m_currentColorTextureId.store(textureId, std::memory_order_release);
}
