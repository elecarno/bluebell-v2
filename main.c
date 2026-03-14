// INCLUDES ----------------------------------------------------------------------------------------
#define CLAY_IMPLEMENTATION
#include "includes/clay.h"
#include "includes/clay_renderer_raylib.c"
#include "includes/cJSON.h"

#include <stdio.h>

// GLOBALS -----------------------------------------------------------------------------------------
const int FONT_ID_BODY = 0;
const int FONT_ID_MONO = 1;
const int FONT_SIZE_BODY = 18;

const Clay_Color COLOUR_WHITE          = { 245, 245, 245, 255 };
const Clay_Color COLOUR_BLACK          = { 15, 15, 15, 255 };
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

Clay_String utilRollingClayString(const char *text) {
    // 256 slots should be plenty for a single frame's worth of transactions
    static char buffers[256][64]; 
    static int currentBuffer = 0;

    // Increment and wrap around
    currentBuffer = (currentBuffer + 1) % 256;
    
    // Copy the text into the static slot safely
    snprintf(buffers[currentBuffer], 64, "%s", text);

    return (Clay_String) {
        .length = (int)strlen(buffers[currentBuffer]),
        .chars = buffers[currentBuffer]
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

// DATA HANDLING -----------------------------------------------------------------------------------
// money sum and whatnot


// LAYOUT ------------------------------------------------------------------------------------------
void layoutTransaction(cJSON *transaction, cJSON *accounts, cJSON *payees) {
    // GET DATA
    // id
    char *id = transaction->string;
    id[5] = '\0';

    // account
    cJSON *account_id = cJSON_GetObjectItem(transaction, "account");
    cJSON *account_obj = cJSON_GetObjectItem(accounts, account_id->valuestring);
    cJSON *account_name = cJSON_GetObjectItem(account_obj, "name");

    // payee
    cJSON *payee_id = cJSON_GetObjectItem(transaction, "payee");
    cJSON *payee_obj = cJSON_GetObjectItem(payees, payee_id->valuestring);
    cJSON *payee_name = cJSON_GetObjectItem(payee_obj, "name");

    // amount
    cJSON *amount = cJSON_GetObjectItem(transaction, "amount");
    cJSON amount_abs = *amount;
    cJSON_SetNumberValue(&amount_abs, abs(amount->valuedouble));

    // other
    cJSON *currency = cJSON_GetObjectItem(transaction, "currency");
    cJSON *timestamp = cJSON_GetObjectItem(transaction, "timestamp");
    cJSON *description = cJSON_GetObjectItem(transaction, "description");
    cJSON *tags = cJSON_GetObjectItem(transaction, "tags");

    // LAYOUT
    CLAY_AUTO_ID({
        .backgroundColor = amount->valuedouble > 0 ? COLOUR_POSITIVE : COLOUR_NEGATIVE,
        .layout = {
            .padding = { 8, 8, 8, 8},
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(32)
            },
            .childGap = 16
        }
    }) {
        CLAY_AUTO_ID({ // currency
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.05) } }
        }) {
            CLAY_TEXT(utilFixedClayString(currency->valuestring), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_MONO,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }

        CLAY_AUTO_ID({ // amount
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.1) } }
        }) {
            CLAY_TEXT(utilFixedClayString(cJSON_Print(&amount_abs)), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_MONO,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }

        CLAY_AUTO_ID({ // account
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } }
        }) {
            CLAY_TEXT(utilFixedClayString(account_name->valuestring), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_BODY,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }

        CLAY_AUTO_ID({ // payee
            .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } }
        }) {
            CLAY_TEXT(utilFixedClayString(payee_name->valuestring), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_BODY,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
            }

        CLAY_AUTO_ID({ // tags
            .layout = { .sizing = { .width = CLAY_SIZING_GROW() } }
        }) {
            CLAY_TEXT(utilFixedClayString(cJSON_Print(tags)), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_BODY,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }

        CLAY_AUTO_ID({ // timestamp
            .layout = { .sizing = { .width = CLAY_SIZING_FIXED(100) } }
        }) {
            CLAY_TEXT(utilFixedClayString(timestamp->valuestring), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_MONO,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }

        CLAY_AUTO_ID({ // id
            .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60) } }
        }) {
            CLAY_TEXT(utilFixedClayString(id), CLAY_TEXT_CONFIG({
                .fontId = FONT_ID_MONO,
                .fontSize = FONT_SIZE_BODY,
                .textColor = COLOUR_BLACK
            }));
        }
    }
}

Clay_RenderCommandArray layoutMain(cJSON *json_data) {
    cJSON *currencies = cJSON_GetObjectItem(json_data, "currencies");
    cJSON *accounts = cJSON_GetObjectItem(json_data, "accounts");
    cJSON *payees = cJSON_GetObjectItem(json_data, "payees");
    cJSON *transactions = cJSON_GetObjectItem(json_data, "transactions");

    Clay_BeginLayout(); // BEGIN LAYOUT

    CLAY(CLAY_ID("containerMain"), {
        .backgroundColor = COLOUR_BACKGROUND,
        .layout = {
            .layoutDirection = CLAY_LEFT_TO_RIGHT,
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_GROW()
            },
            .childGap = 8
        }
    }) {
        CLAY(CLAY_ID("containerSidebar"), {
            .backgroundColor = COLOUR_PANEL,
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3),
                    .height = CLAY_SIZING_GROW()
                },
                .childGap = 2
            }
        }) {

        }
        
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
            int transaction_count = cJSON_GetArraySize(transactions);
            for(int i = transaction_count-1; i >= 0; i--){
                cJSON *transaction = cJSON_GetArrayItem(transactions, i);
                layoutTransaction(transaction, accounts, payees);
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
    cJSON *json_data = ParseFileJSON("resources/data.json");

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
    Font fonts[2];
    fonts[FONT_ID_BODY] = LoadFontEx("resources/fonts/Atkinson_Hyperlegible_Next/static/AtkinsonHyperlegibleNext-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_MONO] = LoadFontEx("resources/fonts/Atkinson_Hyperlegible_Mono/static/AtkinsonHyperlegibleMono-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_MONO].texture, TEXTURE_FILTER_BILINEAR);
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
        Clay_RenderCommandArray renderCommands = layoutMain(json_data);

        // draw the ui
        BeginDrawing();
        ClearBackground(BLACK); // clear the screen each
        Clay_Raylib_Render(renderCommands, fonts);
        EndDrawing();
    }

    Clay_Raylib_Close();
}