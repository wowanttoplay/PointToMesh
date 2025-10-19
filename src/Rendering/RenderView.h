#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QThread>
#include <QPoint>
#include <algorithm>
#include "../Model/Geometry.h"
#include "../Settings/SettingsManager.h"

class RenderWorker;
class QOpenGLTextureBlitter;

class RenderView : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit RenderView(QWidget* parent = nullptr);
    ~RenderView() override;

public slots:
    void setPointCloud(PointCloudPtr cloud);
    void setMesh(MeshPtr mesh);

    // Display toggles - forward to worker
    void setShowPoints(bool on);
    void setShowMesh(bool on);
    void setWireframe(bool on);
    void setPointSize(float s);
    void setMeshColor(const QVector3D& c);
    void setPointColor(const QVector3D& c);
    void setWireColor(const QVector3D& c);

public:
    float pointSize() const { return m_pointSize; }
    void adjustPointSize(float delta) { setPointSize(pointSize() + delta); }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private slots:
    void onWorkerInitialized();
    void onWorkerError(const QString& message);

private:
    // Rendering thread and worker
    QThread* m_renderThread;
    RenderWorker* m_renderWorker;
    
    // Texture blitter for displaying worker's output
    QOpenGLTextureBlitter* m_blitter;
    
    // Cached settings
    float m_pointSize;
    
    // Interaction state
    QPoint m_lastPos;
    bool m_leftDown {false};
    bool m_rightDown {false};

    // Helpers
    static void computeBounds(const PointCloudPtr& cloud, const MeshPtr& mesh, QVector3D& minP, QVector3D& maxP);
    void refitCameraToData(const PointCloudPtr& cloud, const MeshPtr& mesh);
};
