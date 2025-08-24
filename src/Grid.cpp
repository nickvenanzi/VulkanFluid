#include "Grid.h"
#include <cmath>
#include <iostream>
#include <algorithm>

constexpr uint32_t Nx = 40;
constexpr uint32_t Ny = 40;
constexpr uint32_t Nz = 40;
constexpr uint32_t NyNz = Ny * Nz;
constexpr float CELL_WIDTH = 10.0f / (float)Nx;
constexpr float INV_CELL_WIDTH = 1.0f / CELL_WIDTH;
constexpr glm::vec3 SURFACE_COLOR = {1.0f, 1.0f, 1.0f};
constexpr glm::vec3 globalOffset = {-(Nx * CELL_WIDTH) / 2.0f, -(Ny *CELL_WIDTH) / 2.0f, -(Nz *CELL_WIDTH) / 2.0f};
constexpr std::array<float, 4> BODY_FORCES = {0.0f, 0.0f, 0.1f, 0.0f}; // gravity
constexpr float RHO = 1000.0f;
constexpr uint32_t MAX_ITERATIONS = 100;

#define Triple std::array<uint32_t, 3>
#define MarchingCube std::vector<Triple>

const MarchingCube marchingCubeLookup[256] = {
    {},                                               // 0
    {{0, 1, 4}},                                      // 1
    {{0, 2, 5}},                                      // 2
    {{1, 2, 4}, {2, 4, 5}},                           // 3
    {{1, 3, 6}},                                      // 4
    {{0, 3, 4}, {3, 4, 6}},                           // 5
    {{0, 2, 5}, {1, 3, 6}},                           // 6
    {{2, 3, 4}, {2, 4, 5}, {3, 4, 6}},                // 7
    {{2, 3, 7}},                                      // 8
    {{0, 1, 4}, {2, 3, 7}},                           // 9
    {{0, 3, 5}, {3, 5, 7}},                           // 10
    {{1, 3, 4}, {3, 4, 5}, {3, 5, 7}},                // 11
    {{1, 2, 6}, {2, 6, 7}},                           // 12
    {{0, 2, 4}, {2, 4, 6}, {2, 6, 7}},                // 13
    {{1, 6, 7}, {0, 1, 7}, {0, 5, 7}},                // 14
    {{4, 5, 6}, {5, 6, 7}},                           // 15
    {{4, 8, 9}},                                      // 16
    {{0, 1, 8}, {1, 8, 9}},                           // 17
    {{0, 2, 5}, {4, 8, 9}},                           // 18
    {{1, 2, 5}, {1, 5, 8}, {1, 8, 9}},                // 19
    {{1, 3, 6}, {4, 8, 9}},                           // 20
    {{0, 3, 6}, {0, 6, 9}, {0, 8, 9}},                // 21
    {{0, 2, 5}, {1, 3, 6}, {4, 8, 9}},                // 22
    {{2, 3, 6}, {2, 6, 9}, {2, 8, 9}, {2, 5, 8}},     // 23
    {{2, 3, 7}, {4, 8, 9}},                           // 24
    {{0, 1, 8}, {1, 8, 9}, {2, 3, 7}},                // 25
    {{0, 3, 5}, {3, 5, 7}, {4, 8, 9}},                // 26
    {{1, 3, 9}, {3, 7, 9}, {7, 8, 9}, {5, 7, 8}},     // 27
    {{1, 2, 6}, {2, 6, 7}, {4, 8, 9}},                // 28
    {{2, 6, 7}, {2, 6, 8}, {6, 8, 9}, {0, 2, 8}},     // 29
    {{1, 6, 7}, {0, 1, 7}, {0, 5, 7}, {4, 8, 9}},     // 30
    {{6, 7, 9}, {7, 8, 9}, {5, 7, 8}},                // 31
    {{5, 8, 10}},                                     // 32
    {{0, 1, 4}, {5, 8, 10}},                          // 33
    {{0, 2, 8}, {2, 8, 10}},                          // 34
    {{1, 2, 4}, {2, 4, 8}, {2, 8, 10}},               // 35
    {{1, 3, 6}, {5, 8, 10}},                          // 36
    {{0, 3, 4}, {3, 4, 6}, {5, 8, 10}},               // 37
    {{0, 2, 8}, {2, 8, 10}, {1, 3, 6}},               // 38
    {{2, 3, 6}, {2, 4, 6}, {2, 4, 8}, {2, 8, 10}},    // 39
    {{2, 3, 7}, {5, 8, 10}},                          // 40
    {{0, 1, 4}, {2, 3, 7}, {5, 8, 10}},               // 41
    {{0, 3, 7}, {0, 7, 10}, {0, 8, 10}},              // 42
    {{1, 3, 7}, {1, 7, 10}, {1, 8, 10}, {1, 4, 8}},   // 43
    {{1, 2, 6}, {2, 6, 7}, {5, 8, 10}},               // 44
    {{0, 2, 4}, {2, 4, 6}, {2, 6, 7}, {5, 8, 10}},    // 45
    {{0, 1, 8}, {1, 8, 10}, {1, 6, 10}, {6, 7, 10}},  // 46
    {{4, 6, 8}, {6, 8, 10}, {6, 7, 10}},              // 47
    {{4, 5, 9}, {5, 9, 10}},                          // 48
    {{0, 1, 9}, {0, 5, 9}, {5, 9, 10}},               // 49
    {{0, 2, 10}, {0, 4, 10}, {4, 9, 10}},             // 50
    {{1, 2, 9}, {2, 9, 10}},                          // 51
    {{4, 5, 9}, {5, 9, 10}, {1, 3, 6}},               // 52
    {{0, 3, 6}, {0, 6, 9}, {0, 5, 10}, {0, 9, 10}},   // 53
    {{0, 2, 10}, {0, 4, 10}, {4, 9, 10}, {1, 3, 6}},  // 54
    {{2, 3, 10}, {3, 6, 10}, {6, 9, 10}},             // 55
    {{4, 5, 9}, {5, 9, 10}, {2, 3, 7}},               // 56
    {{0, 1, 9}, {0, 5, 9}, {5, 9, 10}, {2, 3, 7}},    // 57
    {{0, 4, 9}, {0, 3, 9}, {3, 9, 10}, {3, 7, 10}},   // 58
    {{1, 3, 9}, {3, 7, 9}, {7, 9, 10}},               // 59
    {{1, 2, 6}, {2, 6, 7}, {4, 5, 9}, {5, 9, 10}},    // 60
    {{6, 7, 9}, {7, 9, 10}, {0, 2, 5}},               // 61
    {{6, 7, 9}, {7, 9, 10}, {0, 1, 4}},               // 62
    {{6, 7, 9}, {7, 9, 10}},                          // 63
    {{6, 9, 11}},                                     // 64
    {{0, 1, 4}, {6, 9, 11}},                          // 65
    {{0, 2, 5}, {6, 9, 11}},                          // 66
    {{1, 2, 4}, {2, 4, 5}, {6, 9, 11}},               // 67
    {{1, 3, 9}, {3, 9, 11}},                          // 68
    {{0, 3, 4}, {3, 4, 9}, {3, 9, 11}},               // 69
    {{1, 3, 9}, {3, 9, 11}, {0, 2, 5}},               // 70
    {{2, 4, 5}, {2, 4, 11}, {4, 9, 11}, {2, 3, 11}},  // 71
    {{2, 3, 7}, {6, 9, 11}},                          // 72
    {{0, 1, 4}, {2, 3, 7}, {6, 9, 11}},               // 73
    {{0, 3, 5}, {3, 5, 7}, {6, 9, 11}},               // 74
    {{1, 3, 4}, {3, 4, 5}, {3, 5, 7}, {6, 9, 11}},    // 75
    {{1, 2, 7}, {1, 7, 11}, {1, 9, 11}},              // 76
    {{0, 4, 9}, {0, 9, 11}, {0, 7, 11}, {0, 2, 7}},   // 77
    {{0, 1, 9}, {0, 5, 9}, {5, 9, 11}, {5, 7, 11}},   // 78
    {{4, 5, 9}, {5, 9, 11}, {5, 7, 11}},              // 79
    {{4, 6, 8}, {6, 8, 11}},                          // 80
    {{0, 1, 8}, {1, 6, 8}, {6, 8, 11}},               // 81
    {{4, 6, 8}, {6, 8, 11}, {0, 2, 5}},               // 82
    {{1, 6, 11}, {1, 2, 11}, {2, 5, 11}, {5, 8, 11}}, // 83
    {{1, 3, 11}, {1, 4, 11}, {4, 8, 11}},             // 84
    {{0, 3, 8}, {3, 8, 11}},                          // 85
    {{1, 3, 11}, {1, 4, 11}, {4, 8, 11}, {0, 2, 5}},  // 86
    {{2, 3, 11}, {2, 5, 11}, {5, 8, 11}},             // 87
    {{4, 6, 8}, {6, 8, 11}, {2, 3, 7}},               // 88
    {{0, 1, 8}, {1, 6, 8}, {6, 8, 11}, {2, 3, 7}},    // 89
    {{0, 3, 5}, {3, 5, 7}, {4, 6, 8}, {6, 8, 11}},    // 90
    {{5, 7, 8}, {7, 8, 11}, {1, 3, 6}},               // 91
    {{2, 7, 11}, {1, 2, 11}, {1, 8, 11}, {1, 4, 8}},  // 92
    {{0, 2, 8}, {2, 7, 8}, {7, 8, 11}},               // 93
    {{5, 7, 8}, {7, 8, 11}, {0, 1, 4}},               // 94
    {{5, 7, 8}, {7, 8, 11}},                          // 95
    {{5, 8, 10}, {6, 9, 11}},                         // 96
    {{0, 1, 4}, {5, 8, 10}, {6, 9, 11}},              // 97
    {{0, 2, 8}, {2, 8, 10}, {6, 9, 11}},              // 98
    {{1, 2, 4}, {2, 4, 8}, {2, 8, 10}, {6, 9, 11}},   // 99
    {{1, 3, 9}, {3, 9, 11}, {5, 8, 10}},              // 100
    {{0, 3, 4}, {3, 4, 9}, {3, 9, 11}, {5, 8, 10}},   // 101
    {{1, 3, 9}, {3, 9, 11}, {0, 2, 8}, {2, 8, 10}},   // 102
    {{2, 3, 10}, {3, 10, 11}, {4, 8, 9}},             // 103
    {{2, 3, 7}, {5, 8, 10}, {6, 9, 11}},              // 104
    {{0, 1, 4}, {2, 3, 7}, {5, 8, 10}, {6, 9, 11}},   // 105
    {{0, 3, 7}, {0, 7, 10}, {0, 8, 10}, {6, 9, 11}},  // 106
    {{1, 3, 6}, {4, 8, 9}, {7, 10, 11}},              // 107
    {{1, 2, 7}, {1, 7, 11}, {1, 9, 11}, {5, 8, 10}},  // 108
    {{0, 2, 5}, {4, 8, 9}, {7, 10, 11}},              // 109
    {{0, 1, 8}, {1, 8, 9}, {7, 10, 11}},              // 110
    {{4, 8, 9}, {7, 10, 11}},                         // 111
    {{4, 6, 11}, {4, 10, 11}, {4, 5, 10}},            // 112
    {{0, 1, 6}, {0, 6, 11}, {0, 10, 11}, {0, 5, 10}}, // 113
    {{0, 4, 6}, {0, 2, 6}, {2, 6, 11}, {2, 10, 11}},  // 114
    {{1, 2, 6}, {2, 6, 11}, {2, 10, 11}},             // 115
    {{1, 3, 11}, {1, 5, 11}, {1, 4, 5}, {5, 10, 11}}, // 116
    {{0, 3, 5}, {3, 5, 10}, {3, 10, 11}},             // 117
    {{2, 3, 10}, {3, 10, 11}, {0, 1, 4}},             // 118
    {{2, 3, 10}, {3, 10, 11}},                        // 119
    {{4, 6, 11}, {4, 10, 11}, {4, 5, 10}, {2, 3, 7}}, // 120
    {{0, 2, 5}, {1, 3, 6}, {7, 10, 11}},              // 121
    {{0, 3, 4}, {3, 4, 6}, {7, 10, 11}},              // 122
    {{1, 3, 6}, {7, 10, 11}},                         // 123
    {{1, 2, 4}, {2, 4, 5}, {7, 10, 11}},              // 124
    {{0, 2, 5}, {7, 10, 11}},                         // 125
    {{0, 1, 4}, {7, 10, 11}},                         // 126
    {{7, 10, 11}},                                    // 127
    {{7, 10, 11}},                                    // 128
    {{0, 1, 4}, {7, 10, 11}},                         // 129
    {{0, 2, 5}, {7, 10, 11}},                         // 130
    {{1, 2, 4}, {2, 4, 5}, {7, 10, 11}},              // 131
    {{1, 3, 6}, {7, 10, 11}},                         // 132
    {{0, 3, 4}, {3, 4, 6}, {7, 10, 11}},              // 133
    {{0, 2, 5}, {1, 3, 6}, {7, 10, 11}},              // 134
    {{2, 3, 4}, {2, 4, 5}, {3, 4, 6}, {7, 10, 11}},   // 135
    {{2, 3, 10}, {3, 10, 11}},                        // 136
    {{2, 3, 10}, {3, 10, 11}, {0, 1, 4}},             // 137
    {{0, 3, 5}, {3, 5, 10}, {3, 10, 11}},             // 138
    {{1, 3, 11}, {1, 5, 11}, {1, 4, 5}, {5, 10, 11}}, // 139
    {{1, 2, 6}, {2, 6, 11}, {2, 10, 11}},             // 140
    {{0, 4, 6}, {0, 2, 6}, {2, 6, 11}, {2, 10, 11}},  // 141
    {{0, 1, 6}, {0, 6, 11}, {0, 10, 11}, {0, 5, 10}}, // 142
    {{4, 6, 11}, {4, 10, 11}, {4, 5, 10}},            // 143
    {{4, 8, 9}, {7, 10, 11}},                         // 144
    {{0, 1, 8}, {1, 8, 9}, {7, 10, 11}},              // 145
    {{0, 2, 5}, {4, 8, 9}, {7, 10, 11}},              // 146
    {{1, 2, 5}, {1, 5, 8}, {1, 8, 9}, {7, 10, 11}},   // 147
    {{1, 3, 6}, {4, 8, 9}, {7, 10, 11}},              // 148
    {{0, 3, 6}, {0, 6, 9}, {0, 8, 9}, {7, 10, 11}},   // 149
    {{0, 2, 5}, {1, 3, 6}, {4, 8, 9}, {7, 10, 11}},   // 150
    {{2, 3, 7}, {5, 8, 10}, {6, 9, 11}},              // 151
    {{2, 3, 10}, {3, 10, 11}, {4, 8, 9}},             // 152
    {{0, 1, 8}, {1, 8, 9}, {2, 3, 10}, {3, 10, 11}},  // 153
    {{0, 3, 5}, {3, 5, 10}, {3, 10, 11}, {4, 8, 9}},  // 154
    {{1, 3, 9}, {3, 9, 11}, {5, 8, 10}},              // 155
    {{1, 2, 6}, {2, 6, 11}, {2, 10, 11}, {4, 8, 9}},  // 156
    {{0, 2, 8}, {2, 8, 10}, {6, 9, 11}},              // 157
    {{0, 1, 4}, {5, 8, 10}, {6, 9, 11}},              // 158
    {{5, 8, 10}, {6, 9, 11}},                         // 159
    {{5, 7, 8}, {7, 8, 11}},                          // 160
    {{5, 7, 8}, {7, 8, 11}, {0, 1, 4}},               // 161
    {{0, 2, 8}, {2, 7, 8}, {7, 8, 11}},               // 162
    {{2, 7, 11}, {1, 2, 11}, {1, 8, 11}, {1, 4, 8}},  // 163
    {{5, 7, 8}, {7, 8, 11}, {1, 3, 6}},               // 164
    {{0, 3, 4}, {3, 4, 6}, {5, 7, 8}, {7, 8, 11}},    // 165
    {{0, 2, 8}, {2, 7, 8}, {7, 8, 11}, {1, 3, 6}},    // 166
    {{4, 6, 8}, {6, 8, 11}, {2, 3, 7}},               // 167
    {{2, 3, 11}, {2, 5, 11}, {5, 8, 11}},             // 168
    {{2, 3, 11}, {2, 5, 11}, {5, 8, 11}, {0, 1, 4}},  // 169
    {{0, 3, 8}, {3, 8, 11}},                          // 170
    {{1, 3, 11}, {1, 4, 11}, {4, 8, 11}},             // 171
    {{1, 6, 11}, {1, 2, 11}, {2, 5, 11}, {5, 8, 11}}, // 172
    {{4, 6, 8}, {6, 8, 11}, {0, 2, 5}},               // 173
    {{0, 1, 8}, {1, 6, 8}, {6, 8, 11}},               // 174
    {{4, 6, 8}, {6, 8, 11}},                          // 175
    {{4, 5, 9}, {5, 9, 11}, {5, 7, 11}},              // 176
    {{0, 1, 9}, {0, 5, 9}, {5, 9, 11}, {5, 7, 11}},   // 177
    {{0, 4, 9}, {0, 9, 11}, {0, 7, 11}, {0, 2, 7}},   // 178
    {{1, 2, 7}, {1, 7, 11}, {1, 9, 11}},              // 179
    {{4, 5, 9}, {5, 9, 11}, {5, 7, 11}, {1, 3, 6}},   // 180
    {{0, 3, 5}, {3, 5, 7}, {6, 9, 11}},               // 181
    {{0, 1, 4}, {2, 3, 7}, {6, 9, 11}},               // 182
    {{2, 3, 7}, {6, 9, 11}},                          // 183
    {{2, 4, 5}, {2, 4, 11}, {4, 9, 11}, {2, 3, 11}},  // 184
    {{1, 3, 9}, {3, 9, 11}, {0, 2, 5}},               // 185
    {{0, 3, 4}, {3, 4, 9}, {3, 9, 11}},               // 186
    {{1, 3, 9}, {3, 9, 11}},                          // 187
    {{1, 2, 4}, {2, 4, 5}, {6, 9, 11}},               // 188
    {{0, 2, 5}, {6, 9, 11}},                          // 189
    {{0, 1, 4}, {6, 9, 11}},                          // 190
    {{6, 9, 11}},                                     // 191
    {{6, 7, 9}, {7, 9, 10}},                          // 192
    {{6, 7, 9}, {7, 9, 10}, {0, 1, 4}},               // 193
    {{6, 7, 9}, {7, 9, 10}, {0, 2, 5}},               // 194
    {{1, 2, 4}, {2, 4, 5}, {6, 7, 9}, {7, 9, 10}},    // 195
    {{1, 3, 9}, {3, 7, 9}, {7, 9, 10}},               // 196
    {{0, 4, 9}, {0, 3, 9}, {3, 9, 10}, {3, 7, 10}},   // 197
    {{1, 3, 9}, {3, 7, 9}, {7, 9, 10}, {0, 2, 5}},    // 198
    {{4, 5, 9}, {5, 9, 10}, {2, 3, 7}},               // 199
    {{2, 3, 10}, {3, 6, 10}, {6, 9, 10}},             // 200
    {{2, 3, 10}, {3, 6, 10}, {6, 9, 10}, {0, 1, 4}},  // 201
    {{0, 3, 6}, {0, 6, 9}, {0, 5, 10}, {0, 9, 10}},   // 202
    {{4, 5, 9}, {5, 9, 10}, {1, 3, 6}},               // 203
    {{1, 2, 9}, {2, 9, 10}},                          // 204
    {{0, 2, 10}, {0, 4, 10}, {4, 9, 10}},             // 205
    {{0, 1, 9}, {0, 5, 9}, {5, 9, 10}},               // 206
    {{4, 5, 9}, {5, 9, 10}},                          // 207
    {{4, 6, 8}, {6, 8, 10}, {6, 7, 10}},              // 208
    {{0, 1, 8}, {1, 8, 10}, {1, 6, 10}, {6, 7, 10}},  // 209
    {{4, 6, 8}, {6, 8, 10}, {6, 7, 10}, {0, 2, 5}},   // 210
    {{1, 2, 6}, {2, 6, 7}, {5, 8, 10}},               // 211
    {{1, 3, 7}, {1, 7, 10}, {1, 8, 10}, {1, 4, 8}},   // 212
    {{0, 3, 7}, {0, 7, 10}, {0, 8, 10}},              // 213
    {{0, 1, 4}, {2, 3, 7}, {5, 8, 10}},               // 214
    {{2, 3, 7}, {5, 8, 10}},                          // 215
    {{2, 3, 6}, {2, 4, 6}, {2, 4, 8}, {2, 8, 10}},    // 216
    {{0, 2, 8}, {2, 8, 10}, {1, 3, 6}},               // 217
    {{0, 3, 4}, {3, 4, 6}, {5, 8, 10}},               // 218
    {{1, 3, 6}, {5, 8, 10}},                          // 219
    {{1, 2, 4}, {2, 4, 8}, {2, 8, 10}},               // 220
    {{0, 2, 8}, {2, 8, 10}},                          // 221
    {{0, 1, 4}, {5, 8, 10}},                          // 222
    {{5, 8, 10}},                                     // 223
    {{6, 7, 9}, {7, 8, 9}, {5, 7, 8}},                // 224
    {{6, 7, 9}, {7, 8, 9}, {5, 7, 8}, {0, 1, 4}},     // 225
    {{2, 6, 7}, {2, 6, 8}, {6, 8, 9}, {0, 2, 8}},     // 226
    {{1, 2, 6}, {2, 6, 7}, {4, 8, 9}},                // 227
    {{1, 3, 9}, {3, 7, 9}, {7, 8, 9}, {5, 7, 8}},     // 228
    {{0, 3, 5}, {3, 5, 7}, {4, 8, 9}},                // 229
    {{0, 1, 8}, {1, 8, 9}, {2, 3, 7}},                // 230
    {{2, 3, 7}, {4, 8, 9}},                           // 231
    {{2, 3, 6}, {2, 6, 9}, {2, 8, 9}, {2, 5, 8}},     // 232
    {{0, 2, 5}, {1, 3, 6}, {4, 8, 9}},                // 233
    {{0, 3, 6}, {0, 6, 9}, {0, 8, 9}},                // 234
    {{1, 3, 6}, {4, 8, 9}},                           // 235
    {{1, 2, 5}, {1, 5, 8}, {1, 8, 9}},                // 236
    {{0, 2, 5}, {4, 8, 9}},                           // 237
    {{0, 1, 8}, {1, 8, 9}},                           // 238
    {{4, 8, 9}},                                      // 239
    {{4, 5, 6}, {5, 6, 7}},                           // 240
    {{1, 6, 7}, {0, 1, 7}, {0, 5, 7}},                // 241
    {{0, 2, 4}, {2, 4, 6}, {2, 6, 7}},                // 242
    {{1, 2, 6}, {2, 6, 7}},                           // 243
    {{1, 3, 4}, {3, 4, 5}, {3, 5, 7}},                // 244
    {{0, 3, 5}, {3, 5, 7}},                           // 245
    {{0, 1, 4}, {2, 3, 7}},                           // 246
    {{2, 3, 7}},                                      // 247
    {{2, 3, 4}, {2, 4, 5}, {3, 4, 6}},                // 248
    {{0, 2, 5}, {1, 3, 6}},                           // 249
    {{0, 3, 4}, {3, 4, 6}},                           // 250
    {{1, 3, 6}},                                      // 251
    {{1, 2, 4}, {2, 4, 5}},                           // 252
    {{0, 2, 5}},                                      // 253
    {{0, 1, 4}},                                      // 254
    {}                                                // 255
};

