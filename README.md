# Cannon Ball

### How to install

First install raylib: 

```brew install raylib```

To compile and run:

```g++ main.cpp PhysicsBody.cpp Projectile.cpp Cannon.cpp -o sim -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit && ./sim```

### How to play:

Use your computer arrow keys (UP, DOWN, LEFT, RIGHT) to change the aim direction of the cannon.

Hold the space bar to begin firing the cannon. The longer you hold the space bar, the more powerful the shot will be. Release the space bar to fire. 

Aim for the targets on the screen. The more centered on the target you hit, the more points you get.

Make sure to consider wind when aiming your shot. 

### Inheritance structure of classes

The root class is ```Entity``` which has a position.
It also has a ```virtual``` destructor method.
It has a ```virtual``` update method which takes in a framerate. 
It has a ```virtual``` draw method which renders the entity to the screen. 

Class ```PhysicsBody``` inherits from entity and implements the update class with all of the relevant physics math. Entites that move with gravity / wind resistance need to inherit from ```PhysicsBody```. This includes bounce mechanics and randomly generated wind. ```PhysicsBody``` defines the gravitational force, wind, and bounce mechanics of a given set of rounds. Wind changes every three round. 

The ```Projectile``` class implements the draw method of entity. It represents the ball being launched, and uses raylib's ```DrawSphere()``` API method. It takes a position, radius and color. 

Then there is the ```Cannon``` class which does NOT inherit from the ```Entity``` class as it is not subject to gravity or wind for it's movement. Only user input can move the cannon, changing its aim direction. The ```Cannon.Fire()``` method takes a ```Projectile``` object by reference, and then affects it's physics metrics. 

main.cpp contains ```DrawWorld()``` which renders the the game environment appearance. It also contains ```DrawWindHUD()``` which will render the wind compass in the top right corner of the screen for the user to consider when shooting a ball. 


