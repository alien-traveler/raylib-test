/*******************************************************************************************
*   raylib example – First‑/Third‑person player CYLINDER with solid collisions
*   Build (MSVC):  cl /EHsc main.cpp /I%RAYLIB%\include /link %RAYLIB%\raylib.lib opengl32.lib gdi32.lib winmm.lib
*   Build (MinGW): g++ main.cpp -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm
********************************************************************************************/

#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"

#define MAX_COLS        20
#define PLAYER_RADIUS   0.5f
#define PLAYER_HEIGHT   1.0f
#define PLAYER_EYE_Y    (PLAYER_HEIGHT*0.5f)   // camera Y inside cylinder

/* Simple vertical‑axis cylinder–cylinder collision */
static bool CheckCollisionCylinders(Vector3 aPos, float aR, float aH,
    Vector3 bPos, float bR, float bH)
{
    /* Horizontal distance in XZ plane */
    float dx = aPos.x - bPos.x;
    float dz = aPos.z - bPos.z;
    if (sqrtf(dx * dx + dz * dz) > (aR + bR)) return false;

    /* Vertical overlap (centres stored at mid‑height) */
    float aMin = aPos.y - aH * 0.5f, aMax = aPos.y + aH * 0.5f;
    float bMin = bPos.y - bH * 0.5f, bMax = bPos.y + bH * 0.5f;
    if (aMax < bMin || bMax < aMin) return false;

    return true;
}

/* -------------------------------------------------------------------------------------- */
int main(void)
{
    const int screenW = 1280, screenH = 720;
    InitWindow(screenW, screenH, "raylib – cylinder player with collisions");

    /* ----- Camera & player ------------------------------------------------------------ */
    Camera cam = { 0 };
    cam.position = (Vector3){ 0.0f, PLAYER_EYE_Y, 4.0f };
    cam.target = (Vector3){ 0.0f, PLAYER_EYE_Y, 0.0f };
    cam.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    cam.fovy = 60.0f;
    cam.projection = CAMERA_PERSPECTIVE;
    int camMode = CAMERA_FIRST_PERSON;

    Vector3 playerPos = cam.position;          // centre of cylinder

    /* ----- World cylinders ------------------------------------------------------------ */
    float    colH[MAX_COLS];
    float    colR[MAX_COLS];
    Vector3  colPos[MAX_COLS];
    Color    colClr[MAX_COLS];

    for (int i = 0; i < MAX_COLS; i++)
    {
        colH[i] = (float)GetRandomValue(2, 8);          // 2–8 m tall
        colR[i] = (float)GetRandomValue(1, 3) * 0.5f;      // 0.5–1.5 m radius
        colPos[i] = (Vector3){ GetRandomValue(-15, 15),
                               colH[i] * 0.5f,
                               GetRandomValue(-15, 15) };
        colClr[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    DisableCursor();
    SetTargetFPS(60);

    /* ------------------------------- GAME LOOP ---------------------------------------- */
    while (!WindowShouldClose())
    {
        /* Save state for rollback */
        Vector3 prevPlayer = playerPos;
        Vector3 prevCamPos = cam.position;
        Vector3 prevCamTar = cam.target;

        /* Mode hotkeys */
        if (IsKeyPressed(KEY_ONE))   camMode = CAMERA_FREE;
        if (IsKeyPressed(KEY_TWO))   camMode = CAMERA_FIRST_PERSON;
        if (IsKeyPressed(KEY_THREE)) camMode = CAMERA_THIRD_PERSON;

        /* Update camera via helper */
        UpdateCamera(&cam, camMode);

        /* Sync player ↔ camera */
        if (camMode == CAMERA_FIRST_PERSON)
            playerPos = cam.position;
        else if (camMode == CAMERA_THIRD_PERSON)
        {
            playerPos = cam.target;
            cam.target = playerPos;            // keep camera aimed at player
        }

        /* Collision test against every world cylinder */
        bool hit = false;
        for (int i = 0; i < MAX_COLS && !hit; i++)
            if (CheckCollisionCylinders(playerPos, PLAYER_RADIUS, PLAYER_HEIGHT,
                colPos[i], colR[i], colH[i]))
                hit = true;

        if (hit)          // rollback on hit
        {
            playerPos = prevPlayer;
            cam.position = prevCamPos;
            cam.target = prevCamTar;
        }

        /* --------------------------------- DRAW -------------------------------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(cam);
        DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 32, 32 }, LIGHTGRAY);

        /* world columns */
        for (int i = 0; i < MAX_COLS; i++)
        {
            DrawCylinder(colPos[i], colR[i], colR[i], colH[i], 16, colClr[i]);
            DrawCylinderWires(colPos[i], colR[i], colR[i], colH[i], 16, MAROON);
        }

        /* player cylinder (visible only in 3rd‑person) */
        if (camMode == CAMERA_THIRD_PERSON)
        {
            DrawCylinder(playerPos, PLAYER_RADIUS, PLAYER_RADIUS,
                PLAYER_HEIGHT, 16, PURPLE);
            DrawCylinderWires(playerPos, PLAYER_RADIUS, PLAYER_RADIUS,
                PLAYER_HEIGHT, 16, DARKPURPLE);
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
