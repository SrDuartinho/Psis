#ifndef CURSOR_PROCESSING_H
#define CURSOR_PROCESSING_H

#include "communication.h"
#include "universe-data.h"

// Applies a thrust impulse based on a keyboard direction from the client.
void new_position(Ship* ship, direction_t direction);

#endif