Grid::Grid()
{
    for (uint32_t storage_idx = 0; storage_idx < 2; storage_idx++)
    {
        phi_arrays[storage_idx].resize(Nx * Ny * Nz);
        u_minus_arrays[storage_idx].resize((Nx + 1) * Ny * Nz);
        v_minus_arrays[storage_idx].resize(Nx * (Ny + 1) * Nz);
        w_minus_arrays[storage_idx].resize(Nx * Ny * (Nz + 1));
    }
    Adiag.resize(Nx * Ny * Nz);
    AplusI.resize(Nx * Ny * Nz);
    AplusJ.resize(Nx * Ny * Nz);
    AplusK.resize(Nx * Ny * Nz);
    D.resize(Nx * Ny * Nz);
    pressures.resize(Nx * Ny * Nz);

    // solver:
    residuals.resize(Nx * Ny * Nz);
    conjugates.resize(Nx * Ny * Nz);

    // add sphere
    float radius = 4.0f;
    glm::vec3 center = {0.0f, 0.0f, 0.0f};
    uint32_t index = 0;
    for (uint32_t i = 0; i < Nx; i++)
    {
        for (uint32_t j = 0; j < Ny; j++)
        {
            for (uint32_t k = 0; k < Nz; k++)
            {
                glm::vec3 position = getPosition(i, j, k);
                float distance = std::sqrt((position.x - center.x) * (position.x - center.x) + (position.y - center.y) * (position.y - center.y) + (position.z - center.z) * (position.z - center.z)) - radius;
                phi_arrays[oldStorage][index] = distance;
                phi_arrays[newStorage][index] = distance;

                index += 1;
            }
        }
    }
}

