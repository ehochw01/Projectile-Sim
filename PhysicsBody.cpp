#include "PhysicsBody.h"

void PhysicsBody::Update(float dt) {
    const float gravity = -9.81f; //m/s2, pulling down along -y. can change to see how projectiles will move on other planets

    velocity.y += gravity * dt;     //gravity accelerates velocity downwards, changing it every dt
    position.y += velocity.y * dt;  //velocity moves the position, changing it every dt

    position.x += velocity.x * dt; // both x and z drift at constant speed determined at launch, since no acceleration force is on x or z, only on y. 
                                   // this will change if we decide to add drag later, and will depend on the radius of the ball.
    position.y += velocity.z * dt; 

    //the loop below is what prevents it from going through the floor, so it bounces
    if (position.y < 0.0f) {
        position.y = 0.0f;
        velocity.y = -velocity.y * 0.7f; //reverse vertical velocity, keep 70%, lose 30% to the bounce
    }
}


