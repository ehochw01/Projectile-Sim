#include "Target.h"
#include "raylib.h"

void Target::Draw() {
    // flat vertical disk facing the cannon: the axis runs along the downrange X axis,
    // and the disk is thin (0.2m) so the ball passes through it in a single frame.
    Vector3 faceFront = { position.x - 0.1f, position.y, position.z };
    Vector3 faceBack  = { position.x + 0.1f, position.y, position.z };
    DrawCylinderEx(faceFront, faceBack, radius, radius, 24, color); //24 sides = smooth disk
    DrawCylinderWiresEx(faceFront, faceBack, radius, radius, 24, BLACK);
};

void Target::Update(float dt) {
    // Targets are static, so no update logic is needed for now.
};

void Target::Shrink() {
    radius -= 1.0f; // shrink the target by 0.5 meters
    if (radius < 1.0f) {
        radius = 1.0f; // enforce a minimum radius of 1 meter
    }
}

void Target::RandomizeColor() {
    // standard raylib colors, picked so the target stays visible against the
    // grass and sky: no GREEN/LIME/DARKGREEN, no SKYBLUE/BLUE
    static const Color kPalette[] = {
        RED, ORANGE, YELLOW, GOLD, PINK, MAGENTA, PURPLE, VIOLET, MAROON, BROWN, BEIGE, DARKBLUE
    };
    constexpr int kPaletteSize = sizeof(kPalette) / sizeof(kPalette[0]);
    color = kPalette[GetRandomValue(0, kPaletteSize - 1)];
}

bool Target::CheckHit(Vector3 prevBallPos, Vector3 ballPos, float ballRadius) const {
    // did the ball cross the disk's plane (x = position.x) this frame, moving downrange?
    // checking the sign change between last frame and this one means we only fire once.
    bool crossedPlane = (prevBallPos.x <= position.x && ballPos.x >= position.x);
    if (!crossedPlane) return false;

    // was the crossing point inside the disk face? measure radial distance in the Y-Z plane.
    float dy = ballPos.y - position.y;
    float dz = ballPos.z - position.z;
    float reach = radius + ballRadius;
    return (dy * dy + dz * dz) <= reach * reach;
};