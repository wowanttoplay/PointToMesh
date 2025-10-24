#ifndef POINTTOMESH_POINTCLOUDPROCESSOR_H
#define POINTTOMESH_POINTCLOUDPROCESSOR_H

#include <string>
#include <vector>
#include <memory>
#include <QMetaType>

#include "BaseInputParameter.h"

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
    JET_ESTIMATION, // Default method using jet fitting for near-surface point sets
    UNIFORM_VOLUME_CENTROID, // New: for uniformly distributed points inside a region (centroid gradient)
    VCM_ESTIMATION, // New: Voronoi Covariance Measure-based normal estimation
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
Q_DECLARE_METATYPE(NormalEstimationMethod)

// Lightweight options for mesh post-processing
struct MeshPostprocessOptions {
    // Connectivity cleanup
    int keep_largest_components = 0; // 0 = no filtering; 1 = keep only the largest, N = keep top-N
    bool remove_degenerate_faces = true;
    bool remove_isolated_vertices = true;
    bool stitch_borders = false; // attempt to stitch coincident borders

    // Hole filling: fill only holes whose boundary cycle has <= this number of edges (0 disables)
    int fill_holes_max_cycle_edges = 0;

    // Isotropic remeshing
    int remesh_iterations = 0; // 0 disables
    double remesh_target_edge_length = 0.0; // <= 0 means auto from average edge length

    // Smoothing (Laplacian-based)
    int smooth_iterations = 0; // 0 disables

    // Recompute vertex normals after processing
    bool recompute_normals = true;
};

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
    virtual bool estimateNormals(NormalEstimationMethod normalMethod) = 0;

    /**
     * @brief Processes the loaded point cloud into a mesh with the given parameters.
     *        Implementations may dynamically cast to the expected parameter type per method.
     * @param meshMethod The algorithm to use for mesh generation.
     * @param params Optional parameters object (may be null). Ownership is not taken here.
     */
    virtual bool processToMesh(MeshGenerationMethod meshMethod, const BaseInputParameter* params) = 0;

    /**
     * @brief Exports the generated mesh to a file.
     * @param filePath The path to the output mesh file.
     * @param withNormals If true, export vertex normals when supported by the format.
     * @return True if exporting was successful, false otherwise.
     */
    virtual bool exportMesh(const std::string& filePath, bool withNormals) = 0;

    /**
     * @brief Provides access to the internal point cloud data.
     * @return A const reference to the point cloud.
     */
    [[nodiscard]] virtual const PointCloud& getPointCloud() const = 0;

    /**
     * @brief Checks if the point cloud has associated normals.
     * @return True if normals are present, false otherwise.
     */
    [[nodiscard]] virtual bool hasNormals() const = 0;

    /**
     * @brief Provides access to the internal mesh data.
     * @return A const reference to the mesh.
     */
    [[nodiscard]] virtual const Mesh& getMesh() const = 0;

    /**
     * @brief Compute and attach normals for the current mesh.
     *        Creates/updates per-vertex normals property (and per-face optionally in impl).
     * @return True if normals were computed successfully, false otherwise.
     */
    virtual bool computeMeshNormals() = 0;

    // --- New: Point cloud utilities ---

    /**
     * @brief Downsample the point cloud using a voxel grid (CGAL grid simplification).
     * @param cell_size Size of the cubic voxel cells in the same units as the point cloud.
     * @return True if downsampling succeeded and the point cloud was modified.
     */
    virtual bool downsampleVoxel(double cell_size) = 0;

    /**
     * @brief Keep or remove points based on an axis-aligned bounding box.
     * @param min_x Minimum X of the AABB
     * @param min_y Minimum Y of the AABB
     * @param min_z Minimum Z of the AABB
     * @param max_x Maximum X of the AABB
     * @param max_y Maximum Y of the AABB
     * @param max_z Maximum Z of the AABB
     * @param keepInside If true, keep points inside the AABB; otherwise remove them
     * @return True if filtering succeeded and the point cloud was modified (or unchanged if no points match)
     */
    virtual bool filterAABB(double min_x, double min_y, double min_z,
                            double max_x, double max_y, double max_z,
                            bool keepInside) = 0;

    /**
     * @brief Keep or remove points based on a sphere.
     * @param cx Center X
     * @param cy Center Y
     * @param cz Center Z
     * @param radius Sphere radius (> 0)
     * @param keepInside If true, keep points inside the sphere; otherwise remove them
     * @return True if filtering succeeded and the point cloud was modified (or unchanged if no points match)
     */
    virtual bool filterSphere(double cx, double cy, double cz, double radius, bool keepInside) = 0;

    // --- New: Mesh post-processing ---

    /**
     * @brief Post-process the current mesh with a set of common cleanup/smoothing/remeshing operations.
     * @param options Options controlling which operations to run and their parameters.
     * @return True if the mesh exists and the operations completed.
     */
    virtual bool postProcessMesh(const MeshPostprocessOptions& options) = 0;
};

#endif //POINTTOMESH_POINTCLOUDPROCESSOR_H
