#include <math.h>
#include "communication.h"
#include "universe-data.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Apply a thrust impulse to the ship's velocity based on direction.
// Position is not changed here; physics loop updates it.
void new_position(Ship* ship, direction_t direction) {
    float angle;
    switch (direction) {
        case UP:    angle = -M_PI / 2.0f; break;   // negative Y
        case DOWN:  angle =  M_PI / 2.0f; break;   // positive Y
        case LEFT:  angle =  M_PI;         break;  // negative X
        case RIGHT: angle =  0.0f;         break;  // positive X
        default:    return; // ignore unknown inputs
    }

    const float thrust = 0.1f; // magnitude of thrust impulse per key press
    float vx = thrust * cosf(angle);
    float vy = thrust * sinf(angle);

    Vector thrust_vec = make_vector(vx, vy);
    ship->velocity = add_vectors(ship->velocity, thrust_vec);
}
