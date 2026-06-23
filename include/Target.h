#pragma once

#include "Entity.h"

class Target : public Entity {
    public:
        void Draw() override;
        void Update(float dt) override;
        // returns true only on the single frame the ball crosses the disk's face,
        // so a ball passing through can't register as many collisions in a row.
        bool CheckHit(Vector3 prevBallPos, Vector3 ballPos, float ballRadius) const;
        void RandomizeColor();   // picks a new random color for the target
        void Shrink();
        float radius{10.0f};    //disk radius in meters, can change
    private:
        Color color{RED};     //red targets
};

