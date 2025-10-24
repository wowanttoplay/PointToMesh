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
    void estimateNormals(NormalEstimationMethod method);
    // New: mesh post-process; takes ownership of params and deletes in worker thread
    void postProcessMeshWith(BaseInputParameter* params);

    // New: point cloud operations
    void downsampleVoxelWith(BaseInputParameter* params);
    void filterPointCloudAABB(BaseInputParameter* params);
    void filterPointCloudSphere(BaseInputParameter* params);
    void filterUniformVolumeSurface(BaseInputParameter* params);

signals:
    void logMessage(const QString& message);
    void pointCloudReady(PointCloudPtr cloud);
    void meshReady(MeshPtr mesh);

private:
    std::unique_ptr<PointCloudProcessor> m_proc;

    // Helpers to reduce duplication
    std::shared_ptr<PointCloudModel> toPointCloudModel(const PointCloud& pc) const;
    std::shared_ptr<MeshModel> toMeshModel(const Mesh& mesh) const;
};
