// INCLUDES ----------------------------------------------------------------------------------------
#define CLAY_IMPLEMENTATION
#include "includes/clay.h"
#include "includes/clay_renderer_raylib.c"
#include "includes/cJSON.h"

#include <stdio.h>

// GLOBALS -----------------------------------------------------------------------------------------
const int FONT_ID_BODY      = 0;
const int FONT_ID_BODY_BOLD = 1;
const int FONT_ID_MONO      = 2;
const int FONT_SIZE_BODY    = 18;
const int FONT_SIZE_TITLE   = 22;

const Clay_Color COLOUR_WHITE          = { 245, 245, 245, 255 };
const Clay_Color COLOUR_BLACK          = { 15, 15, 15, 255 };
const Clay_Color COLOUR_BACKGROUND     = { 36, 36, 36, 255 };
const Clay_Color COLOUR_PANEL          = { 56, 56, 56, 255 };
const Clay_Color COLOUR_PANEL_2        = { 76, 76, 76, 255 };
const Clay_Color COLOUR_PANEL_3        = { 96, 96, 96, 255 };
const Clay_Color COLOUR_PANEL_4        = { 116, 116, 116, 255 };

const Clay_Color COLOUR_POSITIVE_LIGHT      = { 142, 240, 115, 255 };
const Clay_Color COLOUR_POSITIVE_DARK       = { 122, 220,  95, 255 };
const Clay_Color COLOUR_NEGATIVE_LIGHT      = { 240, 115, 115, 255 };
const Clay_Color COLOUR_NEGATIVE_DARK       = { 220,  95,  95, 255 };


// STATE -------------------------------------------------------------------------------------------#
cJSON *selected_transaction = NULL;
cJSON *selected_account     = NULL;
cJSON *selected_payee       = NULL;


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

// DATA HANDLING -----------------------------------------------------------------------------------
double GetBalanceTotal(cJSON *transactions, cJSON *currencies) {
    double balance_total = 0;

    int transaction_count = cJSON_GetArraySize(transactions);
    for(int i = transaction_count-1; i >= 0; i--){
        cJSON *transaction = cJSON_GetArrayItem(transactions, i);
        cJSON *amount = cJSON_GetObjectItem(transaction, "amount");
        cJSON *currency = cJSON_GetObjectItem(transaction, "currency");

        cJSON *rates = cJSON_GetObjectItem(currencies, "rates");
        cJSON *factor = cJSON_GetObjectItem(rates, currency->valuestring);

        balance_total += (amount->valuedouble)*(factor->valuedouble);
    }

    return balance_total;
}


// BUTTON CALLBACKS--- -----------------------------------------------------------------------------
void HandleTransactionInteraction(Clay_ElementId elementId, Clay_PointerData pointerData, void *userData) {
    cJSON *transaction = (cJSON*)userData;
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_transaction = transaction;
    }
}

void HandlePayeeSet(Clay_ElementId elementId, Clay_PointerData pointerData, void *userData) {
    cJSON *payee = (cJSON*)payee;
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_payee = payee;
    }
}

void HandleAccountSet(Clay_ElementId elementId, Clay_PointerData pointerData, void *userData) {
    cJSON *account = (cJSON*)account;
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_account = account;
    }
}

void HandleTransactionClose(Clay_ElementId elementId, Clay_PointerData pointerData, void *userData) {
    if (pointerData.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_transaction = NULL;
    }
}


