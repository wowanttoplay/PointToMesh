#include "Renderer.h"
#include "ShaderLibrary.h"
#include "Camera.h"

#include <QOpenGLShaderProgram>
#include <QVector3D>

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

bool Renderer::initialize(ShaderLibrary& shaders, QString* error) {
    initializeOpenGLFunctions();

    // Global GL state
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);

    // Ensure the required shader program is available by name. File discovery is handled by ShaderLibrary.
    if (!shaders.ensureProgram("basic", error)) {
        return false;
    }
    m_prog = shaders.get("basic");
    m_locMvp = m_prog->uniformLocation("u_mvp");
    m_locColor = m_prog->uniformLocation("u_color");
    m_locPointSize = m_prog->uniformLocation("u_pointSize");
    m_locClipPlane = m_prog->uniformLocation("u_clipPlane");

    // Normals visualization program (geometry shader based)
    QString nerr;
    if (!shaders.ensureProgram("normals", &nerr)) {
        if (error) *error = QStringLiteral("Normals shader failed: %1").arg(nerr);
        return false;
    }
    m_progNormals = shaders.get("normals");
    m_locMvpN = m_progNormals->uniformLocation("u_mvp");
    m_locColorN = m_progNormals->uniformLocation("u_color");
    m_locNormalLen = m_progNormals->uniformLocation("u_normalLen");
    m_locClipPlaneN = m_progNormals->uniformLocation("u_clipPlane");

    // Create buffers/VAOs
    if (!m_vaoPoints.isCreated()) m_vaoPoints.create();
    if (!m_vboPoints.isCreated()) m_vboPoints.create();
    if (!m_vboPointNormals.isCreated()) m_vboPointNormals.create();

    if (!m_vaoMesh.isCreated()) m_vaoMesh.create();
    if (!m_vboMesh.isCreated()) m_vboMesh.create();
    if (!m_iboMesh.isCreated()) m_iboMesh.create();

    setupPointVAO();
    setupMeshVAO();
    return true;
}

void Renderer::setupPointVAO() {
    m_vaoPoints.bind();
    // Positions at location 0
    m_vboPoints.bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<void*>(0));
    m_vboPoints.release();
    // Normals at location 1 (optional)
    m_vboPointNormals.bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D), reinterpret_cast<void*>(0));
    m_vboPointNormals.release();
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
    m_hasPointNormal = false;
    // Positions
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

    // Normals (optional)
    m_vboPointNormals.bind();
    if (cloud && !cloud->normals.empty() && cloud->normals.size() >= cloud->points.size()) {
        const auto nbytes = static_cast<int>(cloud->points.size() * sizeof(QVector3D));
        m_vboPointNormals.allocate(nbytes);
        m_vboPointNormals.write(0, cloud->normals.data(), nbytes);
        m_hasPointNormal = (m_pointCount > 0);
    } else {
        m_vboPointNormals.allocate(0);
        m_hasPointNormal = false;
    }
    m_vboPointNormals.release();
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

void Renderer::draw(const Camera& cam, const RenderSettings& cfg, const QSize& viewport) {
    if (!m_prog) return;

    glViewport(0, 0, viewport.width(), viewport.height());
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (cfg.clipPlaneParams.clipEnabled) {
        glEnable(GL_CLIP_DISTANCE0);
    } else {
        glDisable(GL_CLIP_DISTANCE0);
    }

    const float aspect = viewport.width() > 0 ? float(viewport.width())/float(std::max(1, viewport.height())) : 1.0f;
    const QMatrix4x4 mvp = cam.projMatrix(aspect) * cam.viewMatrix();

    m_prog->bind();
    m_prog->setUniformValue(m_locMvp, mvp);
    if (m_locClipPlane >= 0) {
        m_prog->setUniformValue(m_locClipPlane, cfg.clipPlaneParams.clipPlane);
    }

    // Mesh fill pass
    if (cfg.showMesh && m_indexCount > 0) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        m_prog->setUniformValue(m_locColor, cfg.meshColor);
        m_vaoMesh.bind();
        m_iboMesh.bind();
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        m_iboMesh.release();
        m_vaoMesh.release();
    }

    // Mesh wireframe overlay pass (independent of showMesh)
    if (cfg.wireframe && m_indexCount > 0) {
        // Slightly bias to reduce z-fighting against the filled surface
        glEnable(GL_POLYGON_OFFSET_LINE);
        glPolygonOffset(-1.0f, -1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        m_prog->setUniformValue(m_locColor, cfg.wireColor);
        m_vaoMesh.bind();
        m_iboMesh.bind();
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        m_iboMesh.release();
        m_vaoMesh.release();

        // Restore state
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDisable(GL_POLYGON_OFFSET_LINE);
    }

    // Points
    if (cfg.showPoints && m_pointCount > 0) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        // Set color and point size via uniforms
        m_prog->setUniformValue(m_locColor, cfg.pointColor);
        m_prog->setUniformValue(m_locPointSize, static_cast<float>(cfg.pointSize));
        m_vaoPoints.bind();
        glDrawArrays(GL_POINTS, 0, m_pointCount);
        m_vaoPoints.release();
    }

    m_prog->release();

    // Normals visualization for points (requires normals and program)
    if (cfg.showNormals && m_hasPointNormal && m_progNormals && m_pointCount > 0) {
        m_progNormals->bind();
        m_progNormals->setUniformValue(m_locMvpN, mvp);
        if (m_locClipPlaneN >= 0) {
            m_progNormals->setUniformValue(m_locClipPlaneN, cfg.clipPlaneParams.clipPlane);
        }
        // A default bluish color for normals
        m_progNormals->setUniformValue(m_locColorN, QVector3D(0.2f, 0.6f, 1.0f));
        // Fixed length in world units; could be made configurable
        m_progNormals->setUniformValue(m_locNormalLen, 0.02f);
        m_vaoPoints.bind();
        glDrawArrays(GL_POINTS, 0, m_pointCount);
        m_vaoPoints.release();
        m_progNormals->release();
    }
}
