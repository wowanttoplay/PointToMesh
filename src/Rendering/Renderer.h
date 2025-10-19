#pragma once
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include "../Settings/SettingsManager.h"
#include "../Model/Geometry.h"

class ShaderLibrary;
class Camera;

class Renderer : protected QOpenGLFunctions_4_1_Core {
public:
    Renderer();
    ~Renderer();

    bool initialize(ShaderLibrary& shaders, QString* error = nullptr);

    void updatePoints(const PointCloudPtr& cloud);
    void updateMesh(const MeshPtr& mesh);

    void draw(const Camera& cam, const RenderSettings& cfg, const QSize& viewport);

private:
    // Shaders
    QOpenGLShaderProgram* m_prog {nullptr};
    int m_locMvp {-1};
    int m_locColor {-1};

    // Points
    QOpenGLVertexArrayObject m_vaoPoints;
    QOpenGLBuffer m_vboPoints { QOpenGLBuffer::VertexBuffer };
    GLsizei m_pointCount {0};

    // Mesh
    QOpenGLVertexArrayObject m_vaoMesh;
    QOpenGLBuffer m_vboMesh { QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer m_iboMesh { QOpenGLBuffer::IndexBuffer };
    GLsizei m_indexCount {0};

    void setupPointVAO();
    void setupMeshVAO();
};
