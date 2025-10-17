#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMutex>
#include "../Model/Geometry.h"

class RenderView : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT
public:
    explicit RenderView(QWidget* parent = nullptr);

public slots:
    void setPointCloud(PointCloudPtr cloud);
    void setMesh(MeshPtr mesh);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QMutex m_mutex;
    PointCloudPtr m_cloud;
    MeshPtr m_mesh;
};


