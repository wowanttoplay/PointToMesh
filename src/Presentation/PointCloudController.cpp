#include "PointCloudController.h"

#include <QFileDialog>
#include <QMetaObject>
#include <utility>
#include "ProcessingWorker.h"
#include "../DataProcess/PointCloudProcessor.h"

PointCloudController::PointCloudController(std::unique_ptr<PointCloudProcessor> proc, QObject* parent)
    : QObject(parent) {
    m_worker = new ProcessingWorker(std::move(proc));
    m_worker->moveToThread(&m_thread);

    // Ensure enums and pointer types are known to Qt for queued connections
    qRegisterMetaType<MeshGenerationMethod>("MeshGenerationMethod");
    qRegisterMetaType<NormalEstimationMethod>("NormalEstimationMethod");
    qRegisterMetaType<BaseInputParameter*>("BaseInputParameter*");

    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    // Upstream wiring: controller -> worker (queued)
    connect(this, &PointCloudController::workerImport,      m_worker, &ProcessingWorker::importPointCloud, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerReconstructWithParams, m_worker, &ProcessingWorker::reconstructWithParams, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerExport,      m_worker, &ProcessingWorker::exportMeshTo,      Qt::QueuedConnection);
    connect(this, &PointCloudController::workerEstimateNormals, m_worker, &ProcessingWorker::estimateNormals, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerPostProcessMesh, m_worker, &ProcessingWorker::postProcessMeshWith, Qt::QueuedConnection);

    // New: point cloud ops wiring
    connect(this, &PointCloudController::workerDownsampleVoxel, m_worker, &ProcessingWorker::downsampleVoxelWith, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerFilterAABB, m_worker, &ProcessingWorker::filterPointCloudAABB, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerFilterSphere, m_worker, &ProcessingWorker::filterPointCloudSphere, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerFilterUniformVolumeSurface, m_worker, &ProcessingWorker::filterUniformVolumeSurface, Qt::QueuedConnection);

    // Downstream wiring: worker -> controller
    connect(m_worker, &ProcessingWorker::logMessage,      this, &PointCloudController::onWorkerLog,        Qt::QueuedConnection);
    connect(m_worker, &ProcessingWorker::pointCloudReady, this, &PointCloudController::onPointCloudReady,  Qt::QueuedConnection);
    connect(m_worker, &ProcessingWorker::meshReady,       this, &PointCloudController::onMeshReady,        Qt::QueuedConnection);

    m_thread.start();
}

PointCloudController::~PointCloudController() {
    m_thread.quit();
    m_thread.wait();
}

void PointCloudController::importFromFile(const QString& path) {
    if (!path.isEmpty()) m_lastImportPath = path;
    emit workerImport(path);
}

void PointCloudController::resetToOriginal() {
    if (m_lastImportPath.isEmpty()) {
        emit logMessage(QStringLiteral("No previously imported point cloud to reset."));
        return;
    }
    // Re-import the same file path; processing is in worker, so no duplicate in controller memory
    emit logMessage(QStringLiteral("Resetting point cloud to original data: ") + m_lastImportPath);
    emit workerImport(m_lastImportPath);
}

void PointCloudController::runReconstructionWith(MeshGenerationMethod method, std::unique_ptr<BaseInputParameter> params) {
    // Transfer raw pointer to worker ownership; we'll release here and let worker delete
    BaseInputParameter* raw = params.release();
    emit workerReconstructWithParams(method, raw);
}

void PointCloudController::runNormalEstimation(NormalEstimationMethod method) {
    emit workerEstimateNormals(method);
}

void PointCloudController::exportMesh(const QString& path, bool withNormals) {
    emit workerExport(path, withNormals);
}

void PointCloudController::runPostProcessMesh(std::unique_ptr<BaseInputParameter> params) {
    BaseInputParameter* raw = params.release();
    emit workerPostProcessMesh(raw);
}

void PointCloudController::runDownsampleVoxel(std::unique_ptr<BaseInputParameter> params) {
    BaseInputParameter* raw = params.release();
    emit workerDownsampleVoxel(raw);
}

void PointCloudController::runFilterAABB(std::unique_ptr<BaseInputParameter> params) {
    BaseInputParameter* raw = params.release();
    emit workerFilterAABB(raw);
}

void PointCloudController::runFilterSphere(std::unique_ptr<BaseInputParameter> params) {
    BaseInputParameter* raw = params.release();
    emit workerFilterSphere(raw);
}

void PointCloudController::runFilterUniformVolumeSurface(std::unique_ptr<BaseInputParameter> params) {
    BaseInputParameter* raw = params.release();
    emit workerFilterUniformVolumeSurface(raw);
}
