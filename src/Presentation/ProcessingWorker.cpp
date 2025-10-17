#include "ProcessingWorker.h"

#include <QString>
#include <memory>
#include <QVector3D>

#include "../DataProcess/PointCloudProcessor.h"
#include "../DataProcess/CGALPointCloudProcessor.h"
#include "../Model/Geometry.h"

#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/helpers.h>
#include <CGAL/property_map.h>
#include <boost/graph/graph_traits.hpp>

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
    for (const auto& pn : pc) {
        const auto& p = pn.first; // CGAL point
        model->points.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
    }
    emit pointCloudReady(model);
}

void ProcessingWorker::reconstruct() {
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    if (!m_proc->hasNormals()) {
        emit logMessage(QStringLiteral("Estimating normals..."));
        if (!m_proc->estimateNormals()) {
            emit logMessage(QStringLiteral("Normal estimation failed."));
            return;
        }
    }
    emit logMessage(QStringLiteral("Running surface reconstruction..."));
    if (!m_proc->processToMesh()) {
        emit logMessage(QStringLiteral("Reconstruction failed."));
        return;
    }

    // Convert mesh to model
    const auto& mesh = m_proc->getMesh();

    // Build vertex index map using Vertex_index::idx() and arrays
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

    // Faces -> indices (triangles preferred; skip non-triangles)
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
    emit logMessage(QStringLiteral("Reconstruction finished."));
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
