#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "Vertex.h"

class Grid
{
public:
    Grid();
    glm::vec3 getPosition(uint32_t x_i, uint32_t y_i, uint32_t z_i);
    void advect(float deltaT);
    void updateSOE(float deltaT);
    void solveSOE();
    void project(float deltaT);
    void constructSurface(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);
    void flipStorage();

private:
    std::array<std::vector<float>, 2> phi_arrays;

    std::array<std::vector<float>, 2> u_minus_arrays;
    std::array<std::vector<float>, 2> v_minus_arrays;
    std::array<std::vector<float>, 2> w_minus_arrays;

    std::vector<float> AplusI;
    std::vector<float> AplusJ;
    std::vector<float> AplusK;
    std::vector<float> Adiag;
    std::vector<float> D;
    std::vector<float> pressures;

    uint32_t oldStorage = 0;
    uint32_t newStorage = 1;

    // SOE Solver variables:
    std::vector<float> residuals;
    std::vector<float> conjugates;

    // SOE Solver helpers:
    void mulA(const std::vector<float> &x, std::vector<float> &result);
    float dot(const std::vector<float> &a, const std::vector<float> &b);
    void sumC(const std::vector<float> &a, const std::vector<float> &b, float C, std::vector<float> &result);
};