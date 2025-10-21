#include "CGALPointCloudProcessor.h"
#include <iostream>

// CGAL I/O
#include <CGAL/IO/read_points.h>
#include <CGAL/IO/polygon_mesh_io.h>
#include <CGAL/poisson_surface_reconstruction.h>
#include <CGAL/property_map.h>
#include <CGAL/Polygon_mesh_processing/compute_normal.h>

namespace PMP = CGAL::Polygon_mesh_processing;

// Normal Estimation
#include <CGAL/jet_estimate_normals.h>
#include <CGAL/mst_orient_normals.h>

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/compute_average_spacing.h>
#include <array>

#include "BaseInputParameter.h"

CGALPointCloudProcessor::CGALPointCloudProcessor() = default;

CGALPointCloudProcessor::~CGALPointCloudProcessor() = default;

bool CGALPointCloudProcessor::loadPointCloud(const std::string &filePath) {
    m_pointCloud.clear();
    m_mesh.clear();

    // Read points and normals. CGAL::read_points can handle files with 3 (points) or 6 (points+normals) columns.
    if (!CGAL::IO::read_points(filePath, std::back_inserter(m_pointCloud),
                               CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                   .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()))) {
        std::cerr << "Error: Cannot read points from " << filePath << std::endl;
        return false;
    }

    // If no normals were loaded, ensure the vector part of the pair is cleared to NULL_VECTOR
    if (!hasNormals()) {
        for(auto& p : m_pointCloud) {
            p.second = CGAL::NULL_VECTOR;
        }
    }

    return !m_pointCloud.empty();
}

bool CGALPointCloudProcessor::estimateNormals(NormalEstimationMethod normalMethod) {
    if (m_pointCloud.empty()) {
        std::cerr << "Error: Point cloud is empty. Load a point cloud first." << std::endl;
        return false;
    }

    switch (normalMethod) {
        case NormalEstimationMethod::JET_ESTIMATION: {
            // Estimate normals using jet fitting.
            // This is a common method that requires a neighborhood size (e.g., 24 points).
            const int k_neighbors = 24;
            CGAL::jet_estimate_normals<CGAL::Sequential_tag>(m_pointCloud, k_neighbors,
                                                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                                                 .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));

            // Orient normals to be consistent
            CGAL::mst_orient_normals(m_pointCloud, k_neighbors,
                                     CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                         .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));

            return true;
        }
        // Other cases for different normal estimation methods can be added here.
        default:
            std::cerr << "Error: Unsupported normal estimation method." << std::endl;
            return false;
    }
}

bool CGALPointCloudProcessor::processToMesh(MeshGenerationMethod meshMethod, const BaseInputParameter* params) {
    if (m_pointCloud.empty()) {
        std::cerr << "Error: Point cloud is empty." << std::endl;
        return false;
    }

    switch (meshMethod) {
        case MeshGenerationMethod::POISSON_RECONSTRUCTION: {
            const auto *poisson = params ? dynamic_cast<const PoissonReconstructionParameter*>(params) : nullptr;
            return processPoissonWithParams(poisson);
        }
        case MeshGenerationMethod::SCALE_SPACE_RECONSTRUCTION: {
            const auto *ss = params ? dynamic_cast<const ScaleSpaceReconstructionParameter*>(params) : nullptr;
            return processScaleSpaceWithParams(ss);
        }
        case MeshGenerationMethod::ADVANCING_FRONT_RECONSTRUCTION: {
            const auto *af = params ? dynamic_cast<const AdvancingFrontReconstructionParameter*>(params) : nullptr;
            return processAdvancingFrontWithParams(af);
        }
        default:
            std::cerr << "Error: Unsupported mesh generation method." << std::endl;
            return false;
    }
}

bool CGALPointCloudProcessor::exportMesh(const std::string &filePath, bool withNormals) {
    if (m_mesh.is_empty()) {
        std::cerr << "Error: Mesh is empty. Generate a mesh first." << std::endl;
        return false;
    }

    // If normals requested, ensure we have per-vertex normals and let the generic writer
    // include them for any format that supports normals (OBJ/PLY/NOFF, etc.).
    if (withNormals) {
        auto vnormals_opt = m_mesh.property_map<Mesh::Vertex_index, Vector>("v:normal");
        if (!vnormals_opt) {
            if (!computeMeshNormals()) {
                std::cerr << "Error: Failed to compute normals for export." << std::endl;
                return false;
            }
            vnormals_opt = m_mesh.property_map<Mesh::Vertex_index, Vector>("v:normal");
        }

        if (!vnormals_opt) {
            std::cerr << "Error: Vertex normal property not available after computation." << std::endl;
            return false;
        }

        // Use the generic writer with a vertex_normal_map; formats that support normals will embed them.
        if (!CGAL::IO::write_polygon_mesh(filePath, m_mesh,
                                           CGAL::parameters::vertex_normal_map(*vnormals_opt))) {
            std::cerr << "Error: Cannot write mesh (with normals) to " << filePath << std::endl;
            return false;
        }
        return true;
    }

    // Without normals, fall back to the generic writer.
    if (!CGAL::IO::write_polygon_mesh(filePath, m_mesh)) {
        std::cerr << "Error: Cannot write mesh to " << filePath << std::endl;
        return false;
    }

    return true;
}

