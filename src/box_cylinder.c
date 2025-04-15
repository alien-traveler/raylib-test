/*******************************************************************************************
*   raylib – cylinder player with cylinder & box obstacles, flat ground
********************************************************************************************/
#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"

#define MAX_CYL_COLS   12
#define MAX_BOX_COLS   12
#define PLAYER_R       0.5f
#define PLAYER_H       1.0f
#define PLAYER_EYE_Y   (PLAYER_H*0.5f)

/* ---------- Collision helpers --------------------------------------------------------- */
static bool CheckCollisionCylinders(Vector3 aPos, float aR, float aH,
    Vector3 bPos, float bR, float bH)
{
    float dx = aPos.x - bPos.x, dz = aPos.z - bPos.z;
    if (sqrtf(dx * dx + dz * dz) > (aR + bR)) return false;
    float aMin = aPos.y - aH * 0.5f, aMax = aPos.y + aH * 0.5f;
    float bMin = bPos.y - bH * 0.5f, bMax = bPos.y + bH * 0.5f;
    return !(aMax < bMin || bMax < aMin);
}

/* cylinder (vertical axis) vs axis‑aligned box */
static bool CheckCollisionCylinderBox(Vector3 cPos, float cR, float cH,
    Vector3 bPos, float bW, float bH, float bD)
{
    /* vertical overlap */
    float cMin = cPos.y - cH * 0.5f, cMax = cPos.y + cH * 0.5f;
    float bMin = bPos.y - bH * 0.5f, bMax = bPos.y + bH * 0.5f;
    if (cMax < bMin || bMax < cMin) return false;

    /* closest point in XZ plane */
    float dx = fabsf(cPos.x - bPos.x);
    float dz = fabsf(cPos.z - bPos.z);
    float hx = bW * 0.5f, hz = bD * 0.5f;

    if (dx > hx + cR || dz > hz + cR) return false;            // too far
    if (dx <= hx || dz <= hz) return true;                     // directly beside/inside

    float cornerDistSq = (dx - hx) * (dx - hx) + (dz - hz) * (dz - hz);
    return cornerDistSq <= cR * cR;
}

/* -------------------------------------------------------------------------------------- */
int main(void)
{
    const int scrW = 1280, scrH = 720;
    InitWindow(scrW, scrH, "raylib – flat world with mixed columns");

    /* camera + player --------------------------------------------------------------- */
    Camera cam = { 0 };
    cam.position = (Vector3){ 0.0f, PLAYER_EYE_Y, 4.0f };
    cam.target = (Vector3){ 0.0f, PLAYER_EYE_Y, 0.0f };
    cam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    cam.fovy = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    int camMode = CAMERA_FIRST_PERSON;

    Vector3 playerPos = cam.position;

    /* cylinder obstacles ------------------------------------------------------------ */
    float   cylR[MAX_CYL_COLS], cylH[MAX_CYL_COLS];
    Vector3 cylPos[MAX_CYL_COLS];
    Color   cylClr[MAX_CYL_COLS];

    for (int i = 0; i < MAX_CYL_COLS; i++)
    {
        cylR[i] = (float)GetRandomValue(5, 15) / 10.0f;      // 0.5–1.5 m
        cylH[i] = (float)GetRandomValue(2, 8);             // 2–8 m
        cylPos[i] = (Vector3){ GetRandomValue(-15,15), cylH[i] * 0.5f, GetRandomValue(-15,15) };
        cylClr[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    /* box obstacles ----------------------------------------------------------------- */
    float   boxW[MAX_BOX_COLS], boxD[MAX_BOX_COLS], boxH[MAX_BOX_COLS];
    Vector3 boxPos[MAX_BOX_COLS];
    Color   boxClr[MAX_BOX_COLS];

    for (int i = 0; i < MAX_BOX_COLS; i++)
    {
        boxW[i] = (float)GetRandomValue(10, 30) / 10.0f;      // 1–3 m
        boxD[i] = (float)GetRandomValue(10, 30) / 10.0f;      // 1–3 m
        boxH[i] = (float)GetRandomValue(2, 8);              // 2–8 m
        boxPos[i] = (Vector3){ GetRandomValue(-15,15), boxH[i] * 0.5f, GetRandomValue(-15,15) };
        boxClr[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    DisableCursor();
    SetTargetFPS(60);

    /* ------------------------------- LOOP ------------------------------------------ */
    while (!WindowShouldClose())
    {
        Vector3 prevP = playerPos, prevCamPos = cam.position, prevCamTar = cam.target;

        if (IsKeyPressed(KEY_ONE))   camMode = CAMERA_FREE;
        if (IsKeyPressed(KEY_TWO))   camMode = CAMERA_FIRST_PERSON;
        if (IsKeyPressed(KEY_THREE)) camMode = CAMERA_THIRD_PERSON;

        UpdateCamera(&cam, camMode);

        if (camMode == CAMERA_FIRST_PERSON) playerPos = cam.position;
        else if (camMode == CAMERA_THIRD_PERSON)
        {
            playerPos = cam.target;
            cam.target = playerPos;
        }

        /* ---- collision tests ------------------------------------------------------ */
        bool hit = false;

        for (int i = 0; i < MAX_CYL_COLS && !hit; i++)
            if (CheckCollisionCylinders(playerPos, PLAYER_R, PLAYER_H,
                cylPos[i], cylR[i], cylH[i])) hit = true;

        for (int i = 0; i < MAX_BOX_COLS && !hit; i++)
            if (CheckCollisionCylinderBox(playerPos, PLAYER_R, PLAYER_H,
                boxPos[i], boxW[i], boxH[i], boxD[i])) hit = true;

        if (hit) { playerPos = prevP; cam.position = prevCamPos; cam.target = prevCamTar; }

        /* ------------------------- DRAW ------------------------------------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(cam);
        DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 32, 32 }, LIGHTGRAY);

        /* cylinder obstacles */
        for (int i = 0; i < MAX_CYL_COLS; i++)
        {
            DrawCylinder(cylPos[i], cylR[i], cylR[i], cylH[i], 16, cylClr[i]);
            DrawCylinderWires(cylPos[i], cylR[i], cylR[i], cylH[i], 16, MAROON);
        }

        /* box obstacles */
        for (int i = 0; i < MAX_BOX_COLS; i++)
        {
            DrawCube(boxPos[i], boxW[i], boxH[i], boxD[i], boxClr[i]);
            DrawCubeWires(boxPos[i], boxW[i], boxH[i], boxD[i], DARKBLUE);
        }

        /* player cylinder visible in 3rd‑person */
        if (camMode == CAMERA_THIRD_PERSON)
        {
            DrawCylinder(playerPos, PLAYER_R, PLAYER_R, PLAYER_H, 16, PURPLE);
            DrawCylinderWires(playerPos, PLAYER_R, PLAYER_R, PLAYER_H, 16, DARKPURPLE);
        }
        EndMode3D();

        DrawText("Modes: [1] Free  [2] First‑person  [3] Third‑person", 10, 10, 10, BLACK);
        DrawText(TextFormat("Current: %s",
            camMode == CAMERA_FREE ? "FREE" :
            camMode == CAMERA_FIRST_PERSON ? "FIRST PERSON" : "THIRD PERSON"), 10, 25, 10, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