inline glm::vec3 Grid::getPosition(uint32_t x_i, uint32_t y_i, uint32_t z_i)
{
    return glm::vec3((float)x_i * CELL_WIDTH, (float)y_i * CELL_WIDTH, (float)z_i * CELL_WIDTH) + globalOffset;
}

void Grid::advect(float deltaT)
{
    const std::vector<float> &phi_old = phi_arrays[oldStorage];
    const std::vector<float> &u_minus_old = u_minus_arrays[oldStorage];
    const std::vector<float> &v_minus_old = v_minus_arrays[oldStorage];
    const std::vector<float> &w_minus_old = w_minus_arrays[oldStorage];

    std::vector<float> &phi_new = phi_arrays[newStorage];
    std::vector<float> &u_minus_new = u_minus_arrays[newStorage];
    std::vector<float> &v_minus_new = v_minus_arrays[newStorage];
    std::vector<float> &w_minus_new = w_minus_arrays[newStorage];

    std::array<const std::vector<float> *, 4> parameters_old = {
        &phi_old,
        &u_minus_old,
        &v_minus_old,
        &w_minus_old};

    std::array<std::vector<float> *, 4> parameters_new = {
        &phi_new,
        &u_minus_new,
        &v_minus_new,
        &w_minus_new};

    // advect phi and velocity for each non-solid cell (exclude i/j/k == 0 or N)
    for (uint32_t i = 1; i < Nx - 1; i++)
    {
        for (uint32_t j = 1; j < Ny - 1; j++)
        {
            for (uint32_t k = 1; k < Nz - 1; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;

                // get distance (# of cells) traversed using current velocities
                float x = 0.5f * deltaT * INV_CELL_WIDTH * (u_minus_old[base_index] + u_minus_old[base_index + NyNz]); // u_minus[i,j,k] + u_plus[i,j,k]
                float y = 0.5f * deltaT * INV_CELL_WIDTH * (v_minus_old[base_index] + v_minus_old[base_index + Nz]);   // v_minus[i,j,k] + v_plus[i,j,k]
                float z = 0.5f * deltaT * INV_CELL_WIDTH * (w_minus_old[base_index] + w_minus_old[base_index + 1]);    // w_minus[i,j,k] + w_plus[i,j,k]

                float i_new_f = std::clamp((float)i - x, 1.0f, (float)(Nx - 2));
                float j_new_f = std::clamp((float)j - y, 1.0f, (float)(Ny - 2));
                float k_new_f = std::clamp((float)k - z, 1.0f, (float)(Nz - 2));

                uint32_t i_new = static_cast<uint32_t>(i_new_f);
                uint32_t j_new = static_cast<uint32_t>(j_new_f);
                uint32_t k_new = static_cast<uint32_t>(k_new_f);

                uint32_t dest_index = i_new * NyNz + j_new * Nz + k_new;

                float i_beta = i_new_f - (float)i_new; // 5.2362 -> 0.2362
                float j_beta = j_new_f - (float)j_new;
                float k_beta = k_new_f - (float)k_new;

                float i_alpha = 1.0f - i_beta;
                float j_alpha = 1.0f - j_beta;
                float k_alpha = 1.0f - k_beta;

                // trilinear interpolation
                for (uint32_t param_idx = 0; param_idx < 4; param_idx++)
                {
                    const std::vector<float> &vals = *parameters_old[param_idx];
                    float param_i0 = (i_alpha * vals[dest_index]) + (i_beta * vals[dest_index + NyNz]);                   // i,j,k <-> i+1,j,k
                    float param_i1 = (i_alpha * vals[dest_index + Nz]) + (i_beta * vals[dest_index + NyNz + Nz]);         // i,j+1,k <-> i+1,j+1,k
                    float param_i2 = (i_alpha * vals[dest_index + 1]) + (i_beta * vals[dest_index + NyNz + 1]);           // i,j,k+1 <-> i+1,j,k+1
                    float param_i3 = (i_alpha * vals[dest_index + Nz + 1]) + (i_beta * vals[dest_index + NyNz + Nz + 1]); // i,j+1,k+1 <-> i+1,j+1,k+1

                    float param_ij0 = (j_alpha * param_i0) + (j_beta * param_i1);
                    float param_ij1 = (j_alpha * param_i2) + (j_beta * param_i3);

                    float interp_val = (k_alpha * param_ij0) + (k_beta * param_ij1);

                    (*parameters_new[param_idx])[base_index] = interp_val + (BODY_FORCES[param_idx] * deltaT);
                }
            }
        }
    }
}

