#pragma once

#include "Entity.h"

class Target : public Entity {
    public:
        void Draw() override;
        void Update(float dt) override;
        // checked only on the single frame the ball crosses the disk's face,
        // so a ball passing through can't register as many hits in a row.
        // returns 0 for no hit, 1 for the outer ring, ... up to 5 for a dead-center bullseye.
        int CheckHit(Vector3 prevBallPos, Vector3 ballPos, float ballRadius) const;
        void ChangeColor();   // picks a new random color for the target
        Color GetColor() const { return color; }
        void Shrink();
        float radius{10.0f};    //disk radius in meters, can change
    private:
        static constexpr Color kPalette[] = {
            RED, ORANGE, PINK, GOLD,MAGENTA, PURPLE, VIOLET, MAROON, BROWN, BEIGE, DARKBLUE, 
        };
        int ColorIndex{0};   // index into the palette for the current color
        Color color{kPalette[ColorIndex]};     //red targets
};

