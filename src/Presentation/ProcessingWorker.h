#pragma once
#include <QObject>
#include <memory>
#include <QString>
#include "../DataProcess/PointCloudProcessor.h"
#include "../Model/Geometry.h"

class ProcessingWorker : public QObject {
    Q_OBJECT
public:
    explicit ProcessingWorker(std::unique_ptr<PointCloudProcessor> proc, QObject* parent = nullptr);
    ~ProcessingWorker() override;

public slots:
    void importPointCloud(const QString& filePath);
    void reconstruct();
    void reconstructWith(MeshGenerationMethod method);
    void exportMeshTo(const QString& filePath, bool withNormals);

signals:
    void logMessage(const QString& message);
    void pointCloudReady(PointCloudPtr cloud);
    void meshReady(MeshPtr mesh);

private:
    std::unique_ptr<PointCloudProcessor> m_proc;
};