void Grid::updateSOE(float deltaT)
{
    const std::vector<float> &phi = phi_arrays[newStorage];
    const std::vector<float> &u_minus = u_minus_arrays[newStorage];
    const std::vector<float> &v_minus = v_minus_arrays[newStorage];
    const std::vector<float> &w_minus = w_minus_arrays[newStorage];

    const float CONST_FACTOR = RHO * CELL_WIDTH / deltaT;

    for (uint32_t i = 1; i < Nx - 1; i++)
    {
        for (uint32_t j = 1; j < Ny - 1; j++)
        {
            for (uint32_t k = 1; k < Nz - 1; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;
                if (phi[base_index] >= 0.0f) // only care about fluid cells
                {
                    D[base_index] = 0.0f;
                    continue;
                }

                uint32_t nonSolidNeighbors = 0;
                float d = 0.0f;

                // left neighbor
                if (i != 1) // left neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t leftNeighbor = base_index - NyNz;
                    if (phi[leftNeighbor] < 0.0f) // i-1,j,k: liquid = non-zero velocity
                    {
                        d -= u_minus[base_index];
                        AplusI[leftNeighbor] = -1; // A(i,j,k)(i-1,j,k) = A(i-1,j,k)(i,j,k) so AplusI(i-1,j,k)
                    }
                    else
                    {
                        AplusI[leftNeighbor] = 0;
                    }
                }
                // right neighbor
                if (i != Nx - 2) // right neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t rightNeighbor = base_index + NyNz;
                    if (phi[rightNeighbor] < 0.0f) // i+1,j,k: liquid = non-zero velocity
                    {
                        d += u_minus[rightNeighbor];
                        AplusI[base_index] = -1; // A(i,j,k)(i+1,j,k) so AplusI(i,j,k)
                    }
                    else
                    {
                        AplusI[base_index] = 0;
                    }
                }
                // top neighbor
                if (j != 1) // top neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t topNeighbor = base_index - Nz;
                    if (phi[topNeighbor] < 0.0f) // i,j-1,k: liquid = non-zero velocity
                    {
                        d -= v_minus[base_index];
                        AplusJ[topNeighbor] = -1; // A(i,j,k)(i,j-1,k) = A(i,j-1,k)(i,j,k) so AplusJ(i,j-1,k)
                    }
                    else
                    {
                        AplusJ[topNeighbor] = 0;
                    }
                }
                // bottom neighbor
                if (j != Ny - 2) // bottom neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t bottomNeighbor = base_index + Nz;
                    if (phi[bottomNeighbor] < 0.0f) // i,j+1,k: liquid = non-zero velocity
                    {
                        d += v_minus[bottomNeighbor];
                        AplusJ[base_index] = -1; // A(i,j,k)(i,j+1,k) so AplusJ(i,j,k)
                    }
                    else
                    {
                        AplusJ[base_index] = 0;
                    }
                }
                // front neighbor
                if (k != 1) // front neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t frontNeighbor = base_index - 1;
                    if (phi[frontNeighbor] < 0.0f) // i,j,k-1: liquid = non-zero velocity
                    {
                        d -= w_minus[base_index];
                        AplusK[frontNeighbor] = -1; // A(i,j,k)(i,j,k-1) = A(i,j,k-1)(i,j,k) so AplusK(i,j,k-1)
                    }
                    else
                    {
                        AplusK[frontNeighbor] = 0;
                    }
                }
                // back neighbor
                if (k != Nz - 2) // back neighbor is not SOLID
                {
                    nonSolidNeighbors++;
                    uint32_t backNeighbor = base_index + 1;
                    if (phi[backNeighbor] < 0.0f) // i,j,k+1: liquid = non-zero velocity
                    {
                        d += w_minus[backNeighbor];
                        AplusK[base_index] = -1; // A(i,j,k)(i,j,k+1) so AplusK(i,j,k)
                    }
                    else
                    {
                        AplusK[base_index] = 0;
                    }
                }

                Adiag[base_index] = nonSolidNeighbors;
                D[base_index] = -CONST_FACTOR * d;
            }
        }
    }
}

