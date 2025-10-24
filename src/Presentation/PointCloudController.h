#pragma once
#include <QObject>
#include <QThread>
#include <memory>
#include <QString>
#include "ProcessingWorker.h"
#include "../Model/Geometry.h"
#include "../DataProcess/BaseInputParameter.h"

class PointCloudProcessor;

class PointCloudController : public QObject {
    Q_OBJECT
public:
    explicit PointCloudController(std::unique_ptr<PointCloudProcessor> proc, QObject* parent = nullptr);
    ~PointCloudController() override;

    // Overload that accepts a parameter object; ownership will be transferred to the worker thread
    void runReconstructionWith(MeshGenerationMethod method, std::unique_ptr<BaseInputParameter> params);
    // Trigger normal estimation with a selected method
    void runNormalEstimation(NormalEstimationMethod method);

    // Re-import the last loaded point cloud from disk. If none, emits a log message.
    void resetToOriginal();

public slots:
    void importFromFile(const QString& path);
    void exportMesh(const QString& path, bool withNormals);

signals:
    void logMessage(const QString& message);
    void pointCloudUpdated(PointCloudPtr cloud);
    void meshUpdated(MeshPtr mesh);

    // Internal to worker thread
    void workerImport(const QString& path);
    void workerReconstructWithParams(MeshGenerationMethod method, BaseInputParameter* params); // takes ownership
    void workerExport(const QString& path, bool withNormals);
    void workerEstimateNormals(NormalEstimationMethod method);

private slots:
    void onWorkerLog(const QString& m) { emit logMessage(m); }
    void onPointCloudReady(PointCloudPtr cloud) { emit pointCloudUpdated(std::move(cloud)); }
    void onMeshReady(MeshPtr mesh) { emit meshUpdated(std::move(mesh)); }

private:
    QThread m_thread;
    ProcessingWorker* m_worker {nullptr};
    QString m_lastImportPath; // last successfully requested import path
};
