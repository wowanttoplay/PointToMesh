#pragma once
#include <QObject>
#include <QThread>
#include <memory>
#include <QString>
#include "ProcessingWorker.h"
#include "../Model/Geometry.h"

class PointCloudProcessor;

class PointCloudController : public QObject {
    Q_OBJECT
public:
    explicit PointCloudController(std::unique_ptr<PointCloudProcessor> proc, QObject* parent = nullptr);
    ~PointCloudController() override;

public slots:
    void importFromFile(const QString& path);
    void runReconstruction();
    void runReconstructionWith(MeshGenerationMethod method);
    void exportMesh(const QString& path, bool withNormals);

signals:
    void logMessage(const QString& message);
    void pointCloudUpdated(PointCloudPtr cloud);
    void meshUpdated(MeshPtr mesh);

    // Internal to worker thread
    void workerImport(const QString& path);
    void workerReconstruct();
    void workerReconstructWith(MeshGenerationMethod method);
    void workerExport(const QString& path, bool withNormals);

private slots:
    void onWorkerLog(const QString& m) { emit logMessage(m); }
    void onPointCloudReady(PointCloudPtr cloud) { emit pointCloudUpdated(std::move(cloud)); }
    void onMeshReady(MeshPtr mesh) { emit meshUpdated(std::move(mesh)); }

private:
    QThread m_thread;
    ProcessingWorker* m_worker {nullptr};
};
