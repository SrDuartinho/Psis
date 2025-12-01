#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "universe-data.h"

Vector make_vector(float x, float y) {
    Vector v;
    v.amplitude = sqrt(x * x + y * y);
    v.angle = atan2(y, x);
    return v;
}

Vector add_vectors(Vector v1, Vector v2) {
    // Convert polar to cartesian
    float x1 = v1.amplitude * cos(v1.angle);
    float y1 = v1.amplitude * sin(v1.angle);
    float x2 = v2.amplitude * cos(v2.angle);
    float y2 = v2.amplitude * sin(v2.angle);

    // Add cartesian components
    float sum_x = x1 + x2;
    float sum_y = y1 + y2;

    // Convert back to polar and return
    return make_vector(sum_x, sum_y);
}