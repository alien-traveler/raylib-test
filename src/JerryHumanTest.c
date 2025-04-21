/*******************************************************************************************
*   raylib example – First‑/Third‑person player cube with solid collisions
*   Build (MSVC):   cl /EHsc main.cpp /I%RAYLIB%\include /link %RAYLIB%\raylib.lib opengl32.lib gdi32.lib winmm.lib
*   Build (MinGW):  g++ main.cpp -o game.exe -lraylib -lopengl32 -lgdi32 -lwinmm
********************************************************************************************/

#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"

#define MAX_COLUMNS   20
#define PLAYER_SIZE   1.0f            // Cube side length (1×1×1)
#define PLAYER_EYE_Y  (PLAYER_SIZE*0.5f)

static BoundingBox MakeCubeBox(Vector3 centre, float w, float h, float d)
{
    return (BoundingBox) {
        { centre.x - w * 0.5f, centre.y - h * 0.5f, centre.z - d * 0.5f },
        { centre.x + w * 0.5f, centre.y + h * 0.5f, centre.z + d * 0.5f }
    };
}

int main(void)
{
    const int screenW = 1920, screenH = 1080;
    InitWindow(screenW, screenH, "raylib – player cube with collisions");

    /* --- Camera & player -------------------------------------------------------------- */
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, PLAYER_EYE_Y, 4.0f };
    camera.target = (Vector3){ 0.0f, PLAYER_EYE_Y, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int cameraMode = CAMERA_FIRST_PERSON;       // Start in FPS view
    Vector3 playerPos = camera.position;        // Player cube centre
    Vector3 prevPlayerPos, prevCamPos, prevCamTarget;

    /* --- World geometry (random columns) --------------------------------------------- */
    float    colHeights[MAX_COLUMNS];
    Vector3  colPos[MAX_COLUMNS];
    Color    colColor[MAX_COLUMNS];

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        colHeights[i] = (float)GetRandomValue(1, 12);
        colPos[i] = (Vector3){ GetRandomValue(-15, 15), colHeights[i] * 0.5f, GetRandomValue(-15, 15) };
        colColor[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    DisableCursor();
    SetTargetFPS(60);
    
    // TRY TO IMPORT MODEL HERE
    Model model = LoadModel("Resources/human.obj");
    //bool valid = isModelValid(model);
    //printf("The value of valid is: %s\n", valid ? "true" : "false");


    /* ------------------------------ GAME LOOP ----------------------------------------- */
    while (!WindowShouldClose())
    {
        /* --- Save state for collision rollback --------------------------------------- */
        prevPlayerPos = playerPos;
        prevCamPos = camera.position;
        prevCamTarget = camera.target;

        /* --- Mode switching ---------------------------------------------------------- */
        if (IsKeyPressed(KEY_ONE))   cameraMode = CAMERA_FREE;
        if (IsKeyPressed(KEY_TWO))   cameraMode = CAMERA_FIRST_PERSON;
        if (IsKeyPressed(KEY_THREE)) cameraMode = CAMERA_THIRD_PERSON;

        /* --- Update camera via raylib helper ---------------------------------------- */
        UpdateCamera(&camera, cameraMode);

        /* --- Sync player ↔ camera --------------------------------------------------- */
        if (cameraMode == CAMERA_FIRST_PERSON)
        {
            playerPos = camera.position;                     // cube is invisible but collides
        }
        else if (cameraMode == CAMERA_THIRD_PERSON)
        {
            playerPos = camera.target;                       // cube visible at target
            camera.target = playerPos;                           // keep looking at player
        }

        /* --- Collision test --------------------------------------------------------- */
        BoundingBox playerBox = MakeCubeBox(playerPos, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE);

        bool hit = false;
        for (int i = 0; i < MAX_COLUMNS && !hit; i++)
        {
            BoundingBox colBox = MakeCubeBox(colPos[i], 2.0f, colHeights[i], 2.0f);
            if (CheckCollisionBoxes(playerBox, colBox)) hit = true;
        }

        if (hit)
        {
            /* Roll back everything */
            playerPos = prevPlayerPos;
            camera.position = prevCamPos;
            camera.target = prevCamTarget;
        }

        /* --- DRAW ------------------------------------------------------------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 32, 32 }, LIGHTGRAY);

        /* World columns */
        for (int i = 0; i < MAX_COLUMNS; i++)
        {
            DrawCube(colPos[i], 2, colHeights[i], 2, colColor[i]);
            DrawCubeWires(colPos[i], 2, colHeights[i], 2, MAROON);
        }

        /* Player cube (only in 3rd‑person) */
        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            //DrawCube(playerPos, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, PURPLE);
            //DrawCubeWires(playerPos, PLAYER_SIZE, PLAYER_SIZE, PLAYER_SIZE, DARKPURPLE);
			DrawModel(model, playerPos, 0.1f, WHITE);
			DrawModelWires(model, playerPos, 0.1f, DARKPURPLE);
        }
        EndMode3D();

        DrawText("Modes: [1] Free  [2] First‑person  [3] Third‑person", 10, 10, 10, BLACK);
        DrawText(TextFormat("Current: %s",
            cameraMode == CAMERA_FREE ? "FREE" :
            cameraMode == CAMERA_FIRST_PERSON ? "FIRST PERSON" : "THIRD PERSON"), 10, 25, 10, BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
