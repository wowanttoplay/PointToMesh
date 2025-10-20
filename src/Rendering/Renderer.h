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
    // Shaders (basic)
    QOpenGLShaderProgram* m_prog {nullptr};
    int m_locMvp {-1};
    int m_locColor {-1};
    int m_locPointSize {-1};
    int m_locClipPlane {-1};

    // Shaders (normals visualization)
    QOpenGLShaderProgram* m_progNormals {nullptr};
    int m_locMvpN {-1};
    int m_locColorN {-1};
    int m_locNormalLen {-1};
    int m_locClipPlaneN {-1};

    // Points
    QOpenGLVertexArrayObject m_vaoPoints;
    QOpenGLBuffer m_vboPoints { QOpenGLBuffer::VertexBuffer };       // positions
    QOpenGLBuffer m_vboPointNormals { QOpenGLBuffer::VertexBuffer }; // normals
    GLsizei m_pointCount {0};
    bool m_hasPointNormal {false};

    // Mesh
    QOpenGLVertexArrayObject m_vaoMesh;
    QOpenGLBuffer m_vboMesh { QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer m_iboMesh { QOpenGLBuffer::IndexBuffer };
    GLsizei m_indexCount {0};

    void setupPointVAO();
    void setupMeshVAO();
};
