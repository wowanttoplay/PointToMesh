#pragma once
#include <QOpenGLWidget>
#include <QMutex>
#include <QPoint>
#include "../Model/Geometry.h"
#include "RenderConfig.h"
#include "Camera.h"
#include "ShaderLibrary.h"
#include "Renderer.h"

class RenderView : public QOpenGLWidget {
    Q_OBJECT
public:
    explicit RenderView(QWidget* parent = nullptr);

public slots:
    void setPointCloud(PointCloudPtr cloud);
    void setMesh(MeshPtr mesh);

    // Display toggles
    void setShowPoints(bool on) { m_cfg.showPoints = on; update(); }
    void setShowMesh(bool on) { m_cfg.showMesh = on; update(); }
    void setWireframe(bool on) { m_cfg.wireframe = on; update(); }
    void setPointSize(float s) { m_cfg.pointSize = std::clamp(s, 1.0f, 20.0f); update(); }

public:
    float pointSize() const { return m_cfg.pointSize; }
    void adjustPointSize(float delta) { setPointSize(m_cfg.pointSize + delta); }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;

private:
    // Data (copied snapshots for bounds and uploads)
    QMutex m_mutex;
    PointCloudPtr m_cloud;
    MeshPtr m_mesh;
    bool m_pointsDirty {false};
    bool m_meshDirty {false};

    // Rendering
    RenderConfig m_cfg;
    Camera m_camera;
    ShaderLibrary m_shaders;
    Renderer m_renderer;

    // Interaction state
    QPoint m_lastPos;
    bool m_leftDown {false};
    bool m_rightDown {false};

    // Helpers
    static void computeBounds(const PointCloudPtr& cloud, const MeshPtr& mesh, QVector3D& minP, QVector3D& maxP);
    void refitCameraToData();
};
