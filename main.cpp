#include "raylib.h" //imports raylib library
#include "Projectile.h"
#include <cmath>
#include "Cannon.h"
#include "Target.h"
#include "Constants.h"

//run code, shoot with the R key for now. Will be upon spacebar release when sid adds his part. 

//Hey guys, before main, I've written two helper functions:
//HELPER FUNCTION 1: FIREBALL: nevermind i deleted this one, a fire function is now in the cannon class.
//HELPER FUNCTION 2: DRAWWINDHUD: Just draws the wind direction in the top right corner for the user to see before they launch. in main youll see that wind changes every 3 shots. 
//HELPER FUNCTION 3: DRAWWORLD: This is where I drew the whole scene, the sky, the ground, etc. 

//helper function to draw the wind arrow for the user. Wind changes every 3 shots. 
void DrawWindHUD(Vector3 wind, int screenWidth) {           //wind will be drawn relative to screen width so it doesnt break if we change the window size for different computers
    Vector2 center = { (float)screenWidth - 80.0f, 90.0f };   // top-right corner
    float radius = 38.0f;                                      // arrow length

    float speed = sqrtf(wind.x * wind.x + wind.z * wind.z);   // wind magnitude (no y down or up wind)

    //relative to the camera here is how the world looks:
    // player looks down +X. On their screen:
    //   world +X (downrange, away) -> screen UP
    //   world -Z (viewer's right)  -> screen RIGHT
    float dirX = wind.z;   // screen x-component for wind
    float dirY = wind.x;   // screen y-component for wind

    DrawCircleLines((int)center.x, (int)center.y, radius + 10, GRAY);   // dial ring, DrawCircleLines built into Raylib is great!
    DrawText("WIND", (int)center.x - 20, (int)center.y - radius - 34, 16, BLACK);

    if (speed > 0.01f) {                                       // only draw arrow if there's wind, could randomly generate zero wind!
        float len = sqrtf(dirX * dirX + dirY * dirY);
        dirX /= len;  dirY /= len;                             // normalize to unit direction

        Vector2 tip  = { center.x + dirX * radius, center.y + dirY * radius };  //get the two points drawn up then join em
        Vector2 tail = { center.x - dirX * radius, center.y - dirY * radius };
        DrawLineEx(tail, tip, 4.0f, RED);                      // shaft

        // arrowhead: a triangle at the tip, super annoying to draw
        float perpX = -dirY, perpY = dirX;                     // perpendicular to direction,def variables 
        Vector2 base = { center.x + dirX * (radius - 12), center.y + dirY * (radius - 12) };
        Vector2 left  = { base.x + perpX * 9, base.y + perpY * 9 };
        Vector2 right = { base.x - perpX * 9, base.y - perpY * 9 };
        DrawTriangle(tip, right, left, RED);
    }

    DrawText(TextFormat("%.1f m/s", speed), (int)center.x - 30, (int)center.y + radius + 14, 18, BLACK);
}
//Helper function, drawworld, self explanatory, it draws the world! 
void DrawWorld() {
    // ground plane: a big flat green field at y=0, like grass! 
    DrawPlane({0.0f, 0.0f, 0.0f}, {1000.0f, 1000.0f}, (Color){ 76, 124, 60, 255 });

    // subtle grid on top of the grass, like a marked field of some sort
    DrawGrid(200, 5.0f);   // reads as field markings

    // range markers: low posts every 25m downrange (+X), in pairs 
    for (int dist = 25; dist <= 200; dist += 25) {
        float x = (float)dist;
        // a pair of posts, one on each side of the firing lane,
        DrawCube({ x, 1.0f,  12.0f }, 0.4f, 2.0f, 0.4f, (Color){ 200, 200, 200, 255 });
        DrawCube({ x, 1.0f, -12.0f }, 0.4f, 2.0f, 0.4f, (Color){ 200, 200, 200, 255 });
    }
}

