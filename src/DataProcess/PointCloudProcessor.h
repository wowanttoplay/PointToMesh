#ifndef POINTTOMESH_POINTCLOUDPROCESSOR_H
#define POINTTOMESH_POINTCLOUDPROCESSOR_H

#include <string>
#include <vector>
#include <memory>
#include <QMetaType>

// We use CGAL types for the interface, as it's our primary library.
// This simplifies the design, but for a truly generic library,
// one might define library-agnostic data structures.
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Point_3.h>
#include <CGAL/Surface_mesh.h>

// Define common types for the interface
using K = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point = K::Point_3;
using Vector = K::Vector_3;
using PointWithNormal = std::pair<Point, Vector>;
using PointCloud = std::vector<PointWithNormal>; // Now stores points with normals
using Mesh = CGAL::Surface_mesh<Point>;

/**
 * @enum NormalEstimationMethod
 * @brief Defines the available algorithms for normal estimation.
 */
enum class NormalEstimationMethod {
    JET_ESTIMATION, // Default method using jet fitting
    // Future methods like MLS can be added here
};

/**
 * @enum MeshGenerationMethod
 * @brief Defines the available algorithms for mesh generation from point clouds.
 */
enum class MeshGenerationMethod {
    POISSON_RECONSTRUCTION, // Default method using Poisson reconstruction
    SCALE_SPACE_RECONSTRUCTION, // CGAL Scale-Space Surface Reconstruction 3
    ADVANCING_FRONT_RECONSTRUCTION, // CGAL Advancing Front Surface Reconstruction
};

// Make enums available to Qt meta-object system for queued connections
Q_DECLARE_METATYPE(MeshGenerationMethod)

/**
 * @class PointCloudProcessor
 * @brief An abstract base class defining the interface for point cloud processing.
 *
 * This class provides a common interface for loading, processing, and exporting
 * point cloud and mesh data. Concrete classes will implement these functions
 * using specific libraries (e.g., CGAL, PCL).
 */
class PointCloudProcessor {
public:
    virtual ~PointCloudProcessor() = default;

    /**
     * @brief Loads a point cloud from a file.
     * @param filePath The path to the point cloud file.
     * @return True if loading was successful, false otherwise.
     */
    virtual bool loadPointCloud(const std::string& filePath) = 0;

    /**
     * @brief Estimates normals for the loaded point cloud.
     * @param normalMethod The algorithm to use for normal estimation.
     * @return True if normal estimation was successful, false otherwise.
     */
    virtual bool estimateNormals(NormalEstimationMethod normalMethod = NormalEstimationMethod::JET_ESTIMATION) = 0;

    /**
     * @brief Processes the loaded point cloud into a mesh. Requires normals to be present.
     * @param meshMethod The algorithm to use for mesh generation.
     * @return True if processing was successful, false otherwise.
     */
    virtual bool processToMesh(MeshGenerationMethod meshMethod = MeshGenerationMethod::POISSON_RECONSTRUCTION) = 0;

    /**
     * @brief Exports the generated mesh to a file.
     * @param filePath The path to the output mesh file.
     * @param withNormals If true, export vertex normals when supported by the format.
     * @return True if exporting was successful, false otherwise.
     */
    virtual bool exportMesh(const std::string& filePath, bool withNormals = false) = 0;

    /**
     * @brief Provides access to the internal point cloud data.
     * @return A const reference to the point cloud.
     */
    virtual const PointCloud& getPointCloud() const = 0;

    /**
     * @brief Checks if the point cloud has associated normals.
     * @return True if normals are present, false otherwise.
     */
    virtual bool hasNormals() const = 0;

    /**
     * @brief Provides access to the internal mesh data.
     * @return A const reference to the mesh.
     */
    virtual const Mesh& getMesh() const = 0;

    /**
     * @brief Compute and attach normals for the current mesh.
     *        Creates/updates per-vertex normals property (and per-face optionally in impl).
     * @return True if normals were computed successfully, false otherwise.
     */
    virtual bool computeMeshNormals() = 0;
};

#endif //POINTTOMESH_POINTCLOUDPROCESSOR_H
