#include "Cannon.h"
#include <cmath>   // cosf, sinf

Vector3 Cannon::AimDirection() const {
    float az = azimuth   * DEG2RAD;
    float el = elevation * DEG2RAD;

    return {
        cosf(el) * cosf(az),   // X: downrange
        sinf(el),              // Y: up
        cosf(el) * sinf(az)    // Z: sideways
    };
}

void Cannon::Draw() const {
    // base mount at the origin
    DrawCube({0.0f, 0.5f, 0.0f}, 1.6f, 1.0f, 1.6f, (Color){ 60, 60, 65, 255 }); //dark color cannon

    // barrel: from pivot, out along the aim direction
    Vector3 dir = AimDirection();
    float barrelLen = 3.0f;
    Vector3 barrelEnd = {
        pivot.x + dir.x * barrelLen,
        pivot.y + dir.y * barrelLen,
        pivot.z + dir.z * barrelLen
    };
    DrawCylinderEx(pivot, barrelEnd, 0.35f, 0.28f, 12, (Color){ 40, 40, 45, 255 }); //use drawcylinder from raylib, connect the 2 points
}

void Cannon::Fire(Projectile& ball) const {
    Vector3 dir = AimDirection();

    ball.position = pivot;
    ball.velocity = { dir.x * power, dir.y * power, dir.z * power };   // aim direction * power
}