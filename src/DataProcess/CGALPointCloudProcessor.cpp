#include "CGALPointCloudProcessor.h"
#include <iostream>
#include <algorithm>

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
#include <CGAL/vcm_estimate_normals.h>

#include <CGAL/Scale_space_surface_reconstruction_3.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>
#include <CGAL/compute_average_spacing.h>
#include <array>

// Neighbor search for centroid-based normals
#include <CGAL/Search_traits_3.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>

#include "BaseInputParameter.h"

// New includes for point set processing and mesh post-processing
#include <CGAL/grid_simplify_point_set.h>
#include <CGAL/Polygon_mesh_processing/repair.h>
#include <CGAL/Polygon_mesh_processing/repair_degeneracies.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/stitch_borders.h>
#include <CGAL/Polygon_mesh_processing/smooth_mesh.h>

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
        case NormalEstimationMethod::JET_ESTIMATION:
            return estimateNormalsJet();
        case NormalEstimationMethod::UNIFORM_VOLUME_CENTROID:
            return estimateNormalsUniformVolumeCentroid();
        case NormalEstimationMethod::VCM_ESTIMATION:
            return estimateNormalsVCM();
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

// Helper implementations (mesh)
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
    // sm_radius and sm_distance are specified relative to spacing; no extra scaling needed
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

// Helper implementations (normals)
bool CGALPointCloudProcessor::estimateNormalsJet() {
    const int k_neighbors = 24;
    CGAL::jet_estimate_normals<CGAL::Sequential_tag>(m_pointCloud, k_neighbors,
                                                     CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                                         .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));

    CGAL::mst_orient_normals(m_pointCloud, k_neighbors,
                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                 .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));
    return true;
}

bool CGALPointCloudProcessor::estimateNormalsUniformVolumeCentroid() {
    using Traits = CGAL::Search_traits_3<K>;
    using KnnSearch = CGAL::Orthogonal_k_neighbor_search<Traits>;
    using Tree = KnnSearch::Tree;

    std::vector<Point> pts;
    pts.reserve(m_pointCloud.size());
    for (const auto& pn : m_pointCloud) pts.push_back(pn.first);
    Tree tree(pts.begin(), pts.end());

    const int k_neighbors = 24;
    auto nmap = CGAL::Second_of_pair_property_map<PointWithNormal>();

    for (std::size_t i = 0; i < pts.size(); ++i) {
        const Point& query = pts[i];
        int k = std::min<int>(k_neighbors + 1, static_cast<int>(pts.size()));
        KnnSearch search(tree, query, k);

        K::FT cx = 0, cy = 0, cz = 0;
        int count = 0;
        for (const auto& res : search) {
            if (res.second == 0) continue; // skip self
            const Point& p = res.first;
            cx += p.x(); cy += p.y(); cz += p.z();
            ++count;
        }

        if (count == 0) {
            put(nmap, m_pointCloud[i], CGAL::NULL_VECTOR);
            continue;
        }

        const Point centroid(cx / count, cy / count, cz / count);
        Vector v = Vector(centroid, query);
        const auto s = v.squared_length();
        if (s <= static_cast<K::FT>(1e-16)) {
            put(nmap, m_pointCloud[i], CGAL::NULL_VECTOR);
        } else {
            const double len = std::sqrt(CGAL::to_double(s));
            put(nmap, m_pointCloud[i], v / len);
        }
    }

    CGAL::mst_orient_normals(m_pointCloud, k_neighbors,
                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                 .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));
    return true;
}

bool CGALPointCloudProcessor::estimateNormalsVCM() {
    const double spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>(
        m_pointCloud, 6,
        CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
    );
    const double neighbor_radius = 2.0 * spacing;
    const double convolution_radius = 4.0 * spacing;

    CGAL::vcm_estimate_normals(
        m_pointCloud,
        neighbor_radius,
        convolution_radius,
        CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
            .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>())
    );

    const int k_neighbors = 24;
    CGAL::mst_orient_normals(m_pointCloud, k_neighbors,
                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>())
                                 .normal_map(CGAL::Second_of_pair_property_map<PointWithNormal>()));
    return true;
}

// New: point cloud utilities
bool CGALPointCloudProcessor::downsampleVoxel(double cell_size) {
    if (m_pointCloud.empty()) { std::cerr << "Error: No point cloud loaded." << std::endl; return false; }
    if (!(cell_size > 0.0)) { std::cerr << "Error: cell_size must be > 0." << std::endl; return false; }

    const std::size_t before = m_pointCloud.size();
    auto end = CGAL::grid_simplify_point_set(m_pointCloud,
                                             cell_size,
                                             CGAL::parameters::point_map(CGAL::First_of_pair_property_map<PointWithNormal>()));
    m_pointCloud.erase(end, m_pointCloud.end());

    return m_pointCloud.size() <= before; // true even if unchanged
}

bool CGALPointCloudProcessor::filterAABB(const BaseInputParameter* params) {
    if (m_pointCloud.empty()) { std::cerr << "Error: No point cloud loaded." << std::endl; return false; }
    const auto* aabb = params ? dynamic_cast<const AABBFilterParameter*>(params) : nullptr;
    if (!aabb) { std::cerr << "Error: AABBFilterParameter expected." << std::endl; return false; }

    const double min_x = aabb->min_x, min_y = aabb->min_y, min_z = aabb->min_z;
    const double max_x = aabb->max_x, max_y = aabb->max_y, max_z = aabb->max_z;
    const bool keepInside = aabb->keepInside;

    if (!(min_x <= max_x && min_y <= max_y && min_z <= max_z)) {
        std::cerr << "Error: Invalid AABB extents." << std::endl; return false;
    }
    auto inside = [&](const Point& p) {
        return p.x() >= min_x && p.x() <= max_x &&
               p.y() >= min_y && p.y() <= max_y &&
               p.z() >= min_z && p.z() <= max_z;
    };

    const std::size_t before = m_pointCloud.size();
    m_pointCloud.erase(std::remove_if(m_pointCloud.begin(), m_pointCloud.end(), [&](const PointWithNormal& pn){
        const bool in = inside(pn.first);
        return keepInside ? !in : in;
    }), m_pointCloud.end());

    return m_pointCloud.size() <= before;
}