// LAYOUTS -----------------------------------------------------------------------------------------
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
        .backgroundColor = amount->valuedouble > 0 ? 
        (Clay_Hovered() ? COLOUR_POSITIVE_DARK : COLOUR_POSITIVE_LIGHT) : 
        (Clay_Hovered() ? COLOUR_NEGATIVE_DARK : COLOUR_NEGATIVE_LIGHT),
        .layout = {
            .padding = { 16, 16, 8, 8 },
            .sizing = {
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(32)
            },
            .childGap = 16
        }
    }) {
        if (Clay_Hovered()) { SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); }

        Clay_OnHover(HandleTransactionInteraction, transaction);

        // currency
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.05) } } }) {
            CLAY_TEXT(utilFixedClayString(currency->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // amount
        CLAY_AUTO_ID({  .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.1) } } }) {
            CLAY_TEXT(utilFixedClayString(cJSON_Print(&amount_abs)), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // account
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } } }) {
            CLAY_TEXT(utilFixedClayString(account_name->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // payee
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } } }) {
            CLAY_TEXT(utilFixedClayString(payee_name->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // tags
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_GROW() } } }) {
            CLAY_TEXT(utilFixedClayString(cJSON_Print(tags)), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // timestamp
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(100) } } }) {
            CLAY_TEXT(utilFixedClayString(timestamp->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
        // id
        CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60) } } }) {
            CLAY_TEXT(utilFixedClayString(id), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_BLACK }));
        }
    }
}

void layoutAccount(cJSON *account) {
    cJSON *account_name = cJSON_GetObjectItem(account, "name");

    CLAY_AUTO_ID({  
        .backgroundColor = Clay_Hovered() ? COLOUR_PANEL_3 : COLOUR_PANEL_2,
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(32)
            },
            .padding = { 8, 8, 8, 8 },
            .childGap = 8
        }
    }) {
        if (Clay_Hovered()) { SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); }
        Clay_OnHover(HandleAccountSet, account);
        CLAY_TEXT(utilFixedClayString(account_name->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
    }
}

void layoutPayee(cJSON *payee) {
    cJSON *payee_name = cJSON_GetObjectItem(payee, "name");

    CLAY_AUTO_ID({  
        .backgroundColor = Clay_Hovered() ? COLOUR_PANEL_3 : COLOUR_PANEL_2,
        .layout = {
            .sizing = { 
                .width = CLAY_SIZING_GROW(),
                .height = CLAY_SIZING_FIXED(32)
            },
            .padding = { 8, 8, 8, 8 },
            .childGap = 8
        }
    }) {
        if (Clay_Hovered()) { SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); }
        Clay_OnHover(HandlePayeeSet, payee);
        CLAY_TEXT(utilFixedClayString(payee_name->valuestring), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
    }
}


