/*******************************************************************************************
 *
 *   raylib [models] example - first person maze
 *
 *   This example has been created using raylib 2.5 (www.raylib.com)
 *   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
 *
 *   Copyright (c) 2019 Ramon Santamaria (@raysan5)
 *
 ********************************************************************************************/

#include "raylib.h"
#include "raymath.h"
#include <math.h>

#include <stdlib.h>           // Required for: free()
#include <stdio.h>
			      
#define RLIGHTS_IMPLEMENTATION
#include "src/rlights.h"
			      

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

int main(void)
{
	// Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 450;

	InitWindow(screenWidth, screenHeight, "raylib [models] example - first person maze");

	// Define the camera to look into our 3d world
	Camera3D camera = { 0 };
	camera.position = (Vector3){ 0.0f, 0.6f, 0.0f }; // Camera position
	camera.target = (Vector3){ 0.0f, 0.5f, 1.0f };      // Camera looking at point
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
	camera.fovy = 90.0f;                                // Camera field-of-view Y
	camera.projection = CAMERA_PERSPECTIVE;  

	Image imMap = LoadImage("resources/maze-0.png");      // Load cubicmap image (RAM)
	Texture2D cubicmap = LoadTextureFromImage(imMap);       // Convert image to texture to display (VRAM)
	Mesh mesh = GenMeshCubicmap(imMap, (Vector3){ 1.0f, 1.0f, 1.0f });
	Model model = LoadModelFromMesh(mesh);

	// NOTE: By default each cube is mapped to one part of texture atlas
	Texture2D texture = LoadTexture("resources/cubicmap_atlas.png");    // Load map texture
	model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;             // Set map diffuse texture

	// Get map image data to be used for collision detection
	Color *mapPixels = LoadImageColors(imMap);
	UnloadImage(imMap);             // Unload image from RAM

	Vector3 mapPosition = { -0.0f, 0.0f, -0.0f };  // Set model position
	Vector2 inputDirection = { 0.0f, 0.0f };						
	Vector2 playerPosition = { 1.0f, 1.0f };
	float playerTurn = 0.0f;

	//SetCameraMode(camera, CAMERA_FIRST_PERSON);     // Set camera mode

	Shader shader = LoadShader(TextFormat("resources/shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
			TextFormat("resources/shaders/glsl%i/fog.fs", GLSL_VERSION));
	shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
	shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

	// Ambient light level
	int ambientLoc = GetShaderLocation(shader, "ambient");
	SetShaderValue(shader, ambientLoc, (float[4]){ 2.0f, 2.0f, 2.0f, 1.0f }, SHADER_UNIFORM_VEC4);

	float fogDensity = 0.0f;
	int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
	SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);

	//model.materials[0].shader = shader;

	//Light light = CreateLight(LIGHT_POINT, (Vector3){ 0, 2, 6 }, Vector3Zero(), WHITE, shader);
										     //
	SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
					//--------------------------------------------------------------------------------------

	bool inputonce = false;

	// Main game loop
	while (!WindowShouldClose())    // Detect window close button or ESC key
	{
		// Update
		//----------------------------------------------------------------------------------
		UpdateCamera(&camera);      // Update camera

		// Check player collision (we simplify to 2D collision detection)

		inputDirection.x = 0.0f;
		inputDirection.y = 0.0f;

		// Задаем направление по нажатию кнопки
		if ( IsKeyDown(KEY_W) ) {
			inputDirection.x = 1.0f;
		} else if ( IsKeyDown(KEY_S) ) {
			inputDirection.x = -1.0f;
		} else if ( IsKeyDown(KEY_A) ) {
			inputDirection.y = 1.0f;
		} else if ( IsKeyDown(KEY_D) ) {
			inputDirection.y = -1.0f;
		} else {
			// Сбрасываю флаг если ничего не нажато
			inputonce = false;
		}

		// Сбрасываю все направления если флaг стоит
		if ( inputonce ) {
			inputDirection.x = 0.0f;
			inputDirection.y = 0.0f;
		}

		// Ставклю флаг если что-то нажато
		if ( inputDirection.x || inputDirection.y ) {
			inputonce = true;
		}


		// rotate
		playerTurn += PI / 2.0f * inputDirection.y;

		// move
		float newx = roundf(playerPosition.x + sinf(playerTurn) * inputDirection.x);
		float newy = roundf(playerPosition.y + cosf(playerTurn) * inputDirection.x);
		 int collider = mapPixels[(int)(newy)*cubicmap.width + (int)(newx)].r;
		 if(inputDirection.x != 0.0f) {
		printf("newx: %f, newy: %f, turn: %f \n", newx, newy, playerTurn);
		 }
		if (collider == 0) {
			playerPosition.x = newx;
			playerPosition.y = newy;
		}

		camera.position.x = playerPosition.x;
		camera.position.z = playerPosition.y;
		camera.target.x = camera.position.x + sinf(playerTurn);
		camera.target.y = camera.position.y;
		camera.target.z = camera.position.z + cosf(playerTurn);

		//light.position = camera.position;

		//----------------------------------------------------------------------------------

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(RAYWHITE);

		BeginMode3D(camera);
		DrawModel(model, mapPosition, 1.0f, WHITE);                     // Draw maze map
		EndMode3D();

		//DrawTextureEx(cubicmap, (Vector2){ GetScreenWidth() - cubicmap.width*4.0f - 20, 20.0f }, 0.0f, 4.0f, WHITE);
		//DrawRectangleLines(GetScreenWidth() - cubicmap.width*4 - 20, 20, cubicmap.width*4, cubicmap.height*4, GREEN);

		// Draw player position radar
		//DrawRectangle(GetScreenWidth() - cubicmap.width*4 - 20 + playerCellX*4, 20 + playerCellY*4, 4, 4, RED);

		DrawFPS(10, 10);

		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------
	UnloadImageColors(mapPixels);   // Unload color array

	UnloadTexture(cubicmap);        // Unload cubicmap texture
	UnloadTexture(texture);         // Unload map texture
	UnloadModel(model);             // Unload map model

	CloseWindow();                  // Close window and OpenGL context
					//--------------------------------------------------------------------------------------

	return 0;
}
