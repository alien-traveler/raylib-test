// main.c
#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"   // for UpdateCamera()
#include "parson.h"    // https://github.com/kgabis/parson

#define PLAYER_W   0.5f
#define PLAYER_H   1.0f
#define PLAYER_D   0.5f
#define MOVE_SPEED 5.0f

static BoundingBox MakeCubeBox(Vector3 c, float w, float h, float d) {
    return (BoundingBox) {
        { c.x - w * 0.5f, c.y - h * 0.5f, c.z - d * 0.5f },
        { c.x + w * 0.5f, c.y + h * 0.5f, c.z + d * 0.5f }
    };
}

int main(void)
{
    float camYaw = 180.0f;       // degrees, start looking toward -Z
    float camPitch = 20.0f;        // degrees
    float camDist = 6.0f;         // 3rd‑person distance

    /* ── load JSON boxes ─────────────────────────────────────────────── */
    const char* fileAddr =
        "C:\\Users\\20020\\Desktop\\CSE125\\raylib-quickstart-main\\"
        "raylib-quickstart-main\\resources\\bb#_bboxes.json";

    JSON_Value* rootVal = json_parse_file(fileAddr);
    if (!rootVal) { fprintf(stderr, "Cannot parse %s\n", fileAddr); return 1; }
    JSON_Object* rootObj = json_value_get_object(rootVal);
    size_t       boxCnt = json_object_get_count(rootObj);

    BoundingBox* boxes = malloc(boxCnt * sizeof(BoundingBox));
    Color* colors = malloc(boxCnt * sizeof(Color));

    for (size_t i = 0;i < boxCnt;i++) {
        JSON_Object* o = json_object_get_object(rootObj,
            json_object_get_name(rootObj, i));
        JSON_Array* mn = json_object_get_array(o, "min");
        JSON_Array* mx = json_object_get_array(o, "max");
        /* ---- read Blender coords ---- */
        Vector3 bMin = { (float)json_array_get_number(mn,0),
                         (float)json_array_get_number(mn,1),
                         (float)json_array_get_number(mn,2) };
        Vector3 bMax = { (float)json_array_get_number(mx,0),
                         (float)json_array_get_number(mx,1),
                         (float)json_array_get_number(mx,2) };
        boxes[i].min = (Vector3){
           bMin.x,               // X stays X
           bMin.z,               // Blender Z  becomes Raylib Y
          -bMax.y                // Blender Y  (depth) becomes -Z
        };
        boxes[i].max = (Vector3){
            bMax.x,
            bMax.z,
           -bMin.y
        };
        colors[i] = (Color){ GetRandomValue(150,255),
                           GetRandomValue(150,255),
                           GetRandomValue(150,255),200 };
    }
    json_value_free(rootVal);

    /* ── window & camera ─────────────────────────────────────────────── */
    InitWindow(1920, 1080, "Cube + JSON boxes (3 camera modes)");
    DisableCursor();
    SetTargetFPS(60);

    Camera camera = { 0 };
    //Vector3 spawnPos = (Vector3){ -4.0f, PLAYER_H * 0.5f, -4.0f };
    Vector3 spawnPos = (Vector3){ 1.0f, 1.0f, 0.5f };
    camera.position = spawnPos;
    camera.target = spawnPos;
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.f;
    camera.projection = CAMERA_PERSPECTIVE;

    enum { MODE_FREE = 0, MODE_FIRST, MODE_THIRD };
    int camMode = MODE_FIRST;

    Vector3 playerPos = spawnPos;
    Vector3 prevPlayerPos, prevCamPos, prevCamTar;

    /* ── main loop ───────────────────────────────────────────────────── */
    while (!WindowShouldClose())
    {
        prevPlayerPos = playerPos;
        prevCamPos = camera.position;
        prevCamTar = camera.target;

        /* switch modes */
        if (IsKeyPressed(KEY_ONE))   camMode = MODE_FREE;
        if (IsKeyPressed(KEY_TWO))   camMode = MODE_FIRST;
        if (IsKeyPressed(KEY_THREE)) camMode = MODE_THIRD;

        ///* camera look / orbit -------------------------------------------- */
        //UpdateCamera(&camera,
        //    camMode == MODE_FREE ? CAMERA_FREE :
        //    camMode == MODE_FIRST ? CAMERA_FIRST_PERSON : CAMERA_THIRD_PERSON);

        ///* --- KEEP THE WORLD UPRIGHT ------------------------------------- */
        //camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };   // force Y‑up every frame

        if (camMode == MODE_FREE) {
            UpdateCamera(&camera, CAMERA_FREE);
        }
        else if (camMode == MODE_FIRST) {
            /* lock eyes to cube centre, use Raylib FPS controls */
            camera.position = playerPos;
            Vector3 temp = { 0.0f, 0.0f, -1.0f };
            camera.target = Vector3Add(playerPos, temp);  // dummy forward
            UpdateCamera(&camera, CAMERA_FIRST_PERSON);
        }
        else {  /* MODE_THIRD — custom orbit ------------------------- */
            Vector2 mouse = GetMouseDelta();
            const float SENS = 0.3f;
            camYaw -= mouse.x * SENS;
            camPitch -= mouse.y * SENS;
            if (camPitch > 85) camPitch = 85;
            if (camPitch < -85) camPitch = -85;

            camDist += GetMouseWheelMove() * -0.5f;
            if (camDist < 2.0f) camDist = 2.0f;
            if (camDist > 12.0f) camDist = 12.0f;

            float yawRad = DEG2RAD * camYaw;
            float pitchRad = DEG2RAD * camPitch;

            Vector3 offset = {
                camDist * cosf(pitchRad) * sinf(yawRad),
                camDist * sinf(pitchRad),
                camDist * cosf(pitchRad) * cosf(yawRad)
            };

            camera.target = playerPos;
            camera.position = Vector3Add(playerPos, offset);
            camera.up = (Vector3){ 0,1,0 };   /* KEEP Y‑UP */
        }


        /* handle cube movement on X‑Z plane (always WASD world‑aligned) */
        float dt = GetFrameTime();
        Vector3 dir = { 0 };
        if (IsKeyDown(KEY_W)) dir.z -= 1;
        if (IsKeyDown(KEY_S)) dir.z += 1;
        if (IsKeyDown(KEY_A)) dir.x -= 1;
        if (IsKeyDown(KEY_D)) dir.x += 1;
        if (dir.x || dir.z) {
            dir = Vector3Normalize(dir);
            playerPos = Vector3Add(playerPos, Vector3Scale(dir, MOVE_SPEED * dt));
        }

        /* sync player↔camera depending on mode */
        if (camMode == MODE_FIRST) {
            camera.position = playerPos;                 // eyes in cube center
        }
        else if (camMode == MODE_THIRD) {
            camera.target = playerPos;                 // look at cube
            playerPos = camera.target;
        }


        /* collision test */
        BoundingBox pBox = MakeCubeBox(playerPos, PLAYER_W, PLAYER_H, PLAYER_D);
        bool hit = false;
        for (size_t i = 0;i < boxCnt && !hit;i++)
            if (CheckCollisionBoxes(pBox, boxes[i])) hit = true;
        if (hit) {                       // rollback
            playerPos = prevPlayerPos;
            camera.position = prevCamPos;
            camera.target = prevCamTar;
        }

        /* ── draw ──────────────────────────────────────────────────── */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 50, 50 }, LIGHTGRAY);

        for (size_t i = 0;i < boxCnt;i++) {
            Vector3 sz = Vector3Subtract(boxes[i].max, boxes[i].min);
            Vector3 ce = Vector3Add(boxes[i].min, Vector3Scale(sz, 0.5f));
            DrawCube(ce, sz.x, sz.y, sz.z, colors[i]);
            DrawCubeWires(ce, sz.x, sz.y, sz.z, DARKGRAY);
        }

        DrawCube(playerPos, PLAYER_W, PLAYER_H, PLAYER_D, RED);
        DrawCubeWires(playerPos, PLAYER_W, PLAYER_H, PLAYER_D, MAROON);
        EndMode3D();

        DrawText("Modes: [1] Free  [2] First‑person  [3] Third‑person",
            10, 10, 20, BLACK);
        DrawText(TextFormat("Current: %s",
            camMode == MODE_FREE ? "FREE" :
            camMode == MODE_FIRST ? "FIRST PERSON" : "THIRD PERSON"),
            10, 35, 20, BLACK);
        DrawFPS(1180, 10);
        EndDrawing();
    }

    free(boxes); free(colors);
    CloseWindow();
    return 0;
}
