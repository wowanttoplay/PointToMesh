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

    connect(&m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    // Upstream wiring: controller -> worker (queued)
    connect(this, &PointCloudController::workerImport,      m_worker, &ProcessingWorker::importPointCloud, Qt::QueuedConnection);
    connect(this, &PointCloudController::workerReconstruct, m_worker, &ProcessingWorker::reconstruct,       Qt::QueuedConnection);
    connect(this, &PointCloudController::workerExport,      m_worker, &ProcessingWorker::exportMeshTo,      Qt::QueuedConnection);

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

void PointCloudController::runReconstruction() {
    emit workerReconstruct();
}

void PointCloudController::exportMesh(const QString& path, bool withNormals) {
    emit workerExport(path, withNormals);
}

