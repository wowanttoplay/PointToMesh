#pragma once
#include <QOpenGLWidget>
#include <QMutex>
#include <QPoint>
#include <algorithm>
#include <QTimer>
#include <QElapsedTimer>
#include "../Model/Geometry.h"
#include "../Settings/SettingsManager.h"
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
    void setShowNormals(bool on) { m_cfg.showNormals = on; update(); }
    void setShowMesh(bool on) { m_cfg.showMesh = on; update(); }
    void setWireframe(bool on) { m_cfg.wireframe = on; update(); }
    void setPointSize(float s) { m_cfg.pointSize = static_cast<int>(std::clamp(s, 1.0f, 20.0f)); update(); }
    void setMeshColor(const QVector3D& c) { m_cfg.meshColor = c; update(); }
    void setPointColor(const QVector3D& c) { m_cfg.pointColor = c; update(); }
    void setWireColor(const QVector3D& c) { m_cfg.wireColor = c; update(); }
    void setCameraSpeed(float v) { m_cfg.cameraSpeed = std::clamp(v, 0.01f, 1000.0f); }

public:
    float pointSize() const { return static_cast<float>(m_cfg.pointSize); }
    void adjustPointSize(float delta) { setPointSize(pointSize() + delta); }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;

private slots:
    void onMoveTick();

private:
    // Data (copied snapshots for bounds and uploads)
    QMutex m_mutex;
    PointCloudPtr m_cloud;
    MeshPtr m_mesh;
    bool m_pointsDirty {false};
    bool m_meshDirty {false};

    // Rendering
    RenderSettings m_cfg;
    Camera m_camera;
    ShaderLibrary m_shaders;
    Renderer m_renderer;

    // Interaction state
    QPoint m_lastPos;
    bool m_leftDown {false};
    bool m_rightDown {false};

    // Smooth movement state
    QTimer m_moveTimer;
    QElapsedTimer m_elapsed;
    bool m_keyW{false}, m_keyA{false}, m_keyS{false}, m_keyD{false};
    bool m_keyQ{false}, m_keyE{false};
    bool m_shiftDown{false};

    // Helpers
    static void computeBounds(const PointCloudPtr& cloud, const MeshPtr& mesh, QVector3D& minP, QVector3D& maxP);
    void refitCameraToData();
};