bool CGALPointCloudProcessor::filterSphere(const BaseInputParameter* params) {
    if (m_pointCloud.empty()) { std::cerr << "Error: No point cloud loaded." << std::endl; return false; }
    const auto* s = params ? dynamic_cast<const SphereFilterParameter*>(params) : nullptr;
    if (!s) { std::cerr << "Error: SphereFilterParameter expected." << std::endl; return false; }

    const double cx = s->cx, cy = s->cy, cz = s->cz, radius = s->radius;
    const bool keepInside = s->keepInside;

    if (!(radius > 0.0)) { std::cerr << "Error: radius must be > 0." << std::endl; return false; }
    const double r2 = radius * radius;

    const std::size_t before = m_pointCloud.size();
    m_pointCloud.erase(std::remove_if(m_pointCloud.begin(), m_pointCloud.end(), [&](const PointWithNormal& pn){
        const auto dx = CGAL::to_double(pn.first.x()) - cx;
        const auto dy = CGAL::to_double(pn.first.y()) - cy;
        const auto dz = CGAL::to_double(pn.first.z()) - cz;
        const double d2 = dx*dx + dy*dy + dz*dz;
        const bool in = d2 <= r2;
        return keepInside ? !in : in;
    }), m_pointCloud.end());

    return m_pointCloud.size() <= before;
}

// New: mesh post-processing
bool CGALPointCloudProcessor::postProcessMesh(const BaseInputParameter* params) {
    if (m_mesh.is_empty()) { std::cerr << "Error: Mesh is empty." << std::endl; return false; }
    const auto* options = params ? dynamic_cast<const MeshPostprocessParameter*>(params) : nullptr;
    if (!options) { std::cerr << "Error: MeshPostprocessParameter expected." << std::endl; return false; }

    // Optionally remove degenerate faces first to avoid issues downstream
    if (options->remove_degenerate_faces) {
        PMP::remove_degenerate_faces(m_mesh);
    }

    // Stitch borders (can help before hole filling and remeshing)
    if (options->stitch_borders) {
        PMP::stitch_borders(m_mesh);
    }

    // Keep only the largest (or top-N) connected components
    if (options->keep_largest_components > 0) {
        std::size_t nb_cc = PMP::connected_components(m_mesh, m_mesh.add_property_map<Mesh::Face_index, std::size_t>("f:CC", 0).first);
        (void)nb_cc; // not used further
        if (options->keep_largest_components == 1) {
            PMP::keep_largest_connected_components(m_mesh, 1);
        } else {
            PMP::keep_largest_connected_components(m_mesh, static_cast<std::size_t>(options->keep_largest_components));
        }
    }

    // Remove isolated vertices after possible face removals
    if (options->remove_isolated_vertices) {
        PMP::remove_isolated_vertices(m_mesh);
    }

    // Fill small holes
    if (options->fill_holes_max_cycle_edges > 0) {
        // Iterate over a snapshot of border halfedges by scanning all halfedges
        std::vector<Mesh::Halfedge_index> borders;
        borders.reserve(num_halfedges(m_mesh));
        for (Mesh::Halfedge_index h : halfedges(m_mesh)) {
            if (CGAL::is_border(h, m_mesh)) borders.push_back(h);
        }
        for (Mesh::Halfedge_index h : borders) {
            if (!CGAL::is_border(h, m_mesh)) continue; // may have been filled already
            // Count border cycle length by walking next() around the hole
            int count = 0;
            Mesh::Halfedge_index cur = h;
            const int max_check = options->fill_holes_max_cycle_edges;
            do {
                cur = m_mesh.next(cur);
                ++count;
                if (cur == Mesh::null_halfedge()) { count = max_check + 1; break; }
            } while (cur != h && count <= max_check);

            if (count <= max_check) {
                PMP::triangulate_hole(m_mesh, h);
            }
        }
    }

    // Isotropic remeshing
    if (options->remesh_iterations > 0) {
        // Determine target edge length if not given
        double target = options->remesh_target_edge_length;
        if (!(target > 0.0)) {
            // Approximate average edge length
            double sum = 0.0; std::size_t cnt = 0;
            for (auto e : m_mesh.edges()) {
                const auto h = m_mesh.halfedge(e);
                const auto& p0 = m_mesh.point(m_mesh.source(h));
                const auto& p1 = m_mesh.point(m_mesh.target(h));
                const double dx = CGAL::to_double(p0.x() - p1.x());
                const double dy = CGAL::to_double(p0.y() - p1.y());
                const double dz = CGAL::to_double(p0.z() - p1.z());
                sum += std::sqrt(dx*dx + dy*dy + dz*dz);
                ++cnt;
            }
            if (cnt > 0) target = sum / static_cast<double>(cnt);
            if (!(target > 0.0)) target = 1.0; // safe fallback
        }
        PMP::isotropic_remeshing(faces(m_mesh), target, m_mesh,
                                 PMP::parameters::number_of_iterations(options->remesh_iterations)
                                     .protect_constraints(false));
    }

    // Smoothing
    if (options->smooth_iterations > 0) {
        PMP::smooth_mesh(m_mesh, PMP::parameters::number_of_iterations(options->smooth_iterations));
    }

    if (options->recompute_normals) {
        computeMeshNormals();
    }

    return true;
}
