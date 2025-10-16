#include "CGALPointCloudProcessor.h"
#include <iostream>
#include <fstream>

// CGAL I/O
#include <CGAL/IO/read_points.h>
// #include <CGAL/IO/write_surface_mesh.h>

// Normal Estimation
#include <CGAL/jet_estimate_normals.h>
#include <CGAL/mst_orient_normals.h>

// Poisson Reconstruction
#include <CGAL/poisson_surface_reconstruction.h>

CGALPointCloudProcessor::CGALPointCloudProcessor() = default;

CGALPointCloudProcessor::~CGALPointCloudProcessor() = default;

bool CGALPointCloudProcessor::loadPointCloud(const std::string &filePath) {
    m_pointCloud.clear();
    m_mesh.clear();

    std::ifstream stream(filePath);
    if (!stream) {
        std::cerr << "Error: Cannot open file " << filePath << std::endl;
        return false;
    }

    // Read points and normals. CGAL::read_points can handle files with 3 (points) or 6 (points+normals) columns.
    if (!CGAL::IO::read_points(stream, std::back_inserter(m_pointCloud))) {
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
            CGAL::jet_estimate_normals<CGAL::Sequential_tag>(m_pointCloud, k_neighbors);

            // Orient normals to be consistent
            CGAL::mst_orient_normals(m_pointCloud, k_neighbors);

            return true;
        }
        // Other cases for different normal estimation methods can be added here.
        default:
            std::cerr << "Error: Unsupported normal estimation method." << std::endl;
            return false;
    }
}

bool CGALPointCloudProcessor::processToMesh(MeshGenerationMethod meshMethod) {
    if (m_pointCloud.empty()) {
        std::cerr << "Error: Point cloud is empty." << std::endl;
        return false;
    }
    if (!hasNormals()) {
        std::cerr << "Error: Normals are required for mesh generation but were not found or estimated." << std::endl;
        return false;
    }

    switch (meshMethod) {
        case MeshGenerationMethod::POISSON_RECONSTRUCTION: {
            // Perform Poisson surface reconstruction
            m_mesh.clear();
            if (!CGAL::poisson_surface_reconstruction(m_pointCloud.begin(), m_pointCloud.end(), m_mesh)) {
                std::cerr << "Error: Poisson surface reconstruction failed." << std::endl;
                return false;
            }
            return true;
        }
        // Other cases for different mesh generation methods can be added here.
        default:
            std::cerr << "Error: Unsupported mesh generation method." << std::endl;
            return false;
    }
}

bool CGALPointCloudProcessor::exportMesh(const std::string &filePath) {
    if (m_mesh.is_empty()) {
        std::cerr << "Error: Mesh is empty. Generate a mesh first." << std::endl;
        return false;
    }

    if (!CGAL::IO::write_surface_mesh(filePath, m_mesh)) {
        std::cerr << "Error: Cannot write mesh to " << filePath << std::endl;
        return false;
    }

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
