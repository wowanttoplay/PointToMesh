//
// Created by 范杰 on 2025/10/21.
//

#ifndef POINTTOMESH_BASEINPUTPARAMETER_H
#define POINTTOMESH_BASEINPUTPARAMETER_H

#include <QObject>
#include <memory>
#include <QString>

class BaseInputParameter : public QObject {
    Q_OBJECT
public:
    explicit BaseInputParameter(QObject *parent = nullptr) : QObject(parent) {}
    ~BaseInputParameter() override = default;

    // Create a deep copy without a QObject parent (safe to move across threads)
    [[nodiscard]] virtual std::unique_ptr<BaseInputParameter> clone() const = 0;

    // Optional: return a tooltip/description for a given property name; empty if not available
    [[nodiscard]] virtual QString propertyToolTip(const QString& name) const { Q_UNUSED(name); return {}; }
};

class PoissonReconstructionParameter : public BaseInputParameter {
    Q_OBJECT
    // expose commonly used numeric parameters to Qt meta-object so a UI generator can inspect them
    Q_PROPERTY(double angle MEMBER angle)
    Q_PROPERTY(double radius MEMBER radius)
    Q_PROPERTY(double distance MEMBER distance)
    Q_PROPERTY(int neighbors_number MEMBER neighbors_number)
    Q_PROPERTY(double spacing_scale MEMBER spacing_scale)

public:
    explicit PoissonReconstructionParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~PoissonReconstructionParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<PoissonReconstructionParameter>();
        copy->angle = angle;
        copy->radius = radius;
        copy->distance = distance;
        copy->neighbors_number = neighbors_number;
        copy->spacing_scale = spacing_scale;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "angle") return QStringLiteral("Minimum triangle angle (degrees). Larger avoids skinny triangles; too large may lose details.");
        if (name == "radius") return QStringLiteral("Max triangle size multiplier: max edge ≈ radius × average spacing. Smaller suppresses long edges but may create holes.");
        if (name == "distance") return QStringLiteral("Allowed point-to-surface deviation ≈ distance × average spacing. Smaller fits data (noisier); larger is smoother.");
        if (name == "neighbors_number") return QStringLiteral("Neighbor count used to estimate average spacing. Affects stability and performance.");
        if (name == "spacing_scale") return QStringLiteral("Scale applied to estimated average spacing. >1 smoother/coarser; <1 tighter/more details.");
        return {};
    }

    double angle = 20.0;
    double radius = 30.0;
    double distance = 0.375;
    int neighbors_number = 6; // used to calculate average spacing
    double spacing_scale = 1.0; // used to scale average spacing
};

class ScaleSpaceReconstructionParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(int iterations_number MEMBER iterations_number)
public:
    explicit ScaleSpaceReconstructionParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~ScaleSpaceReconstructionParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<ScaleSpaceReconstructionParameter>();
        copy->iterations_number = iterations_number;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "iterations_number") return QStringLiteral("Number of scale-increase iterations. More is smoother/simpler (possible detail loss).");
        return {};
    }

    int iterations_number = 4;
};

class AdvancingFrontReconstructionParameter : public BaseInputParameter {
    Q_OBJECT
public:
    explicit AdvancingFrontReconstructionParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~AdvancingFrontReconstructionParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        return std::make_unique<AdvancingFrontReconstructionParameter>();
    }
};

// New: Mesh post-process parameters moved from MeshPostprocessOptions
class MeshPostprocessParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(int keep_largest_components MEMBER keep_largest_components)
    Q_PROPERTY(bool remove_degenerate_faces MEMBER remove_degenerate_faces)
    Q_PROPERTY(bool remove_isolated_vertices MEMBER remove_isolated_vertices)
    Q_PROPERTY(bool stitch_borders MEMBER stitch_borders)
    Q_PROPERTY(int fill_holes_max_cycle_edges MEMBER fill_holes_max_cycle_edges)
    Q_PROPERTY(int remesh_iterations MEMBER remesh_iterations)
    Q_PROPERTY(double remesh_target_edge_length MEMBER remesh_target_edge_length)
    Q_PROPERTY(int smooth_iterations MEMBER smooth_iterations)
    Q_PROPERTY(bool recompute_normals MEMBER recompute_normals)
public:
    explicit MeshPostprocessParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~MeshPostprocessParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<MeshPostprocessParameter>();
        copy->keep_largest_components = keep_largest_components;
        copy->remove_degenerate_faces = remove_degenerate_faces;
        copy->remove_isolated_vertices = remove_isolated_vertices;
        copy->stitch_borders = stitch_borders;
        copy->fill_holes_max_cycle_edges = fill_holes_max_cycle_edges;
        copy->remesh_iterations = remesh_iterations;
        copy->remesh_target_edge_length = remesh_target_edge_length;
        copy->smooth_iterations = smooth_iterations;
        copy->recompute_normals = recompute_normals;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "keep_largest_components") return QStringLiteral("Keep only the largest N connected components. 0 keeps all; >1 helps remove floating fragments.");
        if (name == "remove_degenerate_faces") return QStringLiteral("Remove degenerate triangles (zero area/duplicate vertices) to avoid downstream issues.");
        if (name == "remove_isolated_vertices") return QStringLiteral("Remove vertices not used by any face to clean the mesh.");
        if (name == "stitch_borders") return QStringLiteral("Stitch near-coincident boundary edges to close cracks, aiding hole filling and remeshing.");
        if (name == "fill_holes_max_cycle_edges") return QStringLiteral("Fill holes whose border cycle length is ≤ this value. Larger fills more; too large may close real openings.");
        if (name == "remesh_iterations") return QStringLiteral("Number of isotropic remeshing iterations. More improves triangle quality, increases resampling.");
        if (name == "remesh_target_edge_length") return QStringLiteral("Target edge length. 0 uses average edge length; smaller subdivides, larger simplifies.");
        if (name == "smooth_iterations") return QStringLiteral("Number of angle-and-area smoothing iterations. More smoothing, possible shrinkage.");
        if (name == "recompute_normals") return QStringLiteral("Recompute per-vertex normals for consistent/updated normals (also used on export).");
        return {};
    }

    int keep_largest_components = 0;
    bool remove_degenerate_faces = true;
    bool remove_isolated_vertices = true;
    bool stitch_borders = false;
    int fill_holes_max_cycle_edges = 0;
    int remesh_iterations = 0;
    double remesh_target_edge_length = 0.0;
    int smooth_iterations = 0;
    bool recompute_normals = true;
};

