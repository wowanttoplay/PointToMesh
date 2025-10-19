#pragma once
#include <QObject>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QTimer>
#include <QSize>
#include <QVector3D>
#include <atomic>
#include <memory>
#include "../../Model/Geometry.h"
#include "../../Settings/SettingsManager.h"

class Renderer;
class ShaderLibrary;
class Camera;

/**
 * RenderWorker manages a dedicated OpenGL rendering thread.
 * It owns a shared QOpenGLContext, an offscreen surface, and double-buffered FBOs.
 * The worker renders to one FBO while the GUI thread reads from the other.
 * After rendering, buffers are swapped and the current color texture ID is atomically published.
 */
class RenderWorker : public QObject {
    Q_OBJECT
public:
    explicit RenderWorker(QOpenGLContext* shareContext, QObject* parent = nullptr);
    ~RenderWorker() override;

    // Access the current color texture ID (thread-safe, read from GUI thread)
    GLuint currentColorTextureId() const { return m_currentColorTextureId.load(std::memory_order_acquire); }

public slots:
    // Lifecycle
    void initialize();
    void shutdown();
    
    // Resize the FBOs to match the window pixel size
    void resize(const QSize& pixelSize);
    
    // Rendering control
    void startRendering();
    void stopRendering();
    
    // Data updates (called from GUI thread, queued to worker thread)
    void updatePointCloud(PointCloudPtr cloud);
    void updateMesh(MeshPtr mesh);
    
    // Camera control (simple orbit/pan/dolly)
    void orbit(float dx, float dy);
    void pan(float dx, float dy);
    void zoom(float delta);
    
    // View settings
    void setShowPoints(bool on);
    void setShowMesh(bool on);
    void setWireframe(bool on);
    void setPointSize(float size);
    void setMeshColor(const QVector3D& color);
    void setPointColor(const QVector3D& color);
    void setWireColor(const QVector3D& color);

signals:
    void initialized();
    void errorOccurred(const QString& message);

private slots:
    void renderFrame();

private:
    void createFBOs(const QSize& size);
    void swapBuffers();

    // OpenGL resources (accessed only in worker thread)
    QOpenGLContext* m_shareContext;           // Main context to share with
    QOpenGLContext* m_context;                // Worker's own context
    QOffscreenSurface* m_surface;             // Offscreen surface for the worker context
    
    // Double-buffered FBOs
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo[2];
    int m_writeFboIndex;                      // Current FBO being written to
    
    // Atomic texture ID for thread-safe reading by GUI thread
    std::atomic<GLuint> m_currentColorTextureId;
    
    // Rendering state
    QTimer* m_renderTimer;
    QSize m_pixelSize;
    
    // Rendering components (owned by worker thread)
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<ShaderLibrary> m_shaders;
    std::unique_ptr<Camera> m_camera;
    RenderSettings m_settings;
    
    // Data to render
    PointCloudPtr m_pointCloud;
    MeshPtr m_mesh;
    bool m_pointsDirty;
    bool m_meshDirty;
};
