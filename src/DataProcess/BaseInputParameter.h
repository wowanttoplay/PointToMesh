//
// Created by 范杰 on 2025/10/21.
//

#ifndef POINTTOMESH_BASEINPUTPARAMETER_H
#define POINTTOMESH_BASEINPUTPARAMETER_H

#endif //POINTTOMESH_BASEINPUTPARAMETER_H

class BaseInputParameter {

public:
    BaseInputParameter() = default;
    virtual ~BaseInputParameter() = default;
};

class PoissonReconstructionParameter : public BaseInputParameter {

public:
    PoissonReconstructionParameter() = default;
    virtual ~PoissonReconstructionParameter() override = default;

    double angle = 20.0;
    double radius = 30.0;
    double distance = 0.375;
    int neighbors_number = 6; // used to calculate average spacing
    double spacing_scale = 1.0; // used to scale average spacing
};

class ScaleSpaceReconstructionParameter : public BaseInputParameter {
public:
    ScaleSpaceReconstructionParameter() = default;
    virtual ~ScaleSpaceReconstructionParameter() override = default;

    int iterations_number = 4;
};

class AdvancingFrontReconstructionParameter : public BaseInputParameter {
public:
    AdvancingFrontReconstructionParameter() = default;
    virtual ~AdvancingFrontReconstructionParameter() override = default;
};