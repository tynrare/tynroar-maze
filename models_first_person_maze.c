/*******************************************************************************************
 *
 *   raylib [models] example - first person maze
 *
 *   This example has been created using raylib 2.5 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h
 *for details)
 *
 *   Copyright (c) 2019 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <math.h>

#include <stdio.h>
#include <stdlib.h> // Required for: free()

#define RLIGHTS_IMPLEMENTATION
#include "src/rlights.h"

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

int min(int a, int b) { return a > b ? b : a; }

float lerp(float a, float b, float t) { return a + (b - a) * t; }

float rlerp(float a, float b, float t) {
  float CS = (1.0f - t) * cosf(a) + t * cosf(b);
  float SN = (1.0f - t) * sinf(a) + t * sinf(b);

  return atan2f(SN, CS);
}

bool isAnyKeyPressed(int count, ...) {
	bool pressed = false;

	va_list args;
	va_start(args, count);

	for (int i = 0; i < count; i++) {
		if ( IsKeyDown (va_arg(args, int)) ) {
			pressed = true;
		}
	}

	va_end(args);

	return pressed;
}

int main(void) {
  // Initialization
  //--------------------------------------------------------------------------------------
  const int screenWidth = 800;
  const int screenHeight = 450;

  InitWindow(screenWidth, screenHeight,
             "raylib [models] example - first person maze");

  // Define the camera to look into our 3d world
  Camera3D camera = {0};
  camera.position = (Vector3){0.0f, 0.6f, 0.0f}; // Camera position
  camera.target = (Vector3){0.0f, 0.5f, 1.0f};   // Camera looking at point
  camera.up =
      (Vector3){0.0f, 1.0f, 0.0f}; // Camera up vector (rotation towards target)
  camera.fovy = 90.0f;             // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;

  Image imMap = LoadImage("resources/maze-0.png"); // Load cubicmap image (RAM)
  Texture2D cubicmap =
      LoadTextureFromImage(imMap); // Convert image to texture to display (VRAM)
  Mesh mesh = GenMeshCubicmap(imMap, (Vector3){1.0f, 1.0f, 1.0f});
  Model model = LoadModelFromMesh(mesh);

  // NOTE: By default each cube is mapped to one part of texture atlas
  Texture2D texture =
      LoadTexture("resources/cubicmap_atlas.png"); // Load map texture
  model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture =
      texture; // Set map diffuse texture

  // Get map image data to be used for collision detection
  Color *mapPixels = LoadImageColors(imMap);
  UnloadImage(imMap); // Unload image from RAM

  // ---
  // game data
  Vector3 mapPosition = {-0.0f, 0.0f, -0.0f}; // Set model position
  Vector2 inputDirection = {0.0f, 0.0f};
  Vector2 playerPosition = {1.0f, 1.0f};
  float playerTurn = 0.0f;
  float cameraRot = 0.0f;
  int steps = 0;

  // SetCameraMode(camera, CAMERA_FIRST_PERSON);     // Set camera mode

  /*
  Shader shader =
  LoadShader(TextFormat("resources/shaders/glsl%i/base_lighting.vs",
  GLSL_VERSION), TextFormat("resources/shaders/glsl%i/fog.fs", GLSL_VERSION));
  shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
  shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

  // Ambient light level
  int ambientLoc = GetShaderLocation(shader, "ambient");
  SetShaderValue(shader, ambientLoc, (float[4]){ 2.0f, 2.0f, 2.0f, 1.0f },
  SHADER_UNIFORM_VEC4);

  float fogDensity = 0.0f;
  int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
  SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
  */

  // model.materials[0].shader = shader;

  // Light light = CreateLight(LIGHT_POINT, (Vector3){ 0, 2, 6 }, Vector3Zero(),
  // WHITE, shader);
  //
  SetTargetFPS(
      60); // Set our game to run at 60 frames-per-second
           //--------------------------------------------------------------------------------------

  bool inputonce = true;

  // ---- navigation_system >>

  // Texture2D bills_textures = LoadTexture("resources/billboard.png");     //
  // Our texture billboard

  int tagsLimit = 10;
  int tagsCount = 0;
  int tagPositions[tagsLimit][2];
  int tagIndex = 0;
  //
  // ---- navigation_system <<

  // Main game loop
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    // Update
    //----------------------------------------------------------------------------------
    // UpdateCamera(&camera, 0);      // Update camera

    // Check player collision (we simplify to 2D collision detection)

    inputDirection.x = 0.0f;
    inputDirection.y = 0.0f;
    bool inputed = false;

		if ( isAnyKeyPressed (5, KEY_W, KEY_S, KEY_A, KEY_D, KEY_E) ) {
			inputed = true;
		} else {
			inputonce = true;
		}

    // Задаем направление по нажатию кнопки
		if (inputonce) {
      if (IsKeyDown(KEY_W)) {
        inputDirection.x = 1.0f;
      } else if (IsKeyDown(KEY_S)) {
        inputDirection.x = -1.0f;
      } else if (IsKeyDown(KEY_A)) {
        inputDirection.y = 1.0f;
      } else if (IsKeyDown(KEY_D)) {
        inputDirection.y = -1.0f;
      } else if (IsKeyDown(KEY_E)) {
        size_t length = sizeof(tagPositions) / sizeof(int);
        int index = tagIndex++ % tagsLimit;
        tagPositions[index][0] = (int)(playerPosition.x);
        tagPositions[index][1] = (int)(playerPosition.y);
        tagsCount += 1;
      }
		}

		if (inputed) {
			inputonce = false;
		}

    if (inputDirection.x) {
      steps += 1;
    }

    // rotate
    playerTurn += PI / 2.0f * inputDirection.y;

    // move
    float newx = roundf(playerPosition.x + sinf(playerTurn) * inputDirection.x);
    float newy = roundf(playerPosition.y + cosf(playerTurn) * inputDirection.x);
    int collider = mapPixels[(int)(newy)*cubicmap.width + (int)(newx)].r;
    /*
    if(inputDirection.x != 0.0f) {
            printf("newx: %f, newy: %f, turn: %f \n", newx, newy, playerTurn);
    }
    */
    if (collider == 0) {
      playerPosition.x = newx;
      playerPosition.y = newy;
    }

    cameraRot = rlerp(cameraRot, playerTurn, 0.5);

    camera.position.x = lerp(camera.position.x, playerPosition.x, 0.5);
    camera.position.z = lerp(camera.position.z, playerPosition.y, 0.5);
    camera.target.x = camera.position.x + sinf(cameraRot);
    camera.target.y = camera.position.y;
    camera.target.z = camera.position.z + cosf(cameraRot);

    // light.position = camera.position;

    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

    ClearBackground(BLANK);

    BeginMode3D(camera);
    DrawModel(model, mapPosition, 1.0f, WHITE); // Draw maze map
    EndMode3D();

    // DrawTextureEx(cubicmap, (Vector2){ GetScreenWidth() -
    // cubicmap.width*4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);
    // DrawRectangleLines(GetScreenWidth() - cubicmap.width*4 - 20, 20,
    // cubicmap.width*4, cubicmap.height*4, GREEN);

    // Draw player position radar
    // DrawRectangle(GetScreenWidth() - cubicmap.width*4 - 20 + playerCellX*4,
    // 20 + playerCellY*4, 4, 4, RED);

    DrawFPS(10, 10);
    DrawText(TextFormat("Steps: %i", steps), 10, 50, 10, BLACK);

    // ---- navigation_system >>
    //
    for (int i = 0; i < min(tagsCount, tagsLimit); i++) {
        DrawCube((Vector3){(float)(tagPositions[i][0]), 0.5f, (float)(tagPositions[i][1])}, 0.5f, 0.5f, 0.5f, RED);
        DrawText(
                TextFormat(
                    "#%i: %ix%i", i + tagsCount, tagPositions[i][0],
                    tagPositions[i][1]
                ), 
                10, 80 + i * 14, 12, BLACK);
    }
    //
    // ---- navigation_system <<

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  // De-Initialization
  //--------------------------------------------------------------------------------------
  UnloadImageColors(mapPixels); // Unload color array

  UnloadTexture(cubicmap); // Unload cubicmap texture
  UnloadTexture(texture);  // Unload map texture
  UnloadModel(model);      // Unload map model

  CloseWindow(); // Close window and OpenGL context
                 //--------------------------------------------------------------------------------------

  return 0;
}
