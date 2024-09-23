/*******************************************************************************************
*
*   raylib game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2021 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include "screens.h"    // NOTE: Declares global (extern) variables and screens functions

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Shared Variables Definition (global)
// NOTE: Those variables are shared between modules through screens.h
//----------------------------------------------------------------------------------
GameScreen currentScreen = LOGO;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

//----------------------------------------------------------------------------------
// Local Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

// Required variables to manage screen transitions (fade-in, fade-out)
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static GameScreen transToScreen = UNKNOWN;

//----------------------------------------------------------------------------------
// Local Functions Declaration
//----------------------------------------------------------------------------------
static void jump_to_screen(int screen);     // Change to screen, no transition effect

static void fade_to_screen(int screen); // Request transition to next screen
static void update_transition(void);         // Update transition effect
static void draw_transition(void);           // Draw transition effect (full-screen rectangle)

static void update_draw_frame(void);          // Update and draw one frame

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib game template");

    InitAudioDevice();      // Initialize audio device

    // Load global data (assets that must be available in all screens, i.e. font)
    font = LoadFont("resources/mecha.png");
    music = LoadMusicStream("resources/ambient.ogg");
    fxCoin = LoadSound("resources/coin.wav");

    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);

    // Setup and init first screen
    currentScreen = LOGO;
    init_logo_screen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(update_draw_frame, 60, 1);
#else
    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        update_draw_frame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    // Unload current screen data before closing
    switch (currentScreen)
    {
        case LOGO: unload_logo_screen(); break;
        case TITLE: unload_title_screen(); break;
        case OPTIONS: unload_options_screen(); break;
        case GAMEPLAY: unload_gameplay_screen(); break;
        case ENDING: unload_ending_screen(); break;
        default: break;
    }

    // Unload global data loaded
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(fxCoin);

    CloseAudioDevice();     // Close audio context

    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module specific Functions Definition
//----------------------------------------------------------------------------------
// Change to next screen, no transition
static void jump_to_screen(GameScreen screen)
{
    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: unload_logo_screen(); break;
        case TITLE: unload_title_screen(); break;
        case OPTIONS: unload_options_screen(); break;
        case GAMEPLAY: unload_gameplay_screen(); break;
        case ENDING: unload_ending_screen(); break;
        default: break;
    }

    // Init next screen
    switch (screen)
    {
        case LOGO: init_logo_screen(); break;
        case TITLE: init_title_screen(); break;
        case OPTIONS: init_options_screen(); break;
        case GAMEPLAY: init_gameplay_screen(); break;
        case ENDING: init_ending_screen(); break;
        default: break;
    }

    currentScreen = screen;
}

// Request transition to next screen
static void fade_to_screen(GameScreen screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

// Update transition effect (fade-in, fade-out)
static void update_transition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;

        // NOTE: Due to float internal representation, condition jumps on 1.0f instead of 1.05f
        // For that reason we compare against 1.01f, to avoid last frame loading stop
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;

            // Unload current screen
            switch (transFromScreen)
            {
                case LOGO: unload_logo_screen(); break;
                case TITLE: unload_title_screen(); break;
                case OPTIONS: unload_options_screen(); break;
                case GAMEPLAY: unload_gameplay_screen(); break;
                case ENDING: unload_ending_screen(); break;
                default: break;
            }

            // Load next screen
            switch (transToScreen)
            {
                case LOGO: init_logo_screen(); break;
                case TITLE: init_title_screen(); break;
                case OPTIONS: init_options_screen(); break;
                case GAMEPLAY: init_gameplay_screen(); break;
                case ENDING: init_ending_screen(); break;
                default: break;
            }

            currentScreen = transToScreen;

            // Activate fade out effect to next loaded screen
            transFadeOut = true;
        }
    }
    else  // Transition fade out logic
    {
        transAlpha -= 0.02f;

        if (transAlpha < -0.01f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = UNKNOWN;
        }
    }
}

// Draw transition effect (full-screen rectangle)
static void draw_transition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

// Update and draw game frame
static void update_draw_frame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    UpdateMusicStream(music);       // NOTE: Music keeps playing between screens

    if (!onTransition)
    {
        switch(currentScreen)
        {
            case LOGO:
            {
                update_logo_screen();

                if (finish_logo_screen()) fade_to_screen(TITLE);

            } break;
            case TITLE:
            {
                update_title_screen();

                if (finish_title_screen() == 1) fade_to_screen(OPTIONS);
                else if (finish_title_screen() == 2) fade_to_screen(GAMEPLAY);

            } break;
            case OPTIONS:
            {
                update_options_screen();

                if (finish_options_screen()) fade_to_screen(TITLE);

            } break;
            case GAMEPLAY:
            {
                update_gameplay_screen();

                if (finish_gameplay_screen() == 1) fade_to_screen(ENDING);
                //else if (FinishGameplayScreen() == 2) TransitionToScreen(TITLE);

            } break;
            case ENDING:
            {
                update_ending_screen();

                if (finish_ending_screen() == 1) fade_to_screen(TITLE);

            } break;
            default: break;
        }
    }
    else update_transition();    // Update transition (fade-in, fade-out)
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        switch(currentScreen)
        {
            case LOGO: draw_logo_screen(); break;
            case TITLE: draw_title_screen(); break;
            case OPTIONS: draw_options_screen(); break;
            case GAMEPLAY: draw_gameplay_screen(); break;
            case ENDING: draw_ending_screen(); break;
            default: break;
        }

        // Draw full screen rectangle in front of everything
        if (onTransition) draw_transition();

        //DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
