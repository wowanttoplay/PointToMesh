#ifndef POINTTOMESH_CGALPOINTCLOUDPROCESSOR_H
#define POINTTOMESH_CGALPOINTCLOUDPROCESSOR_H

#include "PointCloudProcessor.h"

/**
 * @class CGALPointCloudProcessor
 * @brief A concrete implementation of PointCloudProcessor using the CGAL library.
 *
 * This class implements the point cloud processing interface to load,
 * process, and export data using CGAL's algorithms.
 */
class CGALPointCloudProcessor : public PointCloudProcessor {
public:
    CGALPointCloudProcessor();
    ~CGALPointCloudProcessor() override;

    bool loadPointCloud(const std::string& filePath) override;
    bool estimateNormals(NormalEstimationMethod normalMethod = NormalEstimationMethod::JET_ESTIMATION) override;
    bool processToMesh(MeshGenerationMethod meshMethod = MeshGenerationMethod::POISSON_RECONSTRUCTION) override;
    bool exportMesh(const std::string& filePath) override;

    const PointCloud& getPointCloud() const override;
    const Mesh& getMesh() const override;
    bool hasNormals() const override;

private:
    PointCloud m_pointCloud;
    Mesh m_mesh;
};

#endif //POINTTOMESH_CGALPOINTCLOUDPROCESSOR_H