void Grid::solveSOE()
{
    // Conjugate Gradient Algorithm
    uint32_t iterations = 0;
    float r_dot_r = 0.0f;

    std::vector<float> tmp0 = std::vector<float>(Nx * Ny * Nz);

    mulA(pressures, tmp0);
    sumC(D, tmp0, -1.0f, residuals); // r = D - A*pressure

    r_dot_r = dot(residuals, residuals); // r_dot_r = r*r
    conjugates = residuals;              // p = r
    std::cout << "(" << iterations << ") R^2 = " << r_dot_r << std::endl;

    while (iterations < MAX_ITERATIONS)
    {
        mulA(conjugates, tmp0); // tmp0 = A*p

        float alpha = r_dot_r / dot(conjugates, tmp0); // alpha = r*r / (p*A*p)

        sumC(pressures, conjugates, alpha, pressures); // pressure += alpha*p
        sumC(residuals, tmp0, -alpha, residuals);      // r -= alpha*Ap

        float new_r_dot_r = dot(residuals, residuals);
        float beta = new_r_dot_r / r_dot_r;
        r_dot_r = new_r_dot_r;

        sumC(residuals, conjugates, beta, conjugates);

        iterations++;
        std::cout << "(" << iterations << ") R^2/cell = " << r_dot_r / (Nx * NyNz) << std::endl;
        if (r_dot_r / (Nx * NyNz) < 1e-6)
        {
            break;
        }
    }

    float max_p = 0.0f;
    for (uint32_t idx = 0; idx < Nx * NyNz; idx++)
    {
        if (pressures[idx] > max_p)
        {
            max_p = pressures[idx];
        }
    }
    std::cout << "Max pressure: " << max_p << std::endl;
}

