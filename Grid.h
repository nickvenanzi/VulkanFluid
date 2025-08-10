#pragma once

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include "Vertex.h"

class Grid
{
public:
    Grid();
    float getPhi(uint32_t x_i, uint32_t y_i, uint32_t z_i);
    glm::vec3 getPosition(uint32_t x_i, uint32_t y_i, uint32_t z_i);
    void constructSurface(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices);

private:
    std::vector<float> phi;
};