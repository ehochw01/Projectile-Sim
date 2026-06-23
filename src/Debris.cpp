#include "Debris.h"

void Debris::Draw() {
    DrawSphere(position, radius, color);
    DrawSphereWires(position, radius, 6, 6, BLACK);   // low rings/slices = visible facets

}