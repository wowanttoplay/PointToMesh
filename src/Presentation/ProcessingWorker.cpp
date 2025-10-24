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

// Helper: convert internal point cloud to UI model
std::shared_ptr<PointCloudModel> ProcessingWorker::toPointCloudModel(const PointCloud& pc) const {
    auto model = std::make_shared<PointCloudModel>();
    model->points.reserve(pc.size());
    model->normals.reserve(pc.size());
    for (const auto& pn : pc) {
        const auto& p = pn.first;
        const auto& n = pn.second;
        model->points.emplace_back(static_cast<float>(p.x()), static_cast<float>(p.y()), static_cast<float>(p.z()));
        if (n != CGAL::NULL_VECTOR)
            model->normals.emplace_back(static_cast<float>(n.x()), static_cast<float>(n.y()), static_cast<float>(n.z()));
        else
            model->normals.emplace_back(0.0f, 0.0f, 0.0f);
    }
    return model;
}

// Helper: convert internal mesh to UI model
std::shared_ptr<MeshModel> ProcessingWorker::toMeshModel(const Mesh& mesh) const {
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
    return model;
}

namespace {
struct TaskScope {
    QObject* obj;
    ~TaskScope() { if (obj) QMetaObject::invokeMethod(obj, "taskFinished", Qt::QueuedConnection); }
};
}

void ProcessingWorker::importPointCloud(const QString& filePath) {
    TaskScope scope{this};
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const std::string path = filePath.toStdString();
    if (!m_proc->loadPointCloud(path)) {
        emit logMessage(QStringLiteral("Failed to load point cloud: ") + filePath);
        return;
    }
    emit logMessage(QStringLiteral("Loaded point cloud: ") + filePath);

    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
}

void ProcessingWorker::reconstructWithParams(MeshGenerationMethod method, BaseInputParameter* params) {
    TaskScope scope{this};
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
        if (!m_proc->estimateNormals(NormalEstimationMethod::VCM_ESTIMATION)) {
            emit logMessage(QStringLiteral("Normal estimation failed."));
            return;
        }
    }

    emit logMessage(QStringLiteral("Running ") + methodName + QStringLiteral(" with parameters..."));
    if (!m_proc->processToMesh(method, guard.get())) {
        emit logMessage(methodName + QStringLiteral(" failed."));
        return;
    }

    emit meshReady(toMeshModel(m_proc->getMesh()));
    emit logMessage(methodName + QStringLiteral(" finished."));
}

void ProcessingWorker::exportMeshTo(const QString& filePath, bool withNormals) {
    TaskScope scope{this};
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const std::string path = filePath.toStdString();
    if (!m_proc->exportMesh(path, withNormals)) {
        emit logMessage(QStringLiteral("Export failed: ") + filePath);
        return;
    }
    emit logMessage(QStringLiteral("Exported mesh to: ") + filePath);
}

void ProcessingWorker::estimateNormals(NormalEstimationMethod method) {
    TaskScope scope{this};
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }

    const auto methodName = [method]() -> QString {
        switch (method) {
            case NormalEstimationMethod::JET_ESTIMATION: return QStringLiteral("Jet estimation");
            case NormalEstimationMethod::UNIFORM_VOLUME_CENTROID: return QStringLiteral("Uniform centroid estimation");
            case NormalEstimationMethod::VCM_ESTIMATION: return QStringLiteral("VCM estimation");
            default: return QStringLiteral("Unknown normal estimation");
        }
    }();

    emit logMessage(QStringLiteral("Estimating normals using ") + methodName + QStringLiteral("..."));
    if (!m_proc->estimateNormals(method)) {
        emit logMessage(QStringLiteral("Normal estimation failed."));
        return;
    }

    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
    emit logMessage(QStringLiteral("Normals updated."));
}

