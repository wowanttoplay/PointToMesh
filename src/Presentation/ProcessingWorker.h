#pragma once
#include <QObject>
#include <memory>
#include <QString>
#include "../DataProcess/PointCloudProcessor.h"
#include "../Model/Geometry.h"
#include "../DataProcess/BaseInputParameter.h"

class ProcessingWorker : public QObject {
    Q_OBJECT
public:
    explicit ProcessingWorker(std::unique_ptr<PointCloudProcessor> proc, QObject* parent = nullptr);
    ~ProcessingWorker() override;

public slots:
    void importPointCloud(const QString& filePath);
    // Parameterized reconstruction; takes ownership of params and deletes it in worker thread
    void reconstructWithParams(MeshGenerationMethod method, BaseInputParameter* params);
    void exportMeshTo(const QString& filePath, bool withNormals);

signals:
    void logMessage(const QString& message);
    void pointCloudReady(PointCloudPtr cloud);
    void meshReady(MeshPtr mesh);

private:
    std::unique_ptr<PointCloudProcessor> m_proc;
};
