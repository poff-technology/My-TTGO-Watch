#include "test.h"

#include "HTTPClient.h"
#include "config.h"
#include "gui/mainbar/mainbar.h"
#include "gui/widget_styles.h"
#include "gui/widget_factory.h"
#include "hardware/wifictl.h"
#include "quickglui/quickglui.h"

#define BASEMENT_LIGHT "Hue white lamp 2"
#define KITCHEN_SINK "Kitchen Sink"
#define LIGHTS "Kitchen Sink\nHue white lamp 2\0"

#define ROLLER_ROW_COUNT 2

// App icon must have an size of 64x64 pixel with an alpha channel
// Use https://lvgl.io/tools/imageconverter to convert your images and set "true color with alpha"
LV_IMG_DECLARE(robot_64px);
LV_FONT_DECLARE(Ubuntu_32px);

static Application testApp;
static JsonConfig config("test.json");

static String host, port, apiKey;

static String updatedAt;
static Label lblStatus, lblUpdatedAt;
static Button btnToggleLight;
static lv_obj_t *hour_roller = NULL;

static Style big;

/*
 * setup routine for application
 */
void test_app_setup() {
#if defined(ONLY_ESSENTIAL)
    return;
#endif
    // Create and register new application
    //   params: name, icon, auto add "refresh" button (this app will use synchronize function of the SynchronizedApplication class).
    //   Also, you can configure count of the required pages in the next two params (to have more app screens).
    testApp.init("test", &robot_64px, 1, 1);

    // Build and configure application
    test_build_main_page();
    test_build_settings();
}

void test_publish() {
    int httpcode = -1;
    char value[32] = "";

    lv_roller_get_selected_str(hour_roller, value, sizeof(value));

    char uri[64] = "";
    snprintf(uri, sizeof(uri), R"(http://%s:%s/v1/light/operate)", host.c_str(), port.c_str());

    char payload[256] = "";
    snprintf(payload, sizeof(payload), R"({"light_name": "%s", "light_action": "%s"})", value, "TOGGLE");
    
    log_i("Uri: ->  %s", uri);
    log_i("Payload: ->  %s", payload);

    HTTPClient http_client;
    http_client.useHTTP10(true);
    http_client.begin(uri);
    http_client.addHeader("Content-Type", "application/json");
    // http_client.setAuthorization( kodi_remote_config->user, kodi_remote_config->pass );
    httpcode = http_client.POST((uint8_t*)payload, strlen(payload));
    log_i("HTTP Status Code: %d", httpcode);

    char httpcodeString[64] = "";
    sniprintf(httpcodeString, sizeof(httpcodeString), "Status:  %d", httpcode);
    lblStatus.text(httpcodeString);

    // if (httpcode >= 200 && httpcode < 300 && doc != NULL && doc != nullptr) {
    //     DeserializationError error = deserializeJson(*(doc), publish_client.getStream());
    //     if (error) {
    //         log_e("kodi_remote deserializeJson() failed: %s", error.c_str());
    //         publish_client.end();
    //         return (-2);
    //     }
    // }

    http_client.end();
    // return httpcode;
}

void test_build_main_page() {
    big = Style::Create(ws_get_mainbar_style(), true);
    big.textFont(&Ubuntu_32px)
        .textOpacity(LV_OPA_80);

    AppPage& main = testApp.mainPage();
    lv_obj_t* desks = lv_tileview_create(main.handle(), NULL);
    lv_obj_set_size(desks, LV_HOR_RES, LV_VER_RES);
    lv_page_set_scrollbar_mode(desks, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_style(desks, LV_OBJ_PART_MAIN, ws_get_mainbar_style());

    lv_obj_t* button = lv_btn_create(desks, NULL);
    lv_label_create(button, NULL);

    JoeButton btnConfig = {};
    Button btn(button);
    btn.text("Toggle")
        .size(242, 35);

    btn.clicked(
        [btnConfig](Widget& btn) {
            test_publish();
        });

    lblStatus = Label(&main);
    lblStatus.text("Status")
        // .alignText(LV_LABEL_ALIGN_CENTER)
        .style(big, true)
        .alignInParentCenter(-45, 30);

    lv_obj_t *roller_container = wf_add_container(desks, LV_LAYOUT_PRETTY_MID, LV_FIT_PARENT, LV_FIT_TIGHT, false, APP_STYLE );
    lv_obj_set_style_local_pad_left( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);
    lv_obj_set_style_local_pad_right( roller_container, LV_CONT_PART_MAIN , LV_STATE_DEFAULT, 1);
    lv_obj_align( roller_container, button, LV_ALIGN_OUT_BOTTOM_MID, 0, THEME_PADDING );

    hour_roller = wf_add_roller( roller_container, LIGHTS, LV_ROLLER_MODE_INIFINITE, ROLLER_ROW_COUNT);

    lv_obj_set_width( hour_roller, 242 );
    lv_obj_set_height( hour_roller, 45 );

    main.moveSettingsButtonToForeground();
    main.moveExitButtonToForeground();

    // Refresh screen
    lv_obj_invalidate(lv_scr_act());
}

void test_build_settings() {
    // Create full options list and attach items to variables
    config.addString("host", 32).assign(&host);
    config.addString("port", 32).assign(&port);
    config.addString("apikey", 32).assign(&apiKey);
    config.addBoolean("widget", false);

    // Switch desktop widget state based on the cuurent settings when changed
    config.onLoadSaveHandler([](JsonConfig& cfg) {
        bool widgetEnabled = cfg.getBoolean("widget");  // Is app widget enabled?
        if (widgetEnabled)
            testApp.icon().registerDesktopWidget("test", &robot_64px);
        else
            testApp.icon().unregisterDesktopWidget();
    });

    testApp.useConfig(config, true);  // true - auto create settings page widgets
}
