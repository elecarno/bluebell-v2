// INCLUDES ----------------------------------------------------------------------------------------
#define CLAY_IMPLEMENTATION
#include "includes/clay.h"
#include "includes/clay_renderer_raylib.c"

// GLOBALS -----------------------------------------------------------------------------------------
const int FONT_ID_BODY_16 = 0;

const Clay_Color COLOUR_WHITE          = { 245, 245, 245, 255 };
const Clay_Color COLOUR_BACKGROUND     = { 36, 36, 36, 255 };


// LAYOUT ------------------------------------------------------------------------------------------
Clay_RenderCommandArray layoutMain() {
    Clay_BeginLayout(); // BEGIN LAYOUT

    CLAY(CLAY_ID("containerMain"), {
        .backgroundColor = COLOUR_BACKGROUND,
        .layout = {
            .layoutDirection = CLAY_TOP_TO_BOTTOM,
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW()
            },
            .childGap = 8
        }
    }) {

    }

    // END LAYOUT
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    return renderCommands;
}


// RUN APP -----------------------------------------------------------------------------------------
void HandleClayErrors(Clay_ErrorData errorData) {
    printf("%s", errorData.errorText.chars);
}

int main(void) {
    Clay_Raylib_Initialize(
        1280, 720, // width and height
        "Bluebell v2.0.0", // window title
        FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT //flags
    );

    // get memory allocation
    uint64_t clayRequiredMemory = Clay_MinMemorySize();
    Clay_Arena clayMemory = Clay_CreateArenaWithCapacityAndMemory(
        clayRequiredMemory, 
        malloc(clayRequiredMemory)
    );

    // initalise clay using memory information
    Clay_Initialize(clayMemory, (Clay_Dimensions) {
       .width = GetScreenWidth(),
       .height = GetScreenHeight()
    }, (Clay_ErrorHandler) { HandleClayErrors });

    // handle fonts
    Font fonts[1];
    fonts[FONT_ID_BODY_16] = LoadFontEx("resources/fonts/Atkinson_Hyperlegible_Next/static/AtkinsonHyperlegibleNext-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_16].texture, TEXTURE_FILTER_BILINEAR);
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    // update loop (per frame)
    while (!WindowShouldClose()) {
        Vector2 dpiScale = GetWindowScaleDPI();

        // update dimensions every frame to handle resizing
        Clay_SetLayoutDimensions((Clay_Dimensions) {
            .width = (float)GetRenderWidth() / dpiScale.x,
            .height = (float)GetRenderHeight() / dpiScale.y
        });

        // handle input
        Vector2 mousePosition = GetMousePosition();
        Vector2 scrollDelta = GetMouseWheelMoveV();
        Clay_SetPointerState(
            (Clay_Vector2) { mousePosition.x, mousePosition.y },
            IsMouseButtonDown(0)
        );
        Clay_UpdateScrollContainers(
            true,
            (Clay_Vector2) { scrollDelta.x, scrollDelta.y },
            GetFrameTime()
        );

        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        // get layout render commands for the ui
        Clay_RenderCommandArray renderCommands = layoutMain();

        // draw the ui
        BeginDrawing();
        ClearBackground(BLACK); // clear the screen each
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    Clay_Raylib_Close();
}