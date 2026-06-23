#include "raylib.h" //imports raylib library
#include "Projectile.h"
#include <cmath>
#include "Cannon.h"
#include "Target.h"
#include "Constants.h"
#include "Debris.h"
#include <vector> //we will store debris objects in a vector

using Constants::DEBRIS_COLOR_VARIATION;

// keeps a color channel in [0,255] so it's safe to cast to unsigned char without wrapping around
int ClampColorChannel(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

//helper function to draw the wind arrow for the user. Wind changes every 3 shots. 
void DrawWindHUD(Vector3 wind, int screenWidth) {          
    //wind will be drawn relative to screen width 
    Vector2 center = { (float)screenWidth - 80.0f, 90.0f };   // top-right corner
    // arrow length
    float radius = 38.0f;                                      

    // wind magnitude (no y down or up wind)
    float windSpeed = sqrtf(wind.x * wind.x + wind.z * wind.z);   

    // player looks down +X. On their screen:
    // world +X (downrange, away) -> screen UP
    // world -Z (viewer's right)  -> screen RIGHT
    float dirX = wind.z;   // screen x-component for wind
    float dirY = -wind.x;   // screen y-component for wind

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

        // Background scenery:

    // trees along the edges, well clear of the firing lane
    Color trunk  = {101, 67, 33, 255};   // brown
    Color leaves = {34, 139, 34, 255};   // forest green
    // tree at x=30, z=25 (right side)
    DrawCylinder({30.0f, 0.0f, 25.0f}, 0.5f, 0.3f, 8.0f, 8, trunk);
    DrawSphere({30.0f, 9.8f, 25.0f}, 3.0f, leaves);
    // tree at x=70, z=-30 (left side, tall)
    DrawCylinder({70.0f, 0.0f, -30.0f}, 0.5f, 0.3f, 10.0f, 8, trunk);
    DrawSphere({70.0f, 12.4f, -30.0f}, 4.0f, leaves);
    // tree at x=120, z=28 (right side, shorter)
    DrawCylinder({120.0f, 0.0f, 28.0f}, 0.5f, 0.3f, 6.0f, 8, trunk);
    DrawSphere({120.0f, 7.5f, 28.0f}, 2.5f, leaves);
    // tree at x=45, z=-35 (left side)
    DrawCylinder({45.0f, 0.0f, -35.0f}, 0.5f, 0.3f, 9.0f, 8, trunk);
    DrawSphere({45.0f, 11.1f, -35.0f}, 3.5f, leaves);
    // tree at x=90, z=32 (right side, big)
    DrawCylinder({90.0f, 0.0f, 32.0f}, 0.6f, 0.3f, 12.0f, 8, trunk);
    DrawSphere({90.0f, 14.7f, 32.0f}, 4.5f, leaves);
    // tree at x=160, z=-25 (left side, far)
    DrawCylinder({160.0f, 0.0f, -25.0f}, 0.5f, 0.3f, 7.0f, 8, trunk);
    DrawSphere({160.0f, 8.8f, -25.0f}, 3.0f, leaves);

    // boulders scattered off to the sides
    Color stone = {140, 140, 140, 255};
    DrawSphere({25.0f,  0.9f,  18.0f}, 1.5f, stone);
    DrawSphere({80.0f,  1.2f, -20.0f}, 2.0f, stone);
    DrawSphere({55.0f,  0.6f,  22.0f}, 1.0f, stone);
    DrawSphere({140.0f, 1.0f, -18.0f}, 1.8f, stone);

    // big yellow sun high in the sky
    DrawSphere({200.0f, 110.0f, -110.0f}, 20.0f, YELLOW);

    // cloud clusters: groups of overlapping white spheres 
    Color cloud = {255, 255, 255, 220};
    DrawSphere({60.0f,  80.0f, -48.0f}, 8.0f, cloud);
    DrawSphere({65.0f,  80.0f, -43.0f}, 7.0f, cloud);
    DrawSphere({55.0f,  80.0f, -45.0f}, 6.0f, cloud);

    DrawSphere({120.0f, 70.0f,  45.0f}, 10.0f, cloud);
    DrawSphere({125.0f, 70.0f,  28.0f}, 8.0f,  cloud);
    DrawSphere({120.0f, 70.0f,  34.0f}, 7.0f,  cloud);

    DrawSphere({175.0f, 85.0f, -20.0f}, 9.0f, cloud);
    DrawSphere({185.0f, 80.0f, -16.0f}, 7.0f, cloud);

    // distant mountains: upside down cones behind the firing range 
    Color mountain = {100, 100, 120, 255};
    DrawCylinder({300.0f, 0.0f, -190.0f},  0.0f, 60.0f, 80.0f, 6, mountain);
    DrawCylinder({350.0f, 0.0f, -160.0f},  0.0f, 50.0f, 65.0f, 6, mountain);
    DrawCylinder({260.0f, 0.0f, -210.0f}, 0.0f, 45.0f, 55.0f, 6, mountain);
    DrawCylinder({320.0f, 0.0f,  -90.0f},  0.0f, 55.0f, 70.0f, 6, mountain);


    // how do i make a night time?
    // I would want a random time of day at the start(probably outside drawworld)
    // then inside drawworld, I would check the time of day and draw the sky accordingly

    // daytime with sun top left and clouds(light blue sky)
    // nighttime with moon and stars(dark blue, no clouds, dotted stars: mostly white, some red some blue and some yellow)
    // sunset? pink and orange sky, sun low on horizon on the right, maybe some stripey clouds

    // how difficult is a simple moving background? twinkling stars, ruslting trees etc?
    
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

// helper function to spawn debris, takes in a vector of debris objects and shoots them out from some chosen position, to be determined by the location of a target and activated only when struck
void spawnDebris(std::vector<Debris>& debris, Vector3 origin, Color tColor) { //this function takes a vector of debris objects, and shoots them out from some chosen position
    int count = 100; //number of debris objects to spawn
    const float debrisSpeed = 3.5f;   // <-- master knob: 1.0 = current, higher = faster debris
    for (int i = 0; i < count; i++) {
        Debris piece;
        piece.position = origin; //spawn at the given position vector 
        piece.radius = GetRandomValue(10, 30) / 100.0f;   // radius 0.15..0.35, varied
        int r = GetRandomValue(ClampColorChannel(tColor.r - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.r + DEBRIS_COLOR_VARIATION));
        int g = GetRandomValue(ClampColorChannel(tColor.g - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.g + DEBRIS_COLOR_VARIATION));
        int b = GetRandomValue(ClampColorChannel(tColor.b - DEBRIS_COLOR_VARIATION), ClampColorChannel(tColor.b + DEBRIS_COLOR_VARIATION));
        piece.color = (Color){ (unsigned char)r, (unsigned char)g, (unsigned char)b, 255 }; //casts the ints that random generates and unsigned char (0-255)
        piece.velocity = {
            debrisSpeed * GetRandomValue(-90, -30) / 10.0f, //random x velocity between -6 and 6 m/s, division is because random number gen doesnt give floats
            GetRandomValue(20,120) / 10.0f, //random y velocity between 2 and 8, positive so it "erupts" upwards, recent change to make it more satisfying
            GetRandomValue(-50,50) / 10.0f  //random z velocity between -6 and 6 m/s, outward
        };
        debris.push_back(piece); // didn't have this at first and it didnt work, this is what adds the finished fragment to the original vector
    }
}

// pick a fresh random spot for the target somewhere downrange
void randomizeTarget(Target& target) {
    float x = 50.0f;   // 40..150m downrange (+X)
    float y = (float)GetRandomValue(5, 20);    // 5..30m up
    float z = (float)GetRandomValue(-20, 20);   // -20..20m across the lane
    target.position = { x, y, z };
}

void centerTargetPosition(Target& target) {
    target.position = { 50.0f, 15.0f, 0.0f }; // center of the lane
}

int main() {
    const int screen_width = 1280;   //screen dimensions, const since they shouldn't change
    const int screen_height = 720; 
    SetConfigFlags(FLAG_WINDOW_TOPMOST);  // forces window to the front on launch
    InitWindow(screen_width,screen_height, "Projectile Simulator");
    SetTargetFPS(60); //sets the upper limit of the loop at 60 fps

    int turnCount = 0; //keeps track of how many shots have been fired, used to change wind every 3 shots

    // sets up the camera, behind the cannon looking fwd
    Camera3D camera = {};
    camera.position = {-10.0f, 2.5f, 0.0f}; //behind the camera (neg x), and above, (pos y), classic video game)
                                            // the reason for the f's is that raylib using a Vector3 struct that takes in float numbers, not doubles. 
    camera.target = {0.0f, 3.0f, 0.0f};  //what its looking at, slightly upward, this func draws line between position and target, to determine our line of sight
    camera.up = {0.0f, 1.0f, 0.0f};     //determines which way is up. 
    camera.fovy = 60.0f;   //fovy means field of view, in degrees
    camera.projection = CAMERA_PERSPECTIVE; //makes distant things shrink, parallel lines converge toward horizon, same as our eyes. other views available.

    // setup up a ball, angle and initial velocity. 
    Cannon cannon;   //owns aim + power and fires the ball along the barrel
    Projectile ball;
    ball.position = cannon.getPivot(); //cannon ball is in the barrel
    ball.active = false; //dormant ball, not simulated until fired

    Target target; // target is a sphere, will be drawn at a random location in the world.
    centerTargetPosition(target); // start the target in the center of the lane, not random yet

    ball.GenerateWind(); // generate wind for the first 3 shots

    std::vector<Debris> debris; // vector to hold debris objects, will be filled when target is hit

    bool collision = false; //flag to indicate if the ball has hit the target, initially false
    int score = 0; //initialize score to 0, will increment when target is hit

    bool targetVisible = true;          // hidden briefly after a hit while it "respawns" elsewhere
    float targetRespawnTimer = 0.0f;    // counts down from Constants::TARGET_RESPAWN_DELAY while hidden

    while (!WindowShouldClose()) {    // will be true until we hit escape key 
        float fTime = GetFrameTime(); // seconds since last frame, in our case 1/60 secs, then uses this to feed the physics engine

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
            turnCount++;
            if (turnCount % 3 == 0) { ball.GenerateWind(); }

        }

        Vector3 prevBallPos = ball.position;   // remember where the ball was before integrating this frame
        if (ball.active) ball.Update(fTime);   // only simulate a ball in flight

        // disk hit test: true only on the frame the ball crosses the target's face (no repeated hits)
        // skipped while the target is hidden/respawning so the ball can't hit something that isn't there
        collision = targetVisible && ball.active && target.CheckHit(prevBallPos, ball.position, ball.radius);

        for (Debris& piece : debris) {
            piece.Update(fTime);   // gravity + bounce, all inherited
        }

        if (collision) {
            spawnDebris(debris, ball.position, target.GetColor()); //spawn debris where the ball actually struck the disk
            collision = false; //reset collision flag to avoid repeated spawning
            targetVisible = false;                                   // target "explodes" and disappears on impact
            targetRespawnTimer = Constants::TARGET_RESPAWN_DELAY;     // ...and stays gone for a short pause
            score++; //increment score when target is hit
            if (turnCount % 3 == 0) {
                target.RandomizeColor(); //change the color of the target to make it more visually interesting
                target.Shrink(); 
                centerTargetPosition(target); //center the target after shrinking
            } else {
                randomizeTarget(target);
            }
            
        }

        // once the pause elapses, move the target and bring it back
        if (!targetVisible) {
            targetRespawnTimer -= fTime;
            if (targetRespawnTimer <= 0.0f) {
                targetVisible = true;
            }
        }

        BeginDrawing();
            ClearBackground((Color){ 135, 206, 235, 255 });  //sky blue in RBG values
            
            BeginMode3D(camera);           //enter 3D space from view of camera defined above 
                DrawWorld();       //uses helper function above to draw the world as defined.

                cannon.Draw();   //draw the cannon at the origin, barrel points along the current aim

                ball.Draw(); //draw the ball at its current location, everytime this is hit in each loop, it uses the new updated ball position as derived by the math in the physicsbody source code.
                if (targetVisible) target.Draw(); //draw the target at its location, hidden during the brief respawn pause
                for (Debris& piece : debris) piece.Draw();   // all fragments, must be by reference! early mistake

                DrawSphere({0,0,0}, 0.3f, RED);  //small sphere to mark the center of the grid.
            EndMode3D(); //no longer drawing in the 3d world after this, but on the flat 2d screen

            DrawText("Projectile Sim", 10,10,20,DARKGRAY);  //text, 10, 10 = x y position from left and top edge
            DrawText(TextFormat("Score: %d", score), 10, 40, 20, DARKGRAY);
            if (turnCount == 0) {
                DrawText("Use arrow keys to aim, hold space to charge, release to fire", 10, 70, 20, DARKGRAY);
            }
            
            DrawWindHUD(ball.windAcceleration, screen_width);   // wind indicator, top-right                                                                         // 20 = font size 
            DrawPowerBar(cannon.getLaunchSpeed(), screen_width, screen_height);   // <-- add this
                                                                        

        EndDrawing();  //pushes frame to screen 
    } //closes the while loop
CloseWindow();
return 0;
}