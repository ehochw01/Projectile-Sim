#pragma once
#include "raylib.h"
#include "Projectile.h"

// Hey Sid, read this first, this is where a lot of your work will be, the aim seam, connecting the cannon to instructions
// Right now it just loads the cannon pointing straight (azimuth 0) and at a 45 degree angle
// You will code stuff that writes to azimuth / elevation / power from the users input.
// The barrel and the launch both read these — nothing else to wire up.
// To fire: call cannon.Fire(ball) on spacebar release.
//youll have to do the key input stuff in the while loop in main, so it updates the position of the cannon every dt based on keyboard inputs

class Cannon {
public:
    float azimuth   = 0.0f;   // Sid: left/right, degrees, range -90 to 90
    float elevation = 30.0f;  // Sid: up/down, degrees, range 0 to 85
    float power     = 20.0f;  // Sid: launch speed m/s, range 0 to 100 (spacebar hold)

    Vector3 pivot   = {0.0f, 1.0f, 0.0f};   // barrel hinge + launch point

    Vector3 AimDirection() const;       // (azimuth, elevation) -> unit direction vector
    void Draw() const;                  // draws base + barrel along current aim
    void Fire(Projectile& ball) const;  // launches ball along aim at power
};