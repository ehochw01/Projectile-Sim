#include "raylib.h" //imports raylib library
#include "Projectile.h"
#include <cmath>
#include "Cannon.h"
#include "Target.h"
#include "Constants.h"
#include "Debris.h"
#include <vector> //we will store debris objects in a vector

using Constants::DEBRIS_COLOR_VARIATION;

// screen-space star for the twinkling night sky, stored in an array before the game loop
struct Starry {
    int x, y;
    float phase;   // offset so each star twinkles on its own cycle
    Color color;   // mostly white, occasional red/blue/yellow
};

// keeps a color channel in [0,255] so it's safe to cast to unsigned char without wrapping around
int ClampColorChannel(int value) {
    if (value < 0) return 0;
    if (value > 255) return 255;
    return value;
}

//helper function to draw the wind arrow for the user. Wind changes every 3 shots. 
void DrawWindHUD(Vector3 wind, int screenWidth, bool isNight) {
    Color textCol = isNight ? WHITE : BLACK;
    Color ringCol = isNight ? LIGHTGRAY : GRAY;
    Vector2 center = { (float)screenWidth - 80.0f, 90.0f };
    float radius = 38.0f;

    float windSpeed = sqrtf(wind.x * wind.x + wind.z * wind.z);

    // player looks down +X. On their screen:
    // world +X (downrange, away) -> screen UP
    // world -Z (viewer's right)  -> screen RIGHT
    float dirX = wind.z;
    float dirY = -wind.x;

    DrawCircleLines((int)center.x, (int)center.y, radius + 10, ringCol);

    DrawText("WIND", (int)center.x - 20, (int)center.y - radius - 34, 16, textCol);

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

    DrawText(wholeStr, textX, textY, fontSize, textCol);
    int wholeWidth = MeasureText(wholeStr, fontSize);

    float dotRadius = 2.5f;
    float dotX = textX + wholeWidth + dotRadius + 2;
    float dotY = textY + fontSize - dotRadius - 2;
    DrawCircle((int)dotX, (int)dotY, dotRadius, textCol);

    DrawText(tailStr, (int)(dotX + dotRadius + 3), textY, fontSize, textCol);
}


