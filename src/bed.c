#include "raylib.h"
#include "rcamera.h"
#include "raymath.h"

#define MAX_CYL_COLS   12
#define MAX_BOX_COLS   12
#define PLAYER_WIDTH   0.5f
#define PLAYER_HEIGHT  1.0f
#define PLAYER_DEPTH   0.5f
#define PLAYER_EYE_Y   (PLAYER_HEIGHT*0.5f)

static BoundingBox MakeCubeBox(Vector3 centre, float w, float h, float d)
{
    return (BoundingBox) {
        { centre.x - w * 0.5f, centre.y - h * 0.5f, centre.z - d * 0.5f },
        { centre.x + w * 0.5f, centre.y + h * 0.5f, centre.z + d * 0.5f }
    };
}

/* -------------------------------------------------------------------------------------- */
int main(void)
{
    const int screenW = 1280, screenH = 720;
    InitWindow(screenW, screenH, "raylib – FINAL: scaled models + perfect bounding boxes");

    /* camera + player setup ---------------------------------------------------------- */
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, PLAYER_EYE_Y, 4.0f };
    camera.target = (Vector3){ 0.0f, PLAYER_EYE_Y, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    int cameraMode = CAMERA_FIRST_PERSON;

    Vector3 playerPos = camera.target;
    Vector3 prevPlayerPos, prevCamPos, prevCamTarget;

    /* Load human model */
    Model humanModel = LoadModel("Resources/human.obj");
    float humanScale = 0.1f;    // smaller

    /* Load bed model */
    Model bedModel = LoadModel("Resources/bed_fixed.obj");
    float bedScale = 1.5f;        // bigger
    Vector3 bedPos = (Vector3){ 5.0f, 0.5f * bedScale, 5.0f };  // center based on bed scale

    /* cylinder obstacles */
    float cylR[MAX_CYL_COLS], cylH[MAX_CYL_COLS];
    Vector3 cylPos[MAX_CYL_COLS];
    Color cylClr[MAX_CYL_COLS];

    for (int i = 0; i < MAX_CYL_COLS; i++)
    {
        cylR[i] = (float)GetRandomValue(5, 15) / 10.0f;
        cylH[i] = (float)GetRandomValue(2, 8);
        cylPos[i] = (Vector3){
            GetRandomValue(-15,15),
            cylH[i] * 0.5f,
            GetRandomValue(-15,15)
        };
        cylClr[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    /* box obstacles */
    float boxW[MAX_BOX_COLS], boxD[MAX_BOX_COLS], boxH[MAX_BOX_COLS];
    Vector3 boxPos[MAX_BOX_COLS];
    Color boxClr[MAX_BOX_COLS];

    for (int i = 0; i < MAX_BOX_COLS; i++)
    {
        boxW[i] = (float)GetRandomValue(10, 30) / 10.0f;
        boxD[i] = (float)GetRandomValue(10, 30) / 10.0f;
        boxH[i] = (float)GetRandomValue(2, 8);
        boxPos[i] = (Vector3){
            GetRandomValue(-15,15),
            boxH[i] * 0.5f,
            GetRandomValue(-15,15)
        };
        boxClr[i] = (Color){ GetRandomValue(20,255), GetRandomValue(10,55), 30, 255 };
    }

    DisableCursor();
    SetTargetFPS(60);

    /* ------------------------------------- MAIN LOOP --------------------------------- */
    while (!WindowShouldClose())
    {
        prevPlayerPos = playerPos;
        prevCamPos = camera.position;
        prevCamTarget = camera.target;

        if (IsKeyPressed(KEY_ONE)) cameraMode = CAMERA_FREE;
        if (IsKeyPressed(KEY_TWO)) cameraMode = CAMERA_FIRST_PERSON;
        if (IsKeyPressed(KEY_THREE)) cameraMode = CAMERA_THIRD_PERSON;

        UpdateCamera(&camera, cameraMode);

        if (cameraMode == CAMERA_FIRST_PERSON)
        {
            playerPos = camera.position;
        }
        else if (cameraMode == CAMERA_THIRD_PERSON)
        {
            playerPos = camera.target;
            camera.target = playerPos;
        }

        /* --- collision check --- */
        bool hit = false;

        /* Make bounding box for player */
        BoundingBox playerBox = MakeCubeBox(playerPos,
            PLAYER_WIDTH * humanScale,
            PLAYER_HEIGHT * humanScale,
            PLAYER_DEPTH * humanScale
        );


        /* Check cylinders */
        for (int i = 0; i < MAX_CYL_COLS && !hit; i++)
        {
            BoundingBox cylBox = MakeCubeBox(cylPos[i], cylR[i] * 2.0f, cylH[i], cylR[i] * 2.0f);
            if (CheckCollisionBoxes(playerBox, cylBox)) hit = true;
        }

        /* Check boxes */
        for (int i = 0; i < MAX_BOX_COLS && !hit; i++)
        {
            BoundingBox box = MakeCubeBox(boxPos[i], boxW[i], boxH[i], boxD[i]);
            if (CheckCollisionBoxes(playerBox, box)) hit = true;
        }

        /* Check bed model using real bounding box */
        BoundingBox bedBox = GetMeshBoundingBox(bedModel.meshes[0]);
        bedBox.min = Vector3Scale(bedBox.min, bedScale);
        bedBox.max = Vector3Scale(bedBox.max, bedScale);
        bedBox.min = Vector3Add(bedBox.min, bedPos);
        bedBox.max = Vector3Add(bedBox.max, bedPos);

        if (!hit && CheckCollisionBoxes(playerBox, bedBox)) hit = true;

        if (hit)
        {
            playerPos = prevPlayerPos;
            camera.position = prevCamPos;
            camera.target = prevCamTarget;
        }

        /* ---------------- DRAW ---------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);
        DrawPlane((Vector3) { 0, 0, 0 }, (Vector2) { 32, 32 }, LIGHTGRAY);

        for (int i = 0; i < MAX_CYL_COLS; i++)
        {
            DrawCylinder(cylPos[i], cylR[i], cylR[i], cylH[i], 16, cylClr[i]);
            DrawCylinderWires(cylPos[i], cylR[i], cylR[i], cylH[i], 16, MAROON);
        }

        for (int i = 0; i < MAX_BOX_COLS; i++)
        {
            DrawCube(boxPos[i], boxW[i], boxH[i], boxD[i], boxClr[i]);
            DrawCubeWires(boxPos[i], boxW[i], boxH[i], boxD[i], DARKBLUE);
        }

        /* draw the bed */
        DrawModel(bedModel, bedPos, bedScale, WHITE);
        DrawModelWires(bedModel, bedPos, bedScale, DARKPURPLE);

        /* draw the human */
        if (cameraMode == CAMERA_THIRD_PERSON)
        {
            DrawModel(humanModel, playerPos, humanScale, WHITE);
            DrawModelWires(humanModel, playerPos, humanScale, DARKPURPLE);
        }
        EndMode3D();

        DrawText("Modes: [1] Free  [2] First‑person  [3] Third‑person", 10, 10, 10, BLACK);
        DrawText(TextFormat("Current: %s",
            cameraMode == CAMERA_FREE ? "FREE" :
            cameraMode == CAMERA_FIRST_PERSON ? "FIRST PERSON" : "THIRD PERSON"), 10, 25, 10, BLACK);
        EndDrawing();
    }

    UnloadModel(humanModel);
    UnloadModel(bedModel);
    CloseWindow();
    return 0;
}
