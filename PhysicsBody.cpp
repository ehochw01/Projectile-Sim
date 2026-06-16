#include "PhysicsBody.h"

void PhysicsBody::Update(float dt) {
    const float gravity = -9.81f; //m/s2, pulling down along -y. can change to see how projectiles will move on other planets

    velocity.y += gravity * dt;     //gravity accelerates velocity downwards
    position.y += velocity.y * dt;  //velocity moves the position 

    position.x += velocity.x * dt; // both x and z drift at constant speed, since no acceleration force is on x or z, only on y. 
                                   // this will change if we decide to add drag later, and will depend on the radius of the ball.
    position.y += velocity.z * dt; 
}