// New: AABB filter parameters
class AABBFilterParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(double min_x MEMBER min_x)
    Q_PROPERTY(double min_y MEMBER min_y)
    Q_PROPERTY(double min_z MEMBER min_z)
    Q_PROPERTY(double max_x MEMBER max_x)
    Q_PROPERTY(double max_y MEMBER max_y)
    Q_PROPERTY(double max_z MEMBER max_z)
    Q_PROPERTY(bool keepInside MEMBER keepInside)
public:
    explicit AABBFilterParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~AABBFilterParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<AABBFilterParameter>();
        copy->min_x = min_x; copy->min_y = min_y; copy->min_z = min_z;
        copy->max_x = max_x; copy->max_y = max_y; copy->max_z = max_z;
        copy->keepInside = keepInside;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "min_x" || name == "min_y" || name == "min_z") return QStringLiteral("Minimum coordinate of the axis-aligned bounding box (AABB).");
        if (name == "max_x" || name == "max_y" || name == "max_z") return QStringLiteral("Maximum coordinate of the axis-aligned bounding box (AABB).");
        if (name == "keepInside") return QStringLiteral("If true, keep points inside the box; otherwise remove inside points (keep outside).");
        return {};
    }

    double min_x = 0.0;
    double min_y = 0.0;
    double min_z = 0.0;
    double max_x = 0.0;
    double max_y = 0.0;
    double max_z = 0.0;
    bool keepInside = true;
};

// New: Sphere filter parameters
class SphereFilterParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(double cx MEMBER cx)
    Q_PROPERTY(double cy MEMBER cy)
    Q_PROPERTY(double cz MEMBER cz)
    Q_PROPERTY(double radius MEMBER radius)
    Q_PROPERTY(bool keepInside MEMBER keepInside)
public:
    explicit SphereFilterParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~SphereFilterParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<SphereFilterParameter>();
        copy->cx = cx; copy->cy = cy; copy->cz = cz; copy->radius = radius; copy->keepInside = keepInside;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "cx" || name == "cy" || name == "cz") return QStringLiteral("Sphere center coordinate.");
        if (name == "radius") return QStringLiteral("Sphere radius; controls the selection region size.");
        if (name == "keepInside") return QStringLiteral("If true, keep points inside the sphere; otherwise remove inside points (keep outside).");
        return {};
    }

    double cx = 0.0;
    double cy = 0.0;
    double cz = 0.0;
    double radius = 1.0;
    bool keepInside = true;
};

// New: Surface-from-uniform-volume filter parameters
class UniformVolumeSurfaceFilterParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(int neighbors_number MEMBER neighbors_number)
    Q_PROPERTY(double radius_scale MEMBER radius_scale)
    Q_PROPERTY(int max_neighbors MEMBER max_neighbors)
public:
    explicit UniformVolumeSurfaceFilterParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~UniformVolumeSurfaceFilterParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<UniformVolumeSurfaceFilterParameter>();
        copy->neighbors_number = neighbors_number;
        copy->radius_scale = radius_scale;
        copy->max_neighbors = max_neighbors;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "neighbors_number") return QStringLiteral("Neighbors used to estimate average spacing. Typical: 24.");
        if (name == "radius_scale") return QStringLiteral("Radius = radius_scale × average spacing for neighbor counting. Larger radius increases counts.");
        if (name == "max_neighbors") return QStringLiteral("Keep points whose neighbor count within radius is ≤ this threshold (surface tends to have fewer neighbors).");
        return {};
    }

    int neighbors_number = 24;
    double radius_scale = 2.0;
    int max_neighbors = 24;
};

// New: Voxel downsample parameters
class VoxelDownsampleParameter : public BaseInputParameter {
    Q_OBJECT
    Q_PROPERTY(double cell_size MEMBER cell_size)
public:
    explicit VoxelDownsampleParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~VoxelDownsampleParameter() override = default;

    [[nodiscard]] std::unique_ptr<BaseInputParameter> clone() const override {
        auto copy = std::make_unique<VoxelDownsampleParameter>();
        copy->cell_size = cell_size;
        return copy;
    }

    [[nodiscard]] QString propertyToolTip(const QString& name) const override {
        if (name == "cell_size") return QStringLiteral("Voxel grid cell size (same unit as points). Larger removes more points, increasing sparsity.");
        return {};
    }

    double cell_size = 0.0;
};

Q_DECLARE_METATYPE(BaseInputParameter*)
// Optionally register derived pointer types as well
Q_DECLARE_METATYPE(MeshPostprocessParameter*)
Q_DECLARE_METATYPE(AABBFilterParameter*)
Q_DECLARE_METATYPE(SphereFilterParameter*)
Q_DECLARE_METATYPE(UniformVolumeSurfaceFilterParameter*)
Q_DECLARE_METATYPE(VoxelDownsampleParameter*)

#endif //POINTTOMESH_BASEINPUTPARAMETER_H
