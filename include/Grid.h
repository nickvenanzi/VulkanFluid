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
    void constructSurface(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

private:
    std::array<std::vector<float>, 2> phi_arrays;

    std::array<std::vector<float>, 2> u_minus_arrays;
    std::array<std::vector<float>, 2> v_minus_arrays;
    std::array<std::vector<float>, 2> w_minus_arrays;

    uint32_t oldStorage = 0;
    uint32_t newStorage = 1;
};