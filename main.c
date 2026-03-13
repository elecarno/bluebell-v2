// INCLUDES ----------------------------------------------------------------------------------------
#define CLAY_IMPLEMENTATION
#include "includes/clay.h"
#include "includes/clay_renderer_raylib.c"
#include "includes/cJSON.h"

#include <stdio.h>

// GLOBALS -----------------------------------------------------------------------------------------
const int FONT_ID_BODY_16 = 0;

const Clay_Color COLOUR_WHITE          = { 245, 245, 245, 255 };
const Clay_Color COLOUR_BACKGROUND     = { 36, 36, 36, 255 };
const Clay_Color COLOUR_PANEL          = { 56, 56, 56, 255 };
const Clay_Color COLOUR_PANEL2         = { 76, 76, 76, 255 };

const Clay_Color COLOUR_POSITIVE       = { 142, 240, 115, 255 };
const Clay_Color COLOUR_NEGATIVE       = { 240, 115, 115, 255 };

// utilities
Clay_String utilFixedClayString(char *text) {
    return (Clay_String) {
        .length = (int)strlen(text),
        .chars = text
    };
}


// JSON PARSER -------------------------------------------------------------------------------------
cJSON* ParseFileJSON(char filepath[]) {
    // open json file
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL) {
        printf("Error: Unable to open the file.\n");
    }

    // determine file size
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // allocate memory (+1 for the null terminator)
    char *data = malloc(length + 1);
    if (data == NULL) {
        printf("Error: Out of memory.\n");
        fclose(fp);
    }

    // read into dynamic buffer
    size_t read_size = fread(data, 1, length, fp);
    data[read_size] = '\0'; // Null-terminate the string
    fclose(fp);

    // parse JSON data
    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
    }

    free(data);
    return json;
}


// LAYOUT ------------------------------------------------------------------------------------------
void layoutTransaction(cJSON *transaction) {
    cJSON *account = cJSON_GetObjectItem(transaction, "account");
    cJSON *payee = cJSON_GetObjectItem(transaction, "payee");
    cJSON *currency = cJSON_GetObjectItem(transaction, "currency");
        // amount
        // tags
    cJSON *timestamp = cJSON_GetObjectItem(transaction, "timestamp");

    CLAY_AUTO_ID({
        .backgroundColor = COLOUR_PANEL2,
        .layout = {
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(45)
            },
            .childGap = 16
        }
    }) {
        CLAY_TEXT(utilFixedClayString(currency->valuestring), CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = 16,
            .textColor = COLOUR_WHITE
        }));

        CLAY_TEXT(utilFixedClayString(account->valuestring), CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = 16,
            .textColor = COLOUR_WHITE
        }));

        CLAY_TEXT(utilFixedClayString(payee->valuestring), CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = 16,
            .textColor = COLOUR_WHITE
        }));

        // amount

        CLAY_TEXT(utilFixedClayString(timestamp->valuestring), CLAY_TEXT_CONFIG({
            .fontId = FONT_ID_BODY_16,
            .fontSize = 16,
            .textColor = COLOUR_WHITE
        }));
    }
}

Clay_RenderCommandArray layoutMain(cJSON *json_transactions) {
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
        CLAY(CLAY_ID("containerTransactions"), {
            .backgroundColor = COLOUR_PANEL,
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.7),
                    .height = CLAY_SIZING_GROW()
                },
                .childGap = 2
            }
        }) {
            int transaction_count = cJSON_GetArraySize(json_transactions);
            for(int i = 0; i < transaction_count; i++){
                cJSON *transaction = cJSON_GetArrayItem(json_transactions, i);
                layoutTransaction(transaction);
            }
        }
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
    cJSON *json_transactions = ParseFileJSON("resources/data/transactions.json");

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
        Clay_RenderCommandArray renderCommands = layoutMain(json_transactions);

        // draw the ui
        BeginDrawing();
        ClearBackground(BLACK); // clear the screen each
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    Clay_Raylib_Close();
}