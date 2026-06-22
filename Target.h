#pragma once

#include "Entity.h"

class Target : public Entity {
    public: 
        void Draw() override;
        void Update(float dt) override;
    private:
        float radius{10.0f};    //sphere radius in meters, can change
        Color color{RED};     //red targets
};

