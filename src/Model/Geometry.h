#pragma once
#include <memory>
#include <vector>
#include <QVector3D>
#include <cstdint>

struct PointCloudModel {
    std::vector<QVector3D> points;
};

struct MeshModel {
    std::vector<QVector3D> vertices;
    std::vector<std::uint32_t> indices; // triangle list (3*i,3*i+1,3*i+2)
};

using PointCloudPtr = std::shared_ptr<const PointCloudModel>;
using MeshPtr       = std::shared_ptr<const MeshModel>;

// Declare metatypes for Qt's signal/slot mechanism
#include <QMetaType>
Q_DECLARE_METATYPE(PointCloudPtr)
Q_DECLARE_METATYPE(MeshPtr)

