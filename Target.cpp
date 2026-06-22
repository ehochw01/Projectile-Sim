#include "Target.h"

void Target::Draw() {
    DrawSphere(position,radius,color); //draws sphere at current position, all variables previously defined.
                                       //position updates based on the integration stuff going on in physicsbody
                                       //ready for a test fire 
};

void Target::Update(float dt) {
    // Targets are static, so no update logic is needed for now.
};