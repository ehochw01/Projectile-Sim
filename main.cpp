#include "raylib.h" //imports raylib library
#include "Projectile.h"



//barebones space design, tomorrow morning I'll code up the physics engine classes. 

int main() {

    //these lines setup our window, most of these are raylib functions
    const int screen_width = 1280;   //screen dimensions, const since they shouldn't change
    const int screen_height = 720; 
    SetConfigFlags(FLAG_WINDOW_TOPMOST);  //bug fix, this forces window to the front when it launches
    InitWindow(screen_width,screen_height, "Projectile Simulator");
    SetTargetFPS(60); //sets the upper limit of the loop at 60 fps

    // sets up the camera, behind the cannon looking fwd
    //the Camera3D class already exists in raylib, with useful methods
    Camera3D camera = {};
    camera.position = {-10.0f, 5.0f, 0.0f}; //behind the camera (neg x), and above, (pos y), classic video game)
                                            // the reason for the f's is that raylib using a Vector3 struct that takes in float numbers, not doubles. 
    camera.target = {0.0f, 2.0f, 0.0f};  //what its looking at, slightly upward, this func draws line between position and target, to determine our line of sight
    camera.up = {0.0f, 1.0f, 0.0f};     //determines which way is up. 
    camera.fovy = 60.0f;   //fovy means field of view, in degrees
    camera.projection = CAMERA_PERSPECTIVE; //makes distant thibngs shrink, parallel lines ocnverge toward horizon, same as our eyes. other views available.

    //before our while loop, lets setup up a ball 

    Projectile ball;
    ball.position = {0.0f, 1.0f, 0.0f};   //begin just above the ground
    ball.velocity = {15.0f, 15.0f, 0.0f}; //initial velocity vector is 45 degrees



    //this is our mainloop, 

    while (!WindowShouldClose()) {    //will be true until we hit escape key, only way to end the sim!
        float dt = GetFrameTime(); //seconds since last frame, in our case 1/60 secs, then uses this to feed the physics engine

        ball.Update(dt);  //happens outside drawing


        BeginDrawing();
            ClearBackground(RAYWHITE);  //basic white background
            
            BeginMode3D(camera);           //enter 3D space from view of camera defined above 
                DrawGrid(1000,1.0f);         //1000x1000 grid, 1 meter cells, appears to be infinite plane
                ball.Draw(); //draw the ball at its current location
                DrawSphere({0,0,0}, 0.3f, RED);  //small sphere to mark the center of the grid.
            EndMode3D(); //no longer drawing in the 3d world after this, but on the flat 2d screen

            DrawText("Projectile Sim, Basic Setup for the Boys", 10,10,20,DARKGRAY);  //text, 10, 10 = x y position from left and top edge
                                                                                     // 20 = font size 
                                                                                     // DARKGRAY = color
                                                                                     // Eric this is where the "Target Hit!" text would be.

        EndDrawing();  //pushes frame to screen 
    } //closes the while loop
CloseWindow();
return 0;
}



