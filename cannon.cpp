#include "Cannon.h"
#include "raylib.h"
#include <cmath>   // cosf, sinf
#include "Constants.h"

using namespace Constants;

Vector3 Cannon::getPivot() const {
    return pivot;
}

float Cannon::getLaunchSpeed() const {
    return _launchSpeed;
}

// left/right, degrees, range -90 to 90
void Cannon::incrAzimuth(float frameTime) {
    float temp = _azimuth;
    temp += 60.0f *frameTime;
    if (temp >= AMIN && temp <= AMAX) {
        _azimuth = temp;
    }
}

// left/right, degrees, range -90 to 90
void Cannon::decrAzimuth(float frameTime) {
    float temp = _azimuth;
    temp -= 60.0f *frameTime;
    if (temp >= AMIN && temp <= AMAX) {
        _azimuth = temp;
    }
}

// up/down, degrees, range 0 to 85
void Cannon::incrElevation(float frameTime) {
    float temp = _elevation;
    temp += 60.0f *frameTime;
    if (temp >= EMIN && temp <= EMAX) {
        _elevation = temp;
    }
}

void Cannon::decrElevation(float frameTime) {
    float temp = _elevation;
    temp -= 60.0f *frameTime;
    if (temp >= EMIN && temp <= EMAX) {
        _elevation = temp;
    }
}

void Cannon::incrLaunchSpeed(float frameTime) {
    float temp = _launchSpeed;
    temp += 60.0f *frameTime;
    if (temp >= PMIN && temp <= PMAX) {
        _launchSpeed = temp;
    }
}

Vector3 Cannon::AimDirection() const {
    float az = _azimuth   * DEG2RAD;
    float el = _elevation * DEG2RAD;

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

void Cannon::Fire(Projectile& ball) {

    Vector3 dir = AimDirection();
    ball.position = pivot;
    // aim direction * _launchSpeed 
    ball.velocity = { dir.x * _launchSpeed, dir.y * _launchSpeed, dir.z * _launchSpeed };
    
    _launchSpeed = 0.0f;
}