void Grid::mulA(const std::vector<float> &x, std::vector<float> &result)
{
    const std::vector<float> &phi = phi_arrays[newStorage];

    float sum = 0.0f;
    for (uint32_t i = 0; i < Nx; i++)
    {
        for (uint32_t j = 0; j < Ny; j++)
        {
            for (uint32_t k = 0; k < Nz; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;
                if (phi[base_index] > 0.0f) // row in A matrix is all zeros if non-fluid
                {
                    result[base_index] = 0.0f;
                    continue;
                }
                float val = 0.0f;
                val += Adiag[base_index] * x[base_index];
                val += AplusI[base_index] * x[base_index + NyNz];
                val += AplusJ[base_index] * x[base_index + Nz];
                val += AplusK[base_index] * x[base_index + 1];
                val += AplusI[base_index - NyNz] * x[base_index - NyNz];
                val += AplusJ[base_index - Nz] * x[base_index - Nz];
                val += AplusK[base_index - 1] * x[base_index - 1];
                result[base_index] = val;
                sum += val;
            }
        }
    }
    // std::cout << "DEBUG MUL: " << sum << std::endl;
}

void Grid::sumC(const std::vector<float> &a, const std::vector<float> &b, float C, std::vector<float> &result)
{
    float sum = 0.0f;
    for (uint32_t i = 0; i < Nx; i++)
    {
        for (uint32_t j = 0; j < Ny; j++)
        {
            for (uint32_t k = 0; k < Nz; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;
                result[base_index] = a[base_index] + b[base_index] * C;
                sum += result[base_index];
                // if (std::isnan(result[base_index]))
                // {
                //     std::cout << "{" << i << ", " << j << ", " << k << "}: " << a[base_index] << std::endl;
                // }
            }
        }
    }
    // std::cout << "DEBUG SUMC: " << sum << std::endl;
}