// another helper function to draw the power charge bar. Bottom-center, fills red as power climbs 0...100
void DrawPowerBar(float power, int screenWidth, int screenHeight) {
    int barWidth  = 300;
    int barHeight = 24;
    int x = screenWidth / 2 - barWidth / 2;     // centered horizontally
    int y = screenHeight - 60;                   // near the bottom

    float fraction = power / 100.0f;             // 0.0 to 1.0
    int fillWidth = (int)(barWidth * fraction);  // how much of the bar is filled (how long the second red rectangle is)

    DrawRectangle(x, y, barWidth, barHeight, (Color){ 40, 40, 40, 200 });   // dark background
    DrawRectangle(x, y, fillWidth, barHeight, RED);                          // red fill
    DrawRectangleLines(x, y, barWidth, barHeight, BLACK);                    // border
    DrawText("POWER", x, y - 22, 18, BLACK);                                 // label
}



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
    camera.position = {-10.0f, 2.5f, 0.0f}; //behind the camera (neg x), and above, (pos y), classic video game)
                                            // the reason for the f's is that raylib using a Vector3 struct that takes in float numbers, not doubles. 
    camera.target = {0.0f, 3.0f, 0.0f};  //what its looking at, slightly upward, this func draws line between position and target, to determine our line of sight
    camera.up = {0.0f, 1.0f, 0.0f};     //determines which way is up. 
    camera.fovy = 60.0f;   //fovy means field of view, in degrees
    camera.projection = CAMERA_PERSPECTIVE; //makes distant thibngs shrink, parallel lines ocnverge toward horizon, same as our eyes. other views available.

    //before our while loop, lets setup up a ball, angle and initial velocity. 

    Cannon cannon;   //owns aim + power, Sid's input writes to this. fires the ball along the barrel
    Projectile ball;
    ball.position = cannon.getPivot(); //cannon ball is in the barrel!
    ball.active = false; //dormant ball, not simulated until fired

    Target target; //target is a sphere, will be drawn at a random location in the world.
    target.position = { 50.0f, 15.0f, 0.0f }; //target is at 50m downrange, on the ground, radius is 0.5m so y=0.5 to sit on the ground

   
    ball.GenerateWind(); //generate wind for the first 6 shots
    int shotsSinceWind = 0; //counts shots fired under current wind. Wind changes every 6 shots.
 //this is our mainloop, 
    while (!WindowShouldClose()) {    //will be true until we hit escape key, only way to end the sim!
        float fTime = GetFrameTime(); //seconds since last frame, in our case 1/60 secs, then uses this to feed the physics engine

        // TEMP dev-only aim test — REMOVE before merge (Sid owns the arrows)
        if (IsKeyDown(KEY_LEFT))  cannon.decrAzimuth(fTime);
        if (IsKeyDown(KEY_RIGHT)) cannon.incrAzimuth(fTime);
        if (IsKeyDown(KEY_UP))    cannon.incrElevation(fTime);
        if (IsKeyDown(KEY_DOWN))  cannon.decrElevation(fTime);

        // charge while holding space
        if (IsKeyDown(KEY_SPACE)) {
            cannon.incrLaunchSpeed(fTime);
        }

        // fire on release
        if (IsKeyReleased(KEY_SPACE)) {
            cannon.Fire(ball);
            ball.active = true;

            shotsSinceWind++;
            if (shotsSinceWind >= 3) { ball.GenerateWind(); shotsSinceWind = 0; }

        }

        if (ball.active) ball.Update(fTime);   // only simulate a ball in flight


        BeginDrawing();
            ClearBackground((Color){ 135, 206, 235, 255 });  //sky blue in RBG values
            
            BeginMode3D(camera);           //enter 3D space from view of camera defined above 
                DrawWorld();       //uses helper function above to draw the world as defined.

                cannon.Draw();   //draw the cannon at the origin, barrel points along the current aim

                ball.Draw(); //draw the ball at its current location, everytime this is hit in each loop, it uses the new updated ball position as derived by the math in the physicsbody source code.
                target.Draw(); //draw the target at its location, currently static but we could make it move later if we wanted.
                DrawSphere({0,0,0}, 0.3f, RED);  //small sphere to mark the center of the grid.
            EndMode3D(); //no longer drawing in the 3d world after this, but on the flat 2d screen

            

            DrawText("Projectile Sim, Posts every 25 meters", 10,10,20,DARKGRAY);  //text, 10, 10 = x y position from left and top edge
                                                                                      // // DARKGRAY = color
                                                                                     // Eric this is where the "Target Hit!" text would be.
            
            DrawWindHUD(ball.windAcceleration, screen_width);   // wind indicator, top-right                                                                         // 20 = font size 
            DrawPowerBar(cannon.getLaunchSpeed(), screen_width, screen_height);   // <-- add this
                                                                        

        EndDrawing();  //pushes frame to screen 
    } //closes the while loop
CloseWindow();
return 0;
}