//Helper function, drawworld — draws ground, scenery, and sky objects
// isNight is decided once at startup and passed in each frame
void DrawWorld(bool isNight) {
    // ground plane — (darker grass at night)
    Color grass = isNight ? (Color){30, 50, 25, 255} : (Color){76, 124, 60, 255};
    DrawPlane({0.0f, 0.0f, 0.0f}, {1000.0f, 1000.0f}, grass);

    DrawGrid(200, 5.0f);

    // range markers: low posts every 25m downrange (+X), in pairs
    for (int dist = 25; dist <= 200; dist += 25) {
        float x = (float)dist;
        DrawCube({ x, 1.0f,  12.0f }, 0.4f, 2.0f, 0.4f, (Color){ 200, 200, 200, 255 });
        DrawCube({ x, 1.0f, -12.0f }, 0.4f, 2.0f, 0.4f, (Color){ 200, 200, 200, 255 });
    }

    // Foreground/Background scenery:

    // trees along the edges, well clear of the firing lane
    // colors darken at night so they read as silhouettes
    Color trunk  = isNight ? (Color){60, 40, 20, 255}  : (Color){101, 67, 33, 255};
    Color leaves = isNight ? (Color){15, 70, 15, 255}   : (Color){34, 139, 34, 255};
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

    // tree shadows — dark discs on the ground, only during the day
    if (!isNight) {
        Color shadow = {0, 0, 0, 60};
        DrawCylinder({30.0f,  0.02f, 25.0f},  3.5f, 3.5f, 0.01f, 10, shadow);
        DrawCylinder({70.0f,  0.02f, -30.0f}, 4.5f, 4.5f, 0.01f, 10, shadow);
        DrawCylinder({120.0f, 0.02f, 28.0f},  3.0f, 3.0f, 0.01f, 10, shadow);
        DrawCylinder({45.0f,  0.02f, -35.0f}, 4.0f, 4.0f, 0.01f, 10, shadow);
        DrawCylinder({90.0f,  0.02f, 32.0f},  5.0f, 5.0f, 0.01f, 10, shadow);
        DrawCylinder({160.0f, 0.02f, -25.0f}, 3.5f, 3.5f, 0.01f, 10, shadow);
    }

    // boulders scattered off to the sides
    Color stone = isNight ? (Color){80, 80, 80, 255} : (Color){140, 140, 140, 255};
    DrawSphere({25.0f,  0.9f,  18.0f}, 1.5f, stone);
    DrawSphere({80.0f,  1.2f, -20.0f}, 2.0f, stone);
    DrawSphere({55.0f,  0.6f,  22.0f}, 1.0f, stone);
    DrawSphere({140.0f, 1.0f, -18.0f}, 1.8f, stone);

    // distant mountains — darker at night
    Color mountain = isNight ? (Color){60, 60, 75, 255} : (Color){100, 100, 120, 255};
    DrawCylinder({300.0f, 0.0f, -190.0f},  0.0f, 60.0f, 80.0f, 6, mountain);
    DrawCylinder({350.0f, 0.0f, -160.0f},  0.0f, 50.0f, 65.0f, 6, mountain);
    DrawCylinder({260.0f, 0.0f, -210.0f}, 0.0f, 45.0f, 55.0f, 6, mountain);
    DrawCylinder({320.0f, 0.0f,  -90.0f},  0.0f, 55.0f, 70.0f, 6, mountain);

    // sky objects — sun and clouds during the day, moon at night
    if (!isNight) {
        // big yellow sun — y=110 keeps it within the camera's ~120 visible height at this distance
        DrawSphere({200.0f, 90.0f, -120.0f}, 15.0f, YELLOW);

        // clouds — flat bottoms, bumpy tops, wide horizontal spread
        // KEY: z controls left/right. keep |z| under ~25 for close clouds (x<100),
        //      under ~40 for mid-range, so they stay within the camera's horizontal FOV
        Color cloudTop = {255, 255, 255, 230};
        Color cloudMid = {240, 240, 245, 220};
        Color cloudBot = {210, 215, 225, 200};

        // cloud 1 — big cumulus, right side
        DrawSphere({80.0f, 78.0f, -90.0f}, 7.0f, cloudBot);
        DrawSphere({87.0f, 78.0f, -87.0f}, 8.0f, cloudBot);
        DrawSphere({95.0f, 78.0f, -84.0f}, 7.0f, cloudBot);
        DrawSphere({83.0f, 82.0f, -89.0f}, 7.0f, cloudMid);
        DrawSphere({90.0f, 83.0f, -86.0f}, 8.0f, cloudMid);
        DrawSphere({97.0f, 81.0f, -83.0f}, 6.0f, cloudMid);
        DrawSphere({86.0f, 86.0f, -88.0f}, 5.0f, cloudTop);
        DrawSphere({92.0f, 88.0f, -85.0f}, 6.0f, cloudTop);

        // cloud 2 — wide, far left
        DrawSphere({130.0f, 72.0f, 100.0f}, 8.0f, cloudBot);
        DrawSphere({138.0f, 72.0f, 103.0f}, 9.0f, cloudBot);
        DrawSphere({146.0f, 72.0f, 106.0f}, 7.0f, cloudBot);
        DrawSphere({134.0f, 77.0f, 101.0f}, 8.0f, cloudMid);
        DrawSphere({142.0f, 78.0f, 104.0f}, 9.0f, cloudMid);
        DrawSphere({138.0f, 82.0f, 102.0f}, 6.0f, cloudTop);
        DrawSphere({144.0f, 81.0f, 105.0f}, 7.0f, cloudTop);

        // cloud 3 — wisp, far right (shifted away from the sun)
        DrawSphere({120.0f, 85.0f, -130.0f}, 6.0f, cloudBot);
        DrawSphere({127.0f, 85.0f, -127.0f}, 7.0f, cloudBot);
        DrawSphere({123.0f, 89.0f, -129.0f}, 5.0f, cloudMid);
        DrawSphere({129.0f, 88.0f, -126.0f}, 4.0f, cloudTop);

        // cloud 4 — cumulus, left
        DrawSphere({70.0f, 68.0f, 60.0f}, 7.0f, cloudBot);
        DrawSphere({77.0f, 68.0f, 63.0f}, 8.0f, cloudBot);
        DrawSphere({73.0f, 73.0f, 61.0f}, 7.0f, cloudMid);
        DrawSphere({79.0f, 72.0f, 64.0f}, 6.0f, cloudMid);
        DrawSphere({75.0f, 77.0f, 62.0f}, 5.0f, cloudTop);

        // cloud 5 — long streak, center
        DrawSphere({170.0f, 80.0f, -10.0f}, 6.0f, cloudBot);
        DrawSphere({178.0f, 80.0f, -7.0f},  7.0f, cloudBot);
        DrawSphere({186.0f, 80.0f, -4.0f},  6.0f, cloudBot);
        DrawSphere({194.0f, 80.0f, -1.0f},  5.0f, cloudBot);
        DrawSphere({174.0f, 84.0f, -8.0f},  6.0f, cloudMid);
        DrawSphere({183.0f, 85.0f, -5.0f},  7.0f, cloudMid);
        DrawSphere({180.0f, 88.0f, -6.0f},  5.0f, cloudTop);

        // cloud 6 — small puff, far left
        DrawSphere({240.0f, 82.0f, 130.0f}, 5.0f, cloudBot);
        DrawSphere({246.0f, 82.0f, 133.0f}, 6.0f, cloudBot);
        DrawSphere({243.0f, 86.0f, 131.0f}, 5.0f, cloudTop);
    } else {
        // pale moon low in the sky
        DrawSphere({150.0f, 70.0f, -120.0f}, 12.0f, (Color){240, 240, 220, 255});
    }
}