void ProcessingWorker::postProcessMeshWith(BaseInputParameter* params) {
    TaskScope scope{this};
    // Takes ownership
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }

    emit logMessage(QStringLiteral("Post-processing mesh..."));
    if (!m_proc->postProcessMesh(guard.get())) {
        emit logMessage(QStringLiteral("Mesh post-process failed."));
        return;
    }

    emit meshReady(toMeshModel(m_proc->getMesh()));
    emit logMessage(QStringLiteral("Mesh post-process finished."));
}

void ProcessingWorker::downsampleVoxelWith(BaseInputParameter* params) {
    TaskScope scope{this};
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const auto before = static_cast<long long>(m_proc->getPointCloud().size());
    emit logMessage(QStringLiteral("Downsampling point cloud (voxel grid)..."));
    if (!m_proc->downsampleVoxel(guard.get())) {
        emit logMessage(QStringLiteral("Voxel downsample failed."));
        return;
    }
    const auto after = static_cast<long long>(m_proc->getPointCloud().size());
    const auto delta = after - before;
    emit logMessage(QStringLiteral("Points: ") + QString::number(before) + QStringLiteral(" -> ") + QString::number(after) +
                    QStringLiteral(" (Δ ") + QString::number(delta) + QStringLiteral(")"));
    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
    emit logMessage(QStringLiteral("Voxel downsample finished."));
}

void ProcessingWorker::filterPointCloudAABB(BaseInputParameter* params) {
    TaskScope scope{this};
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const auto before = static_cast<long long>(m_proc->getPointCloud().size());
    emit logMessage(QStringLiteral("Filtering point cloud by AABB..."));
    if (!m_proc->filterAABB(guard.get())) {
        emit logMessage(QStringLiteral("AABB filter failed."));
        return;
    }
    const auto after = static_cast<long long>(m_proc->getPointCloud().size());
    const auto delta = after - before;
    emit logMessage(QStringLiteral("Points: ") + QString::number(before) + QStringLiteral(" -> ") + QString::number(after) +
                    QStringLiteral(" (Δ ") + QString::number(delta) + QStringLiteral(")"));
    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
    emit logMessage(QStringLiteral("AABB filter finished."));
}

void ProcessingWorker::filterPointCloudSphere(BaseInputParameter* params) {
    TaskScope scope{this};
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const auto before = static_cast<long long>(m_proc->getPointCloud().size());
    emit logMessage(QStringLiteral("Filtering point cloud by sphere..."));
    if (!m_proc->filterSphere(guard.get())) {
        emit logMessage(QStringLiteral("Sphere filter failed."));
        return;
    }
    const auto after = static_cast<long long>(m_proc->getPointCloud().size());
    const auto delta = after - before;
    emit logMessage(QStringLiteral("Points: ") + QString::number(before) + QStringLiteral(" -> ") + QString::number(after) +
                    QStringLiteral(" (Δ ") + QString::number(delta) + QStringLiteral(")"));
    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
    emit logMessage(QStringLiteral("Sphere filter finished."));
}

void ProcessingWorker::filterUniformVolumeSurface(BaseInputParameter* params) {
    TaskScope scope{this};
    std::unique_ptr<BaseInputParameter> guard(params);
    if (!m_proc) { emit logMessage("Processor not initialized."); return; }
    const auto before = static_cast<long long>(m_proc->getPointCloud().size());
    emit logMessage(QStringLiteral("Filtering surface points from uniform volume..."));
    if (!m_proc->filterSurfaceFromUniformVolume(guard.get())) {
        emit logMessage(QStringLiteral("Uniform-volume surface filter failed."));
        return;
    }
    const auto after = static_cast<long long>(m_proc->getPointCloud().size());
    const auto delta = after - before;
    emit logMessage(QStringLiteral("Points: ") + QString::number(before) + QStringLiteral(" -> ") + QString::number(after) +
                    QStringLiteral(" (Δ ") + QString::number(delta) + QStringLiteral(")"));
    emit pointCloudReady(toPointCloudModel(m_proc->getPointCloud()));
    emit logMessage(QStringLiteral("Uniform-volume surface filter finished."));
}
