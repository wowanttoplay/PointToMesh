#include "Renderer.h"
#include "ShaderLibrary.h"
#include "Camera.h"

#include <QOpenGLShaderProgram>
#include <QVector3D>

namespace {
constexpr const char* kBasicVert = R"(#version 330 core
layout(location=0) in vec3 a_pos;
uniform mat4 u_mvp;
void main(){ gl_Position = u_mvp * vec4(a_pos, 1.0); }
)";

constexpr const char* kBasicFrag = R"(#version 330 core
uniform vec3 u_color;
out vec4 fragColor;
void main(){ fragColor = vec4(u_color, 1.0); }
)";
}

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

bool Renderer::initialize(ShaderLibrary& shaders, QString* error) {
    initializeOpenGLFunctions();

    // Prepare shader
    if (!shaders.get("basic")) {
        if (!shaders.addProgram("basic", kBasicVert, kBasicFrag, error)) {
            return false;
        }
    }
    m_prog = shaders.get("basic");
    m_locMvp = m_prog->uniformLocation("u_mvp");
    m_locColor = m_prog->uniformLocation("u_color");

    // Create buffers/VAOs
    if (!m_vaoPoints.isCreated()) m_vaoPoints.create();
    if (!m_vboPoints.isCreated()) m_vboPoints.create();

    if (!m_vaoMesh.isCreated()) m_vaoMesh.create();
    if (!m_vboMesh.isCreated()) m_vboMesh.create();
    if (!m_iboMesh.isCreated()) m_iboMesh.create();

    setupPointVAO();
    setupMeshVAO();
    return true;
}

void Renderer::setupPointVAO() {
    m_vaoPoints.bind();
    m_vboPoints.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<void*>(0));
    m_vboPoints.release();
    m_vaoPoints.release();
}

void Renderer::setupMeshVAO() {
    m_vaoMesh.bind();
    m_vboMesh.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<void*>(0));
    m_vboMesh.release();
    m_vaoMesh.release();
}

void Renderer::updatePoints(const PointCloudPtr& cloud) {
    m_pointCount = 0;
    m_vboPoints.bind();
    if (cloud && !cloud->points.empty()) {
        const auto bytes = static_cast<int>(cloud->points.size() * sizeof(QVector3D));
        m_vboPoints.allocate(bytes);
        m_vboPoints.write(0, cloud->points.data(), bytes);
        m_pointCount = static_cast<GLsizei>(cloud->points.size());
    } else {
        m_vboPoints.allocate(0);
    }
    m_vboPoints.release();
}

void Renderer::updateMesh(const MeshPtr& mesh) {
    m_indexCount = 0;
    // Vertices
    m_vboMesh.bind();
    if (mesh && !mesh->vertices.empty()) {
        const auto vbytes = static_cast<int>(mesh->vertices.size() * sizeof(QVector3D));
        m_vboMesh.allocate(vbytes);
        m_vboMesh.write(0, mesh->vertices.data(), vbytes);
    } else {
        m_vboMesh.allocate(0);
    }
    m_vboMesh.release();

    // Indices
    m_iboMesh.bind();
    if (mesh && !mesh->indices.empty()) {
        const auto ibytes = static_cast<int>(mesh->indices.size() * sizeof(std::uint32_t));
        m_iboMesh.allocate(ibytes);
        m_iboMesh.write(0, mesh->indices.data(), ibytes);
        m_indexCount = static_cast<GLsizei>(mesh->indices.size());
    } else {
        m_iboMesh.allocate(0);
    }
    m_iboMesh.release();
}

void Renderer::draw(const Camera& cam, const RenderConfig& cfg, const QSize& viewport) {
    if (!m_prog) return;

    glViewport(0, 0, viewport.width(), viewport.height());
    glEnable(GL_DEPTH_TEST);

    const float aspect = viewport.width() > 0 ? float(viewport.width())/float(std::max(1, viewport.height())) : 1.0f;
    QMatrix4x4 mvp = cam.projMatrix(aspect) * cam.viewMatrix();

    m_prog->bind();
    m_prog->setUniformValue(m_locMvp, mvp);

    // Mesh
    if (cfg.showMesh && m_indexCount > 0) {
        if (cfg.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        m_prog->setUniformValue(m_locColor, cfg.meshColor);
        m_vaoMesh.bind();
        m_iboMesh.bind();
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        m_iboMesh.release();
        m_vaoMesh.release();
        if (cfg.wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Points
    if (cfg.showPoints && m_pointCount > 0) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        glPointSize(cfg.pointSize);
        m_prog->setUniformValue(m_locColor, cfg.pointColor);
        m_vaoPoints.bind();
        glDrawArrays(GL_POINTS, 0, m_pointCount);
        m_vaoPoints.release();
    }

    m_prog->release();
}

