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
    emit workerImport(path);
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
