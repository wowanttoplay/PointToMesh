#include "ProcessingWorker.h"

#include <QString>
#include <memory>
#include <QVector3D>

#include "../DataProcess/PointCloudProcessor.h"
#include "../Model/Geometry.h"

#include <CGAL/Surface_mesh.h>

ProcessingWorker::ProcessingWorker(std::unique_ptr<PointCloudProcessor> proc, QObject* parent)
    : QObject(parent), m_proc(std::move(proc)) {}

ProcessingWorker::~ProcessingWorker() = default;

void ProcessingWorker::importPointCloud(const QString& filePath) {
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const std::string path = filePath.toStdString();
    if (!m_proc->loadPointCloud(path)) {
        emit logMessage(QStringLiteral("Failed to load point cloud: ") + filePath);
        return;
    }
    emit logMessage(QStringLiteral("Loaded point cloud: ") + filePath);

    // Convert to model
    const auto& pc = m_proc->getPointCloud();
    auto model = std::make_shared<PointCloudModel>();
    model->points.reserve(pc.size());
    model->normals.reserve(pc.size());
    for (const auto& pn : pc) {
        const auto& p = pn.first;  // CGAL point
        const auto& n = pn.second; // CGAL normal (may be NULL_VECTOR)
        model->points.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
        if (n != CGAL::NULL_VECTOR) {
            model->normals.emplace_back(static_cast<float>(n.x()), static_cast<float>(n.y()), static_cast<float>(n.z()));
        } else {
            model->normals.emplace_back(0.0f, 0.0f, 0.0f);
        }
    }
    emit pointCloudReady(model);
}

void ProcessingWorker::reconstructWithParams(MeshGenerationMethod method, BaseInputParameter* params) {
    // Take ownership in a unique_ptr to ensure deletion in this thread
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }

    const auto methodName = [method]() -> QString {
        switch (method) {
            case MeshGenerationMethod::POISSON_RECONSTRUCTION: return QStringLiteral("Poisson Reconstruction");
            case MeshGenerationMethod::SCALE_SPACE_RECONSTRUCTION: return QStringLiteral("Scale-Space Reconstruction");
            case MeshGenerationMethod::ADVANCING_FRONT_RECONSTRUCTION: return QStringLiteral("Advancing Front Reconstruction");
            default: return QStringLiteral("Unknown Reconstruction");
        }
    }();

    if (method == MeshGenerationMethod::POISSON_RECONSTRUCTION && !m_proc->hasNormals()) {
        emit logMessage(QStringLiteral("Estimating normals (required for Poisson)..."));
        if (!m_proc->estimateNormals()) {
            emit logMessage(QStringLiteral("Normal estimation failed."));
            return;
        }
    }

    emit logMessage(QStringLiteral("Running ") + methodName + QStringLiteral(" with parameters..."));
    if (!m_proc->processToMesh(method, guard.get())) {
        emit logMessage(methodName + QStringLiteral(" failed."));
        return;
    }

    const auto& mesh = m_proc->getMesh();

    auto model = std::make_shared<MeshModel>();
    const auto nv = static_cast<std::size_t>(num_vertices(mesh));
    model->vertices.reserve(nv);

    std::vector<std::uint32_t> vmap(nv);
    std::uint32_t idx = 0;
    for (auto v : mesh.vertices()) {
        const auto& p = mesh.point(v);
        model->vertices.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
        vmap[static_cast<std::size_t>(v.idx())] = idx++;
    }

    for (auto f : mesh.faces()) {
        std::vector<std::uint32_t> faceIdx;
        for (auto v : CGAL::vertices_around_face(mesh.halfedge(f), mesh)) {
            faceIdx.push_back(vmap[static_cast<std::size_t>(v.idx())]);
        }
        if (faceIdx.size() == 3) {
            model->indices.insert(model->indices.end(), { faceIdx[0], faceIdx[1], faceIdx[2] });
        }
    }

    emit meshReady(model);
    emit logMessage(methodName + QStringLiteral(" finished."));
}

void ProcessingWorker::exportMeshTo(const QString& filePath, bool withNormals) {
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const std::string path = filePath.toStdString();
    if (!m_proc->exportMesh(path, withNormals)) {
        emit logMessage(QStringLiteral("Export failed: ") + filePath);
        return;
    }
    emit logMessage(QStringLiteral("Exported mesh to: ") + filePath);
}