bool CGALPointCloudProcessor::computeMeshNormals() {
    if (m_mesh.is_empty()) {
        std::cerr << "Error: Mesh is empty. Cannot compute normals." << std::endl;
        return false;
    }

    // Create or get per-vertex normal property and compute
    auto vnormals = m_mesh.add_property_map<Mesh::Vertex_index, Vector>("v:normal", CGAL::NULL_VECTOR).first;
    try {
        PMP::compute_vertex_normals(m_mesh, vnormals);
    } catch (const std::exception& e) {
        std::cerr << "Error computing vertex normals: " << e.what() << std::endl;
        return false;
    }

    // Optionally compute face normals and store as property (not required for export)
    // auto fnormals = m_mesh.add_property_map<Mesh::Face_index, Vector>("f:normal", CGAL::NULL_VECTOR).first;
    // PMP::compute_face_normals(m_mesh, fnormals);

    return true;
}

const PointCloud &CGALPointCloudProcessor::getPointCloud() const {
    return m_pointCloud;
}

const Mesh &CGALPointCloudProcessor::getMesh() const {
    return m_mesh;
}

bool CGALPointCloudProcessor::hasNormals() const {
    if (m_pointCloud.empty()) {
        return false;
    }
    // If the first point has a non-zero normal vector, we assume the file contained normals.
    return m_pointCloud[0].second != CGAL::NULL_VECTOR;
}

// Helper implementations
bool CGALPointCloudProcessor::processPoissonWithParams(const PoissonReconstructionParameter* poisson) {
    if (!hasNormals()) {
        std::cerr << "Error: Normals are required for Poisson mesh generation but were not found or estimated." << std::endl;
        return false;
    }
    m_mesh.clear();
    double sm_angle = 20.0;
    double sm_radius = 30.0;
    double sm_distance = 0.375;
    int neighbors = 6;
    double spacing_scale = 1.0;
    if (poisson) {
        sm_angle = poisson->angle;
        sm_radius = poisson->radius;
        sm_distance = poisson->distance;
        neighbors = poisson->neighbors_number;
        spacing_scale = poisson->spacing_scale;
    }
    const double base_spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>(
        m_pointCloud, neighbors,
        CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
    );
    const double spacing = base_spacing * spacing_scale;
    const bool ok = CGAL::poisson_surface_reconstruction_delaunay(
        m_pointCloud.begin(), m_pointCloud.end(),
        CGAL::First_of_pair_property_map<PointWithNormal>(),
        CGAL::Second_of_pair_property_map<PointWithNormal>(),
        m_mesh,
        spacing, sm_angle, sm_radius, sm_distance);
    if (!ok) {
        std::cerr << "Error: Poisson surface reconstruction failed." << std::endl;
        return false;
    }
    return true;
}

bool CGALPointCloudProcessor::processScaleSpaceWithParams(const ScaleSpaceReconstructionParameter* ss) {
    std::vector<Point> pts;
    pts.reserve(m_pointCloud.size());
    for (const auto& pn : m_pointCloud) pts.push_back(pn.first);
    using SSSR = CGAL::Scale_space_surface_reconstruction_3<K>;
    SSSR recon(pts.begin(), pts.end());
    int iters = 4;
    if (ss) iters = ss->iterations_number;
    if (iters < 0) iters = 0;
    recon.increase_scale(iters);
    recon.reconstruct_surface();

    m_mesh.clear();
    std::vector<Mesh::Vertex_index> vdesc;
    vdesc.reserve(recon.number_of_points());
    for (const auto& p : recon.points()) vdesc.push_back(m_mesh.add_vertex(p));
    for (const auto& f : recon.facets()) {
        Mesh::Vertex_index v0 = vdesc[f[0]];
        Mesh::Vertex_index v1 = vdesc[f[1]];
        Mesh::Vertex_index v2 = vdesc[f[2]];
        if (v0 != v1 && v1 != v2 && v2 != v0) m_mesh.add_face(v0, v1, v2);
    }
    return !m_mesh.is_empty();
}

bool CGALPointCloudProcessor::processAdvancingFrontWithParams(const AdvancingFrontReconstructionParameter* /*af*/) {
    // No parameters currently, so run default advancing-front behavior
    std::vector<Point> pts;
    pts.reserve(m_pointCloud.size());
    for (const auto& pn : m_pointCloud) pts.push_back(pn.first);

    std::vector<std::array<std::size_t,3>> facets;
    CGAL::advancing_front_surface_reconstruction(pts.begin(), pts.end(), std::back_inserter(facets));

    m_mesh.clear();
    std::vector<Mesh::Vertex_index> vindices;
    vindices.reserve(pts.size());
    for (const auto& p : pts) vindices.push_back(m_mesh.add_vertex(p));
    for (const auto& f : facets) {
        const auto a = f[0], b = f[1], c = f[2];
        if (a < vindices.size() && b < vindices.size() && c < vindices.size() && a != b && b != c && c != a) {
            m_mesh.add_face(vindices[a], vindices[b], vindices[c]);
        }
    }
    return !m_mesh.is_empty();
}