Clay_RenderCommandArray layoutMain(cJSON *json_data) {
    cJSON *currencies   = cJSON_GetObjectItem(json_data, "currencies");
    cJSON *accounts     = cJSON_GetObjectItem(json_data, "accounts");
    cJSON *payees       = cJSON_GetObjectItem(json_data, "payees");
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
        CLAY(CLAY_ID("containerSidebar"), { // SIDEBAR
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.3),
                    .height = CLAY_SIZING_GROW()
                },
                .childGap = 8
            }
        }) {
            CLAY(CLAY_ID("panelSettings"), { // SETTINGS
                .backgroundColor = COLOUR_PANEL,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_PERCENT(0.05)
                    },
                    .padding = { 8, 8, 8, 8 },
                    .childGap = 4
                }
            }) {
                
            }

            CLAY(CLAY_ID("panelStats"), { // STATS
                .backgroundColor = COLOUR_PANEL,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_PERCENT(0.3)
                    },
                    .padding = { 8, 8, 8, 8 },
                    .childGap = 4
                }
            }) { 
                CLAY(CLAY_ID("labelBalanceTotal")) {
                    double balance_total = GetBalanceTotal(transactions, currencies);
                    char balance_str[128];
                    cJSON *base = cJSON_GetObjectItem(currencies, "base");
                    snprintf(balance_str, sizeof(balance_str), "%.2f %s", balance_total, base->valuestring);

                    CLAY(CLAY_ID("textBalanceTotal")) {
                        CLAY_TEXT(CLAY_STRING("Total: "), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_BODY, .fontSize = FONT_SIZE_TITLE, .textColor = COLOUR_WHITE }));
                    }
                    CLAY(CLAY_ID("valueBalanceTotal")) {
                        CLAY_TEXT(utilFixedClayString(balance_str), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_TITLE, .textColor = COLOUR_WHITE }));
                    }
                }
            }

            CLAY(CLAY_ID("containerPickers"), { // PICKERS
                .layout = {
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_GROW()
                    },
                    .childGap = 4
                }
            }) {
                CLAY(CLAY_ID("containerAccountPicker"), { // ACCOUNT PICKER
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() },
                        .childGap = 4,
                    },
                }) {

                    CLAY(CLAY_ID("panelAccountList"), { // ACCOUNT LIST
                        .backgroundColor = COLOUR_PANEL,
                        .layout = {
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .sizing = {
                                .width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()
                            },
                            .padding = { 8, 8, 8, 8 },
                            .childGap = 2,
                        },
                        .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() }
                    }) {
                        int account_count = cJSON_GetArraySize(accounts);
                        for(int i = 0; i < account_count; i++){
                            cJSON *account = cJSON_GetArrayItem(accounts, i);
                            layoutAccount(account);
                        }
                    }

                    if (selected_account != NULL) { // ACCOUNT EDIT
                        CLAY(CLAY_ID("panelAccountEdit"), {
                            .backgroundColor = COLOUR_PANEL,
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(),
                                    .height = CLAY_SIZING_PERCENT(0.1)
                                },
                                .padding = { 8, 8, 8, 8 },
                                .childGap = 2,
                            },
                        }) {
                            // name edit
                            // total amount in base currency
                            // totals in each currency
                        }
                    }
                }

                CLAY(CLAY_ID("containerPayeePicker"), { // PAYEE PICKER
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = { .width = CLAY_SIZING_GROW(), .height = CLAY_SIZING_GROW() },
                        .childGap = 4,
                    },
                }) {

                    CLAY(CLAY_ID("panelPayeeList"), { // PAYEE LIST
                        .backgroundColor = COLOUR_PANEL,
                        .layout = {
                            .layoutDirection = CLAY_TOP_TO_BOTTOM,
                            .sizing = {
                                .width = CLAY_SIZING_GROW(),
                                .height = CLAY_SIZING_GROW()
                            },
                            .padding = { 8, 8, 8, 8 },
                            .childGap = 2,
                        },
                        .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() }
                    }) {
                        int payee_count = cJSON_GetArraySize(payees);
                        for(int i = 0; i < payee_count; i++){
                            cJSON *payee = cJSON_GetArrayItem(payees, i);
                            layoutPayee(payee);
                        }
                    }

                    if (selected_payee != NULL) { // PAYEE EDIT
                        CLAY(CLAY_ID("panelPayeeEdit"), {
                            .backgroundColor = COLOUR_PANEL,
                            .layout = {
                                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                                .sizing = {
                                    .width = CLAY_SIZING_GROW(),
                                    .height = CLAY_SIZING_PERCENT(0.1)
                                },
                                .padding = { 8, 8, 8, 8 },
                                .childGap = 2,
                            },
                        }) {
                            // name edit
                            // amount sent to
                            // amount recieved from
                        }
                    }
                }
            }
        }
        
        CLAY(CLAY_ID("containerTransactions"), { // TRANSACTIONS CONTAINER
            .layout = {
                .layoutDirection = CLAY_TOP_TO_BOTTOM,
                .sizing = {
                    .width = CLAY_SIZING_PERCENT(0.7),
                    .height = CLAY_SIZING_GROW()
                },
                .childGap = 4
            },
        }) {
            CLAY(CLAY_ID("panelTransactionAdd"), { // ADD TRANSACTION
                .backgroundColor = COLOUR_PANEL,
                .layout = {
                    .layoutDirection = CLAY_LEFT_TO_RIGHT,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_PERCENT(0.1)
                    },
                    .childGap = 2,
                    .padding = { 8, 8, 8, 8 }
                },
            }) {
                
            }

            CLAY(CLAY_ID("panelTransactionsList"), { // TRANSACTIONS LIST
                .backgroundColor = COLOUR_PANEL,
                .layout = {
                    .layoutDirection = CLAY_TOP_TO_BOTTOM,
                    .sizing = {
                        .width = CLAY_SIZING_GROW(),
                        .height = CLAY_SIZING_GROW()
                    },
                    .childGap = 4,
                    .padding = { 8, 8, 8, 8 }
                },
            }) {
                CLAY(CLAY_ID("containerTransactionsHeader"), {
                    .layout = {
                        .sizing = {
                            .width = CLAY_SIZING_GROW(),
                            .height = CLAY_SIZING_GROW()
                        },
                        .padding = { 16, 16, 8, 8 }
                    }
                }) {
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.07) } } }) {
                        CLAY_TEXT(CLAY_STRING("cur"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.1) } } }) {
                        CLAY_TEXT(CLAY_STRING("Amount"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } } }) {
                        CLAY_TEXT(CLAY_STRING("Account"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_PERCENT(0.15) } } }) {
                        CLAY_TEXT(CLAY_STRING("Payee"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_GROW() } } }) {
                        CLAY_TEXT(CLAY_STRING("Tags"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(100) } } }) {
                        CLAY_TEXT(CLAY_STRING("Date"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                    CLAY_AUTO_ID({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(60) } } }) {
                        CLAY_TEXT(CLAY_STRING("id"), CLAY_TEXT_CONFIG({ .fontId = FONT_ID_MONO, .fontSize = FONT_SIZE_BODY, .textColor = COLOUR_WHITE }));
                    }
                }

                CLAY(CLAY_ID("containerTransactionsList"), {
                    .layout = {
                        .layoutDirection = CLAY_TOP_TO_BOTTOM,
                        .sizing = {
                            .width = CLAY_SIZING_GROW(),
                            .height = CLAY_SIZING_GROW()
                        },
                        .childGap = 2,
                    },
                    .clip = { .vertical = true, .childOffset = Clay_GetScrollOffset() }
                }) {
                    int transaction_count = cJSON_GetArraySize(transactions);
                    for(int i = transaction_count-1; i >= 0; i--){
                        cJSON *transaction = cJSON_GetArrayItem(transactions, i);
                        layoutTransaction(transaction, accounts, payees);
                    }
                }
            }
            
            if (selected_transaction != NULL) { // SELECTED TRANSACTION INFO
                CLAY(CLAY_ID("panelTransactionSelected"), { 
                    .backgroundColor = COLOUR_PANEL,
                    .layout = {
                        .layoutDirection = CLAY_LEFT_TO_RIGHT,
                        .sizing = {
                            .width = CLAY_SIZING_GROW(),
                            .height = CLAY_SIZING_PERCENT(0.1)
                        },
                        .childGap = 2,
                        .padding = { 8, 8, 8, 8 }
                    },
                }) {
                    CLAY(CLAY_ID("buttonCloseTransaction"), {
                        .layout = { .padding = { 16, 16, 8, 8 }},
                        .backgroundColor = Clay_Hovered() ? COLOUR_PANEL_3 : COLOUR_PANEL_2,
                    }) {
                        if (Clay_Hovered()) { SetMouseCursor(MOUSE_CURSOR_POINTING_HAND); }

                        Clay_OnHover(HandleTransactionClose, NULL);

                        CLAY_TEXT(CLAY_STRING("Close"), CLAY_TEXT_CONFIG({
                            .fontId = FONT_ID_BODY,
                            .fontSize = FONT_SIZE_BODY,
                            .textColor = COLOUR_WHITE
                        }));
                    }
                }
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
    Font fonts[3];
    fonts[FONT_ID_BODY] = LoadFontEx("resources/fonts/Atkinson_Hyperlegible_Next/static/AtkinsonHyperlegibleNext-Regular.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY].texture, TEXTURE_FILTER_BILINEAR);
    fonts[FONT_ID_BODY_BOLD] = LoadFontEx("resources/fonts/Atkinson_Hyperlegible_Next/static/AtkinsonHyperlegibleNext-Bold.ttf", 48, 0, 400);
    SetTextureFilter(fonts[FONT_ID_BODY_BOLD].texture, TEXTURE_FILTER_BILINEAR);
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
            false,
            (Clay_Vector2) { scrollDelta.x * 4, scrollDelta.y * 4 },
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