float Grid::dot(const std::vector<float> &a, const std::vector<float> &b)
{
    float tmpResult = 0.0f;
    for (uint32_t i = 0; i < Nx; i++)
    {
        for (uint32_t j = 0; j < Ny; j++)
        {
            for (uint32_t k = 0; k < Nz; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;
                tmpResult += a[base_index] * b[base_index];
            }
        }
    }
    return tmpResult;
}

void Grid::project(float deltaT)
{
    std::vector<float> &phi = phi_arrays[newStorage];
    std::vector<float> &u_minus_new = u_minus_arrays[newStorage];
    std::vector<float> &v_minus_new = v_minus_arrays[newStorage];
    std::vector<float> &w_minus_new = w_minus_arrays[newStorage];

    float CONST_FACTOR = deltaT / (RHO * CELL_WIDTH);
    for (uint32_t i = 1; i < Nx; i++)
    {
        for (uint32_t j = 1; j < Ny; j++)
        {
            for (uint32_t k = 1; k < Nz; k++)
            {
                uint32_t base_index = i * NyNz + j * Nz + k;

                // if (phi[base_index] > 0.0f)
                // {
                //     u_minus_new[base_index] = 0.0f;
                //     v_minus_new[base_index] = 0.0f;
                //     w_minus_new[base_index] = 0.0f;
                // }
                // else
                // {
                u_minus_new[base_index] -= CONST_FACTOR * (pressures[base_index] - pressures[base_index - NyNz]);
                v_minus_new[base_index] -= CONST_FACTOR * (pressures[base_index] - pressures[base_index - Nz]);
                w_minus_new[base_index] -= CONST_FACTOR * (pressures[base_index] - pressures[base_index - 1]);
                // }
            }
        }
    }
}