// another helper function to draw the power charge bar. Bottom-center, fills red as power climbs 0...100
void DrawPowerBar(float power, int screenWidth, int screenHeight, bool isNight) {
    Color textCol = isNight ? WHITE : BLACK;
    int barWidth  = 300;
    int barHeight = 24;
    int x = screenWidth / 2 - barWidth / 2;
    int y = screenHeight - 60;

    float fraction = power / 100.0f;
    int fillWidth = (int)(barWidth * fraction);

    DrawRectangle(x, y, barWidth, barHeight, (Color){ 40, 40, 40, 200 });
    DrawRectangle(x, y, fillWidth, barHeight, RED);
    DrawRectangleLines(x, y, barWidth, barHeight, textCol);
    DrawText("POWER", x, y - 22, 18, textCol);
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

    // random time of day — decided once at startup, affects sky color, scenery colors, and stars
    bool isNight = GetRandomValue(0, 1);

    // pre-generate stars for the night sky (only drawn when isNight is true)
    const int STAR_COUNT = 200;
    Starry stars[200];
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = GetRandomValue(0, screen_width);
        stars[i].y = GetRandomValue(0, screen_height / 2);  // upper half of screen only
        stars[i].phase = GetRandomValue(0, 628) / 100.0f;   // 0 to ~2*pi
        // mostly white, occasional colored star
        int colorRoll = GetRandomValue(0, 10);
        if (colorRoll == 0)      stars[i].color = {255, 100, 100, 255};  // red
        else if (colorRoll == 1) stars[i].color = {100, 150, 255, 255};  // blue
        else if (colorRoll == 2) stars[i].color = {255, 255, 100, 255};  // yellow
        else                     stars[i].color = {255, 255, 255, 255};  // white
    }



    int turnCount = 0; //keeps track of how many shots have been fired, used to change wind every 3 shots
    int hitCount = 0;  //keeps track of how many hits the player has scored, used to change target color and size every 3 hits
    int missCount = 0; //keeps track of how many misses the player has left, game over when it reaches 0

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

    int hitValue = 0;     //0 = no hit, 1 = outer ring ... 5 = bullseye, from Target::CheckHit
    bool collision = false; //flag to indicate if the ball has hit the target, initially false
    int score = 0; //initialize score to 0, will increment when target is hit

    bool targetVisible = true;          // hidden briefly after a hit while it "respawns" elsewhere
    float targetRespawnTimer = 0.0f;    // counts down from Constants::TARGET_RESPAWN_DELAY while hidden

    Vector3 hitPopupPos = {0.0f, 0.0f, 0.0f};   // where the target was when it was last hit, for the "+N" popup
    int hitPopupValue = 0;                       // the ring value to show in that popup

    while (!WindowShouldClose()) {    // will be true until we hit escape key 
        float fTime = GetFrameTime(); // seconds since last frame, in our case 1/60 secs, then uses this to feed the physics engine

        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))    cannon.incrElevation(fTime);
        if (IsKeyDown(KEY_LEFT) ||IsKeyDown(KEY_A))  cannon.decrAzimuth(fTime);
        if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))  cannon.decrElevation(fTime);
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) cannon.incrAzimuth(fTime);

        // charge while holding space
        if (IsKeyDown(KEY_SPACE)) {
            cannon.incrLaunchSpeed(fTime);
        }
        // fire on release
        if (IsKeyReleased(KEY_SPACE)) {
            cannon.Fire(ball);
            ball.active = true;
            turnCount++;
        }

        Vector3 prevBallPos = ball.position;   // remember where the ball was before integrating this frame
        if (ball.active) ball.Update(fTime);   // only simulate a ball in flight

        // disk hit test: only checked on the frame the ball crosses the target's face (no repeated hits)
        // skipped while the target is hidden/respawning so the ball can't hit something that isn't there
        hitValue = (targetVisible && ball.active) ? target.CheckHit(prevBallPos, ball.position, ball.radius) : 0;
        collision = hitValue > 0;

        for (Debris& piece : debris) {
            piece.Update(fTime);   // gravity + bounce, all inherited
        }

        if (collision) {
            hitCount++;
            spawnDebris(debris, ball.position, target.GetColor()); //spawn debris where the ball actually struck the disk
            collision = false; //reset collision flag to avoid repeated spawning
            ball.active = false; //stop simulating/drawing the ball now that it's struck something
            targetVisible = false;                                   // target "explodes" and disappears on impact
            targetRespawnTimer = Constants::TARGET_RESPAWN_DELAY;     // ...and stays gone for a short pause
            hitPopupPos = target.position;   // remember where it was hit, before it moves/shrinks below
            hitPopupValue = hitValue;
            score += hitValue; //add the ring value (1 = outer ring ... 5 = bullseye)
            if (hitCount % 3 == 0) {
                target.ChangeColor(); //change the color of the target to make it more visually interesting
                target.Shrink(); 
                ball.GenerateWind(); //generate new wind every 3 shots, so the player has to adjust their aim
                centerTargetPosition(target); //center the target after shrinking
            } else {
                randomizeTarget(target);
                missCount++; //increment the number of misses, if the player misses the target
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
            // sky color: light blue by day, deep navy at night
            if (isNight) ClearBackground((Color){ 15, 15, 40, 255 });
            else         ClearBackground((Color){ 135, 206, 235, 255 });

            // twinkling stars — drawn BEFORE the 3D scene so mountains/trees paint over them
            if (isNight) {
                float t = (float)GetTime();
                for (int i = 0; i < STAR_COUNT; i++) {
                    // each star blinks once every ~8-17 seconds (varies per star)
                    // the blink itself is a quick 0.3s dip to ~10% brightness
                    float period = 8.0f + stars[i].phase * 1.5f;
                    float offset = stars[i].x * 0.34f + stars[i].y * 0.69f;
                    float cycle = fmodf(t + offset, period);
                    float blinkLen = 1.5f;
                    float alpha;
                    if (cycle < blinkLen) {
                        float progress = cycle / blinkLen;
                        alpha = 0.1f + 0.9f * fabsf(cosf(progress * 3.14159f));
                    } else {
                        alpha = 1.0f;
                    }
                    Color c = stars[i].color;
                    c.a = (unsigned char)(alpha * 255);
                    DrawCircle(stars[i].x, stars[i].y, 1.5f, c);
                }
            }

            BeginMode3D(camera);
                DrawWorld(isNight);

                cannon.Draw();   //draw the cannon at the origin, barrel points along the current aim

                if (ball.active) ball.Draw(); //draw the ball only while it's in flight; hidden once it strikes something
                if (targetVisible) target.Draw(); //draw the target at its location, hidden during the brief respawn pause
                for (Debris& piece : debris) piece.Draw();   // all fragments, must be by reference! early mistake

                DrawSphere({0,0,0}, 0.3f, RED);  //small sphere to mark the center of the grid.


            EndMode3D(); //no longer drawing in the 3d world after this, but on the flat 2d screen

            Color textCol = isNight ? WHITE : DARKGRAY;

            DrawText("Projectile Sim", 10,10,20, textCol);
            DrawText(TextFormat("Score: %d", score), 10, 40, 20, textCol);

            // "+N" popup floats above where the target was hit, for as long as it's gone (TARGET_RESPAWN_DELAY)
            if (!targetVisible) {
                Vector3 popupWorldPos = { hitPopupPos.x, hitPopupPos.y + target.radius /3, hitPopupPos.z };
                Vector2 popupScreenPos = GetWorldToScreen(popupWorldPos, camera);
                const char* popupText = TextFormat("+%d", hitPopupValue);
                int popupFontSize = 30;
                int popupTextWidth = MeasureText(popupText, popupFontSize);
                DrawText(popupText, (int)popupScreenPos.x - popupTextWidth / 2, (int)popupScreenPos.y, popupFontSize, textCol);
            }
            if (turnCount == 0) {
                DrawText("Use arrow keys to aim, hold space to charge, release to fire", 10, 70, 20, textCol);
            }

            DrawText("Misses Left: ", 10,680,20, textCol);  //text, 10, 10 = x y position from left and top edge
            
            DrawWindHUD(ball.windAcceleration, screen_width, isNight);
            DrawPowerBar(cannon.getLaunchSpeed(), screen_width, screen_height, isNight);
                                                                        

        EndDrawing();  //pushes frame to screen 
    } //closes the while loop
CloseWindow();
return 0;
}