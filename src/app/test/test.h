#ifndef _test_APP_H
    #define _test_APP_H

    #ifdef NATIVE_64BIT
        #include <string>
        using namespace std;
        #define String string
    #endif

    struct JoeButton {
        char name[16];
    };

    void test_app_setup();

    // bool test_wifictl_event_cb(EventBits_t event, void *arg);
    // bool fetch_data(String apiKey);

    void test_build_main_page();
    void test_build_settings();
    void test_publish();

#endif // _test_APP_H