void Grid::constructSurface(std::vector<Vertex> &vertices, std::vector<uint32_t> &indices)
{
    const std::vector<float> &phi = phi_arrays[newStorage];

    vertices.resize(0);
    indices.resize(0);

    std::array<float, 8> localPhis;
    std::array<bool, 8> isWater{};

    // loop through entire grid and draw marching cubes
    for (uint32_t i = 0; i < Nx - 1; i++)
    {
        for (uint32_t j = 0; j < Ny - 1; j++)
        {
            for (uint32_t k = 0; k < Nz - 1; k++)
            {
                uint8_t vertexMask = 0;

                uint32_t baseIndex = i * NyNz + j * Nz + k;
                localPhis[0] = phi[baseIndex];                 // i, j, k
                localPhis[1] = phi[baseIndex + NyNz];          // i+1, j, k
                localPhis[2] = phi[baseIndex + Nz];            // i, j+1, k
                localPhis[3] = phi[baseIndex + NyNz + Nz];     // i+1, j+1, k
                localPhis[4] = phi[baseIndex + 1];             // i, j, k+1
                localPhis[5] = phi[baseIndex + NyNz + 1];      // i+1, j, k+1
                localPhis[6] = phi[baseIndex + Nz + 1];        // i, j+1, k+1
                localPhis[7] = phi[baseIndex + NyNz + Nz + 1]; // i+1, j+1, k+1

                // identify polarity of air-water boundary, 8-bit pattern
                for (uint8_t byte = 0; byte < 8; byte++)
                {
                    isWater[byte] = localPhis[byte] < 0.0f;
                    vertexMask |= isWater[byte] << byte;
                }

                MarchingCube marchingCube = marchingCubeLookup[vertexMask];

                if (marchingCube.size() == 0)
                {
                    continue;
                }

                glm::vec3 position = getPosition(i, j, k);

                // std::cout << position.x << ", " << position.y << ", " << position.z << std::endl;

                // pre-compute each edge's boundary
                std::array<glm::vec3, 12> boundaryVertices{};
                boundaryVertices[0] = isWater[0] != isWater[1] ? glm::vec3((-CELL_WIDTH * localPhis[0]) / (localPhis[1] - localPhis[0]), 0.0f, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);              // v0 -> v1
                boundaryVertices[1] = isWater[0] != isWater[2] ? glm::vec3(0.0f, (-CELL_WIDTH * localPhis[0]) / (localPhis[2] - localPhis[0]), 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);              // v0 -> v2
                boundaryVertices[2] = isWater[1] != isWater[3] ? glm::vec3(CELL_WIDTH, (-CELL_WIDTH * localPhis[1]) / (localPhis[3] - localPhis[1]), 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);        // v1 -> v3
                boundaryVertices[3] = isWater[2] != isWater[3] ? glm::vec3((-CELL_WIDTH * localPhis[2]) / (localPhis[3] - localPhis[2]), CELL_WIDTH, 0.0f) : glm::vec3(0.0f, 0.0f, 0.0f);        // v2 -> v3
                boundaryVertices[4] = isWater[0] != isWater[4] ? glm::vec3(0.0f, 0.0f, (-CELL_WIDTH * localPhis[0]) / (localPhis[4] - localPhis[0])) : glm::vec3(0.0f, 0.0f, 0.0f);              // v0 -> v4
                boundaryVertices[5] = isWater[1] != isWater[5] ? glm::vec3(CELL_WIDTH, 0.0f, (-CELL_WIDTH * localPhis[1]) / (localPhis[5] - localPhis[1])) : glm::vec3(0.0f, 0.0f, 0.0f);        // v1 -> v5
                boundaryVertices[6] = isWater[2] != isWater[6] ? glm::vec3(0.0f, CELL_WIDTH, (-CELL_WIDTH * localPhis[2]) / (localPhis[6] - localPhis[2])) : glm::vec3(0.0f, 0.0f, 0.0f);        // v2 -> v6
                boundaryVertices[7] = isWater[3] != isWater[7] ? glm::vec3(CELL_WIDTH, CELL_WIDTH, (-CELL_WIDTH * localPhis[3]) / (localPhis[7] - localPhis[3])) : glm::vec3(0.0f, 0.0f, 0.0f);  // v3 -> v7
                boundaryVertices[8] = isWater[4] != isWater[5] ? glm::vec3((-CELL_WIDTH * localPhis[4]) / (localPhis[5] - localPhis[4]), 0.0f, CELL_WIDTH) : glm::vec3(0.0f, 0.0f, 0.0f);        // v4 -> v5
                boundaryVertices[9] = isWater[4] != isWater[6] ? glm::vec3(0.0f, (-CELL_WIDTH * localPhis[4]) / (localPhis[6] - localPhis[4]), CELL_WIDTH) : glm::vec3(0.0f, 0.0f, 0.0f);        // v4 -> v6
                boundaryVertices[10] = isWater[5] != isWater[7] ? glm::vec3(CELL_WIDTH, (-CELL_WIDTH * localPhis[5]) / (localPhis[7] - localPhis[5]), CELL_WIDTH) : glm::vec3(0.0f, 0.0f, 0.0f); // v5 -> v7
                boundaryVertices[11] = isWater[6] != isWater[7] ? glm::vec3((-CELL_WIDTH * localPhis[6]) / (localPhis[7] - localPhis[6]), CELL_WIDTH, CELL_WIDTH) : glm::vec3(0.0f, 0.0f, 0.0f); // v6 -> v7

                for (uint32_t i = 0; i < marchingCube.size(); i++)
                {
                    Triple triangle = marchingCube[i]; // { 0, 1, 4}
                    uint32_t startIndex = vertices.size();
                    uint32_t indicesSize = indices.size();
                    vertices.resize(startIndex + 3);
                    indices.resize(indicesSize + 3);
                    glm::vec3 normal = glm::normalize(glm::cross(boundaryVertices[triangle[1]] - boundaryVertices[triangle[0]], boundaryVertices[triangle[2]] - boundaryVertices[triangle[0]]));
                    for (uint32_t j = 0; j < 3; j++)
                    {
                        vertices[startIndex + j] = {boundaryVertices[triangle[j]] + position, normal, SURFACE_COLOR};
                        indices[indicesSize + j] = startIndex + j;
                    }
                }
            }
        }
    }
    std::cout << "Number of triangles: " << indices.size() / 3 << std::endl;
}

void Grid::flipStorage()
{
    newStorage = 1 - newStorage;
    oldStorage = 1 - oldStorage;
}
