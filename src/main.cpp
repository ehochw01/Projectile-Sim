#include "raylib.h" //imports raylib library
#include "Projectile.h"
#include <cmath>
#include "Cannon.h"
#include "Target.h"
#include "Constants.h"
#include "Debris.h"
#include <vector> //we will store debris objects in a vector



//helper function to draw the wind arrow for the user. Wind changes every 3 shots. 
void DrawWindHUD(Vector3 wind, int screenWidth) {          
    //wind will be drawn relative to screen width 
    Vector2 center = { (float)screenWidth - 80.0f, 90.0f };   // top-right corner
    // arrow length
    float radius = 38.0f;                                      

    // wind magnitude (no y down or up wind)
    float windSpeed = sqrtf(wind.x * wind.x + wind.z * wind.z);   

    // relative to the camera here is how the world looks:
    // player looks down +X. On their screen:
    // world +X (downrange, away) -> screen UP
    // world -Z (viewer's right)  -> screen RIGHT
    float dirX = wind.z;   // screen x-component for wind
    float dirY = wind.x;   // screen y-component for wind

    // draws wind dial ring to screen, with a radius slightly larger than the arrow length.
    DrawCircleLines((int)center.x, (int)center.y, radius + 10, GRAY);   

    DrawText("WIND", (int)center.x - 20, (int)center.y - radius - 34, 16, BLACK);

    if (windSpeed > 0.01f) {
        float len = sqrtf(dirX * dirX + dirY * dirY);
        dirX /= len;  dirY /= len;                             // normalize to unit direction

        // arrow length steps through 4 buckets so speed changes are easy to read at a glance
        // (wind strength is generated in [0, 10.0] m/s, see PhysicsBody::GenerateWind)
        float arrowLen;
        if (windSpeed < 2.0f) {
            arrowLen = 14.0f;   // calm / light air
        } else if (windSpeed < 5.0f)  {
            arrowLen = 22.0f;   // light-to-gentle breeze
        } else if (windSpeed < 8.0f) { 
            arrowLen = 30.0f;   // moderate breeze
        } else { 
            arrowLen = radius;  // fresh breeze (8-10 m/s)
        }

        Vector2 tip  = { center.x + dirX * arrowLen, center.y + dirY * arrowLen };  //get the two points drawn up then join em
        Vector2 tail = { center.x - dirX * arrowLen, center.y - dirY * arrowLen };
        DrawLineEx(tail, tip, 4.0f, RED);                      // shaft

        // arrowhead: a triangle at the tip
        float perpX = -dirY, perpY = dirX;                     // perpendicular to direction,def variables
        Vector2 base = { center.x + dirX * fmaxf(arrowLen - 12, 0.0f), center.y + dirY * fmaxf(arrowLen - 12, 0.0f) };
        Vector2 left  = { base.x + perpX * 9, base.y + perpY * 9 };
        Vector2 right = { base.x - perpX * 9, base.y - perpY * 9 };
        DrawTriangle(tip, right, left, RED);
    }

    // draw the decimal point as a manual dot since the font's "." glyph
    // is too small to read at this size; carry the rounding into the whole part
    int fontSize = 18;
    int wholePart = (int)windSpeed;
    int tenths = (int)roundf((windSpeed - wholePart) * 10.0f);
    if (tenths >= 10) { tenths = 0; wholePart += 1; }

    const char* wholeStr = TextFormat("%d", wholePart);
    const char* tailStr  = TextFormat("%d m/s", tenths);

    int textX = (int)center.x - 30;
    int textY = (int)center.y + radius + 14;

    DrawText(wholeStr, textX, textY, fontSize, BLACK);
    int wholeWidth = MeasureText(wholeStr, fontSize);

    float dotRadius = 2.5f;
    float dotX = textX + wholeWidth + dotRadius + 2;
    float dotY = textY + fontSize - dotRadius - 2;   // sits on the text baseline
    DrawCircle((int)dotX, (int)dotY, dotRadius, BLACK);

    DrawText(tailStr, (int)(dotX + dotRadius + 3), textY, fontSize, BLACK);
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

//helper function to spawn debris, takes in a vector of debris objects and shoots them out from some chosen position, to be determined by the location of a target and activated only when struck
void spawnDebris(std::vector<Debris>& debris, Vector3 origin) { //this function takes a vector of debris objects, and shoots them out from some chosen position, to be determined by the location of a target and activated only when struck
    int count = 50; //number of debris objects to spawn
    for (int i = 0; i < count; ++i) {
        Debris piece;
        piece.position = origin; //spawn at the given position vector 
        piece.radius = GetRandomValue(15, 35) / 100.0f;   // radius 0.15..0.35, varied
        int r = GetRandomValue(130, 255);
        int g = GetRandomValue(0, 100);
        int b = GetRandomValue(100, 200);
        piece.color = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 }; //casts the ints tha random generates and unsigned char (0-255
        piece.velocity = {
            GetRandomValue(-100,100) / 10.0f, //random x velocity between -6 and 6 m/s, division is because random number gen doesnt give floats
            GetRandomValue(100,150) / 10.0f, //random y velocity between 2 and 8, positive so it "erupts" upwards, recent change to make it more satisfying
            GetRandomValue(-100,100) / 10.0f  //random z velocity between -6 and 6 m/s, outward
        };
        debris.push_back(piece); // didn't have this at first and it didnt work, this is what adds the finished fragment to the original vector
    }
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

    std::vector<Debris> debris; //vector to hold debris objects, will be filled when target is hit
 //this is our mainloop, 
    while (!WindowShouldClose()) {    //will be true until we hit escape key, only way to end the sim!
        float fTime = GetFrameTime(); //seconds since last frame, in our case 1/60 secs, then uses this to feed the physics engine

        // INPUT 

        //for now, spawn debris is called at this position as a test, eventually will be called at the location of struck targets.
        if (IsKeyPressed(KEY_D)) spawnDebris(debris, {20.0f, 5.0f, 0.0f});   // TEST: erupt debris

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
        for (Debris& piece : debris) {
            piece.Update(fTime);   // gravity + bounce, all inherited
        }

        BeginDrawing();
            ClearBackground((Color){ 135, 206, 235, 255 });  //sky blue in RBG values
            
            BeginMode3D(camera);           //enter 3D space from view of camera defined above 
                DrawWorld();       //uses helper function above to draw the world as defined.

                cannon.Draw();   //draw the cannon at the origin, barrel points along the current aim

                ball.Draw(); //draw the ball at its current location, everytime this is hit in each loop, it uses the new updated ball position as derived by the math in the physicsbody source code.
                target.Draw(); //draw the target at its location, currently static but we could make it move later if we wanted.
                for (Debris& piece : debris) piece.Draw();   // all fragments, must be by reference! early mistake

                DrawSphere({0,0,0}, 0.3f, RED);  //small sphere to mark the center of the grid.
            EndMode3D(); //no longer drawing in the 3d world after this, but on the flat 2d screen

            

            DrawText("Projectile Sim, Posts every 25 meters", 10,10,20,DARKGRAY);  //text, 10, 10 = x y position from left and top edge
            DrawText("Use arrow keys to aim, hold space to charge, release to fire", 10, 40, 20, DARKGRAY);
            DrawText("Score: ", 10, 70, 20, DARKGRAY);                                                                          // // DARKGRAY = color
                                                                                     // Eric this is where the "Target Hit!" text would be.
            
            DrawWindHUD(ball.windAcceleration, screen_width);   // wind indicator, top-right                                                                         // 20 = font size 
            DrawPowerBar(cannon.getLaunchSpeed(), screen_width, screen_height);   // <-- add this
                                                                        

        EndDrawing();  //pushes frame to screen 
    } //closes the while loop
CloseWindow();
return 0;
}