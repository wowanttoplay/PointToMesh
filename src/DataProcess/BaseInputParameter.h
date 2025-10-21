//
// Created by 范杰 on 2025/10/21.
//

#ifndef POINTTOMESH_BASEINPUTPARAMETER_H
#define POINTTOMESH_BASEINPUTPARAMETER_H

#include <QObject>

class BaseInputParameter : public QObject {
    Q_OBJECT
public:
    explicit BaseInputParameter(QObject *parent = nullptr) : QObject(parent) {}
    ~BaseInputParameter() override = default;
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

    int iterations_number = 4;
};

class AdvancingFrontReconstructionParameter : public BaseInputParameter {
    Q_OBJECT
public:
    explicit AdvancingFrontReconstructionParameter(QObject *parent = nullptr) : BaseInputParameter(parent) {}
    ~AdvancingFrontReconstructionParameter() override = default;
};

#endif //POINTTOMESH_BASEINPUTPARAMETER_H
