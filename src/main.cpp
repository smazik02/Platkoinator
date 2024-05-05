#include <Arduino.h>
#include <ESP32Servo.h>
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>

#include "Free_Fonts.h"
#include "constants.h"
#include "logo.h"

// #define WIFI
#define TEST

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite logoSprite = TFT_eSprite(&tft);

#ifdef WIFI
WiFiServer server(80);
const char *ssid = "Platkoinator";
const char *password = "zaq1@WSX";
#endif

ButtonWidget btn_start = ButtonWidget(&tft);

ButtonWidget btn1 = ButtonWidget(&tft);
ButtonWidget btn2 = ButtonWidget(&tft);
ButtonWidget btn3 = ButtonWidget(&tft);

ButtonWidget btn4 = ButtonWidget(&tft);
ButtonWidget btn5 = ButtonWidget(&tft);
ButtonWidget btn6 = ButtonWidget(&tft);

ButtonWidget btn_back = ButtonWidget(&tft);
ButtonWidget btn_next = ButtonWidget(&tft);
ButtonWidget btn_ok = ButtonWidget(&tft);

ButtonWidget *btn_s1[] = {&btn1, &btn2, &btn3, &btn_next};
ButtonWidget *btn_s2[] = {&btn4, &btn5, &btn6, &btn_next, &btn_back};

bool start, cereal_chosen, milk_chosen, ready;
int8_t cereal, milk, scene;
#ifndef TEST
Servo servo[3];
uint8_t cereal_sensors[] = {SENSOR_CEREAL_1, SENSOR_CEREAL_2, SENSOR_CEREAL_3};

void main_function(void);
void pump_milk(void);
void deposit_cereal(void);
#endif
#ifdef WIFI
void handleWiFi(void);
#endif
void btn1_pressAction(void);
void btn2_pressAction(void);
void btn3_pressAction(void);
void btn4_pressAction(void);
void btn5_pressAction(void);
void btn6_pressAction(void);
void ok_btn_pressAction(void);
void back_btn_pressAction(void);
void next_btn_pressAction(void);
void switchScene(uint8_t scene);
void initButtons(void);
void initScreen(void);
void touch_calibrate(void);

void setup() {
    Serial.begin(115200);

#ifndef TEST
    pinMode(BELT, OUTPUT);
    pinMode(BELT_FORWARD, OUTPUT);
    pinMode(BELT_BACK, OUTPUT);
    pinMode(PUMP, OUTPUT);
    pinMode(PUMP_FORWARD, OUTPUT);
    pinMode(PUMP_BACK, OUTPUT);
    pinMode(SENSOR_START, INPUT_PULLUP);
    pinMode(SENSOR_MILK, INPUT_PULLUP);
    pinMode(SENSOR_CEREAL_1, INPUT_PULLUP);
    pinMode(SENSOR_CEREAL_2, INPUT_PULLUP);
    pinMode(SENSOR_CEREAL_3, INPUT_PULLUP);
    pinMode(SENSOR_END, INPUT_PULLUP);
    pinMode(SERVO_CEREAL_1, OUTPUT);
    pinMode(SERVO_CEREAL_2, OUTPUT);
    pinMode(SERVO_CEREAL_3, OUTPUT);
    pinMode(SOLENOID_1, OUTPUT);
    pinMode(SOLENOID_2, OUTPUT);

    digitalWrite(BELT, 0);
    digitalWrite(BELT_FORWARD, 1);
    digitalWrite(BELT_BACK, 0);
    digitalWrite(PUMP, 0);
    digitalWrite(PUMP_FORWARD, 0);
    digitalWrite(PUMP_BACK, 0);
    analogWrite(SERVO_CEREAL_1, 90.0f);
    digitalWrite(SERVO_CEREAL_2, 90.0f);
    digitalWrite(SERVO_CEREAL_3, 90.0f);
    digitalWrite(SOLENOID_1, 0);
    digitalWrite(SOLENOID_2, 0);

    servo[0].attach(SERVO_CEREAL_1);
    servo[1].attach(SERVO_CEREAL_2);
    servo[2].attach(SERVO_CEREAL_3);
    servo[0].write(90.0f);
    servo[1].write(90.0f);
    servo[2].write(90.0f);
#endif

    tft.begin();
    tft.setRotation(1);
    tft.setSwapBytes(true);

    logoSprite.createSprite(LOGO_W, LOGO_H);
    logoSprite.setSwapBytes(true);

#ifdef WIFI
    if (!WiFi.softAP(ssid, password)) {
        log_e("Soft AP creation failed.");
        while (1);
    }
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    server.begin();

    Serial.println("Server started");
#endif

    touch_calibrate();
    initScreen();
}

void loop() {
    static uint32_t scanTime = millis();
    uint16_t t_x, t_y;

#ifdef WIFI
    handleWiFi();
#endif

    if (millis() - scanTime >= 50) {
        bool pressed = tft.getTouch(&t_x, &t_y);
        // if (pressed) tft.fillCircle(t_x, t_y, 2, TFT_BLACK);
        scanTime = millis();
        switch (scene) {
            case 1:
                for (auto button : btn_s1) {
                    if (pressed) {
                        if (button->contains(t_x, t_y)) {
                            button->press(true);
                            button->pressAction();
                        }
                    } else {
                        button->press(false);
                    }
                }
                break;
            case 2:
                for (auto button : btn_s2) {
                    if (pressed) {
                        if (button->contains(t_x, t_y)) {
                            button->press(true);
                            button->pressAction();
                        }
                    } else {
                        button->press(false);
                    }
                }
                break;
            case 3:
                if (pressed) {
                    if (btn_ok.contains(t_x, t_y)) {
                        btn_ok.press(true);
                        btn_ok.pressAction();
                    }
                } else {
                    btn_ok.press(false);
                }
                break;
        }
        if (pressed)
            Serial.printf(
                "Cereal chosen: %s Cereal: %d\nMilk chosen: %s Milk: %d\nScene: %d, Both: %s\n\n",
                cereal_chosen ? "true" : "false",
                cereal, milk_chosen ? "true" : "false",
                milk, scene,
                (cereal_chosen && milk_chosen) ? "true" : "false");

        if (ready) {
#ifndef TEST
            main_function();
#else
            Serial.println("Platki czas zaczac");
            Serial.printf("Platki %d, mleko %d\n", cereal, milk);
            tft.drawRoundRect(99, 99, tft.width() - 198, tft.height() - 198, 5, TFT_BLACK);
            tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_YELLOW);
            tft.setTextColor(TFT_BLACK);
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            tft.drawString("Platki sa przygotowywane", tft.width() / 2, tft.height() / 2 - 18);
            tft.drawString("(Tu beda etapy po kolei)", tft.width() / 2, tft.height() / 2 + 18);
            delay(5000);

            tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_GREEN);
            tft.drawString("Odbierz platki", tft.width() / 2, tft.height() / 2 - 18);
            tft.drawString("Smacznego :)", tft.width() / 2, tft.height() / 2 + 18);
            delay(2000);

            initScreen();
#endif
        }
    }
}

#ifndef TEST
void main_function(void) {
    Serial.println("Platki czas zaczac");
    Serial.printf("Platki %d, mleko %d\n", cereal, milk);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);

    tft.drawRoundRect(99, 99, tft.width() - 198, tft.height() - 198, 5, TFT_BLACK);
    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Platki sa przygotowywane", tft.width() / 2, tft.height() / 2 - 18);
    tft.drawString("Wydawanie miski", tft.width() / 2, tft.height() / 2 + 18);
    delay(1000);

    // TODO - wydajemy miskę
    while (analogRead(SENSOR_START) < SENSOR_SENSITIVITY);

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Nalewanie mleka", tft.width() / 2, tft.height() / 2);

    analogWrite(BELT, BELT_SPEED);
    while (analogRead(SENSOR_MILK) < SENSOR_SENSITIVITY);

    analogWrite(BELT, 0.0f);
    delay(500);
    pump_milk();
    delay(500);

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Nasypywanie platkow", tft.width() / 2, tft.height() / 2);

    analogWrite(BELT, BELT_SPEED);
    while (analogRead(cereal_sensors[cereal]) < SENSOR_SENSITIVITY);

    analogWrite(BELT, 0.0f);
    delay(500);
    deposit_cereal();
    delay(500);

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Juz prawie gotowe", tft.width() / 2, tft.height() / 2);

    analogWrite(BELT, BELT_SPEED);
    while (analogRead(SENSOR_END) < SENSOR_SENSITIVITY);

    analogWrite(BELT, 0.0f);
    tft.setTextColor(TFT_BLACK);
    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_GREEN);
    tft.drawString("Odbierz platki", tft.width() / 2, tft.height() / 2 - 18);
    tft.drawString("Smacznego :)", tft.width() / 2, tft.height() / 2 + 18);

    while (analogRead(SENSOR_END) >= SENSOR_SENSITIVITY);
    delay(5000);

    initScreen();
}

void pump_milk(void) {
    digitalWrite(PUMP_FORWARD, 1);
    digitalWrite(PUMP_BACK, 0);
    analogWrite(PUMP, PUMP_SPEED);
    delay(3000);  // TODO - specify delay based on chosen milk amount

    analogWrite(PUMP, 0);
    delay(500);

    digitalWrite(PUMP_FORWARD, 0);
    digitalWrite(PUMP_BACK, 1);
    analogWrite(PUMP, PUMP_SPEED);
    delay(1000);
    analogWrite(PUMP, 0);
}

void deposit_cereal(void) {
}
#endif

#ifdef WIFI
void handleWiFi(void) {
    WiFiClient client = server.available();
    if (client) {
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        client.print("<!DOCTYPE html><html>\n");
                        client.print("<head><title>PLATKOINATOR</title><style>\n");
                        client.print("body {font-family: Verdana, Geneva, Tahoma, sans-serif;background-color: #ffffff;}\n");
                        client.print("div {background-color: #ffffff;}\n");
                        client.print("div#header {margin-left: 1vw;font-size: 4vw;text-align: center;}\n");
                        client.print("div.buttons {padding-left: 10px;padding-right: 10px;text-align: center;}\n");
                        client.print(".btn {font-size: min(5.5vw, 50px);color: antiquewhite;border: none;border-radius: 10px;padding: 20px;text-align: center;text-decoration: none;display: inline-block;margin: 0.5vh 0.3vw;height: min(12vh, 100px);width: 30vw;background-color: black;}\n");
                        client.print("#btn1 {font-size: 5.5vw;width: 50wv;}</style></head>\n");
                        client.print("<body><div id=\"header\"><h1>PLATKOINATOR 3000</h1></div>\n");
                        client.print("<div class=\"buttons\">\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/P1')\">Platki 1</button>\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/P2')\">Platki 2</button>\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/P3')\">Platki 3</button></div>\n");
                        client.print("<div class=\"buttons\">\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/M1')\">Mleko 1</button>\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/M2')\">Mleko 2</button>\n");
                        client.print("<button class=\"btn\" onclick=\"fetch('http:\\/\\/192.168.4.1/M3')\">Mleko 3</button></div><br />\n");
                        client.print("<div id=\"done\" class=\"buttons\"><buttonclass=\"btn\"id=\"btn1\"onclick=\"fetch('http:\\/\\/192.168.4.1/OK')\">PROCEED</button></div>\n");
                        client.print("</body></html>");

                        // client.print("");

                        client.println();
                        break;
                    } else {
                        currentLine = "";
                    }
                } else if (c != '\r') {
                    currentLine += c;
                }

                if (currentLine.endsWith("GET /C1")) {
                    if (scene == 1) {
                        if (btn2.getState())
                            btn2.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn3.getState())
                            btn3.drawSmoothButton(false, 3, TFT_WHITE);
                        btn1.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    cereal_chosen = true;
                    cereal = 0;
                    Serial.println("Platki 1 wybrane");
                }
                if (currentLine.endsWith("GET /C2")) {
                    if (scene == 1) {
                        if (btn1.getState())
                            btn1.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn3.getState())
                            btn3.drawSmoothButton(false, 3, TFT_WHITE);
                        btn2.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    cereal_chosen = true;
                    cereal = 1;
                    Serial.println("Platki 2 wybrane");
                }
                if (currentLine.endsWith("GET /C3")) {
                    if (scene == 1) {
                        if (btn1.getState())
                            btn1.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn2.getState())
                            btn2.drawSmoothButton(false, 3, TFT_WHITE);
                        btn3.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    cereal_chosen = true;
                    cereal = 2;
                    Serial.println("Platki 3 wybrane");
                }

                if (currentLine.endsWith("GET /M1")) {
                    if (scene == 2) {
                        if (btn5.getState())
                            btn5.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn6.getState())
                            btn6.drawSmoothButton(false, 3, TFT_WHITE);
                        btn4.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    milk_chosen = true;
                    milk = 0;
                    Serial.println("Mleko 1 wybrane");
                }
                if (currentLine.endsWith("GET /M2")) {
                    if (scene == 2) {
                        if (btn4.getState())
                            btn4.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn6.getState())
                            btn6.drawSmoothButton(false, 3, TFT_WHITE);
                        btn5.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    milk_chosen = true;
                    milk = 1;
                    Serial.println("Mleko 2 wybrane");
                }
                if (currentLine.endsWith("GET /M3")) {
                    if (scene == 2) {
                        if (btn4.getState())
                            btn4.drawSmoothButton(false, 3, TFT_WHITE);
                        if (btn5.getState())
                            btn5.drawSmoothButton(false, 3, TFT_WHITE);
                        btn6.drawSmoothButton(true, 3, TFT_WHITE);
                    }
                    milk_chosen = true;
                    milk = 2;
                    Serial.println("Mleko 3 wybrane");
                }

                if (currentLine.endsWith("GET /OK")) {
                    if (scene == 3)
                        btn_ok.drawSmoothButton(!btn_ok.getState(), 3, TFT_WHITE);
                    if (!cereal_chosen || !milk_chosen) {
                        tft.drawRoundRect(99, 99, tft.width() - 198, tft.height() - 198, 5, TFT_BLACK);
                        tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_RED);
                        tft.setTextColor(TFT_WHITE);
                        tft.setTextSize(1);
                        tft.setTextDatum(MC_DATUM);
                        if (!cereal_chosen) {
                            tft.drawString("Wybierz platki", tft.width() / 2, tft.height() / 2);
                            scene = 1;
                        } else {
                            tft.drawString("Wybierz mleko", tft.width() / 2, tft.height() / 2);
                            scene = 2;
                        }
                        delay(2000);

                        switchScene(scene);
                        return;
                    }
                    Serial.println("Platkoinator naprzod");
                    ready = true;
                }
            }
        }

        client.stop();
    }
}
#endif

void btn1_pressAction() {
    if (btn1.justPressed()) {
        if (btn2.getState())
            btn2.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn3.getState())
            btn3.drawSmoothButton(false, 3, TFT_WHITE);
        btn1.drawSmoothButton(!btn1.getState(), 3, TFT_WHITE);
        cereal_chosen = btn1.getState() ? true : false;
        cereal = btn1.getState() ? 0 : -1;
        if (btn1.getState())
            Serial.println("Platki 1 wybrane");
        else
            Serial.println("Platki odznaczone");
    }
}

void btn2_pressAction() {
    if (btn2.justPressed()) {
        if (btn1.getState())
            btn1.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn3.getState())
            btn3.drawSmoothButton(false, 3, TFT_WHITE);
        btn2.drawSmoothButton(!btn2.getState(), 3, TFT_WHITE);
        cereal_chosen = btn2.getState() ? true : false;
        cereal = btn2.getState() ? 1 : -1;
        if (btn2.getState())
            Serial.println("Platki 2 wybrane");
        else
            Serial.println("Platki odznaczone");
    }
}

void btn3_pressAction() {
    if (btn3.justPressed()) {
        if (btn1.getState())
            btn1.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn2.getState())
            btn2.drawSmoothButton(false, 3, TFT_WHITE);
        btn3.drawSmoothButton(!btn3.getState(), 3, TFT_WHITE);
        cereal_chosen = btn3.getState() ? true : false;
        cereal = btn3.getState() ? 2 : -1;
        if (btn3.getState())
            Serial.println("Platki 3 wybrane");
        else
            Serial.println("Platki odznaczone");
    }
}

void btn4_pressAction() {
    if (btn4.justPressed()) {
        if (btn5.getState())
            btn5.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn6.getState())
            btn6.drawSmoothButton(false, 3, TFT_WHITE);
        btn4.drawSmoothButton(!btn4.getState(), 3, TFT_WHITE);
        milk_chosen = btn4.getState() ? true : false;
        milk = btn4.getState() ? 0 : -1;
        if (btn4.getState())
            Serial.println("Mleko 1 wybrane");
        else
            Serial.println("Mleko odznaczone");
    }
}

void btn5_pressAction() {
    if (btn5.justPressed()) {
        if (btn4.getState())
            btn4.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn6.getState())
            btn6.drawSmoothButton(false, 3, TFT_WHITE);
        btn5.drawSmoothButton(!btn5.getState(), 3, TFT_WHITE);
        milk_chosen = btn5.getState() ? true : false;
        milk = btn5.getState() ? 1 : -1;
        if (btn5.getState())
            Serial.println("Mleko 2 wybrane");
        else
            Serial.println("Mleko odznaczone");
    }
}

void btn6_pressAction() {
    if (btn6.justPressed()) {
        if (btn4.getState())
            btn4.drawSmoothButton(false, 3, TFT_WHITE);
        if (btn5.getState())
            btn5.drawSmoothButton(false, 3, TFT_WHITE);
        btn6.drawSmoothButton(!btn6.getState(), 3, TFT_WHITE);
        milk_chosen = btn6.getState() ? true : false;
        milk = btn6.getState() ? 2 : -1;
        if (btn6.getState())
            Serial.println("Mleko 3 wybrane");
        else
            Serial.println("Mleko odznaczone");
    }
}

void ok_btn_pressAction() {
    if (btn_ok.justPressed()) {
        btn_ok.drawSmoothButton(!btn_ok.getState(), 3, TFT_WHITE);
        if (!cereal_chosen || !milk_chosen) {
            tft.drawRoundRect(99, 99, tft.width() - 198, tft.height() - 198, 5, TFT_BLACK);
            tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_RED);
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(1);
            tft.setTextDatum(MC_DATUM);
            if (!cereal_chosen) {
                tft.drawString("Wybierz platki", tft.width() / 2, tft.height() / 2);
                scene = 1;
            } else {
                tft.drawString("Wybierz mleko", tft.width() / 2, tft.height() / 2);
                scene = 2;
            }
            delay(2000);

            switchScene(scene);
            return;
        }
        Serial.println("Platkoinator naprzod");
        ready = true;
    }
}

void back_btn_pressAction() {
    if (btn_back.justPressed()) {
        switchScene(--scene);
    }
}

void next_btn_pressAction() {
    if (btn_next.justPressed()) {
        switchScene(++scene);
    }
}

void switchScene(uint8_t scene) {
    tft.fillScreen(TFT_WHITE);
    switch (scene) {
        case 1:
            btn1.drawSmoothButton(cereal == 0 ? true : false, 3, TFT_WHITE);
            btn2.drawSmoothButton(cereal == 1 ? true : false, 3, TFT_WHITE);
            btn3.drawSmoothButton(cereal == 2 ? true : false, 3, TFT_WHITE);
            btn_next.drawSmoothButton(false, 3, TFT_WHITE);
            break;

        case 2:
            btn4.drawSmoothButton(milk == 0 ? true : false, 3, TFT_WHITE);
            btn5.drawSmoothButton(milk == 1 ? true : false, 3, TFT_WHITE);
            btn6.drawSmoothButton(milk == 2 ? true : false, 3, TFT_WHITE);
            btn_back.drawSmoothButton(false, 3, TFT_WHITE);
            btn_next.drawSmoothButton(false, 3, TFT_WHITE);
            break;

        case 3:
            btn_ok.drawSmoothButton(false, 3, TFT_WHITE);
            break;
    }
    Serial.printf("Switching to scene %d\n\n", scene);
}

void initButtons() {
    uint16_t x = (tft.width() - BUTTON_W) / 2 - BUTTON_W - 10;
    uint16_t y = tft.height() / 2 - BUTTON_H;
    btn1.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 1", 1);
    btn1.setPressAction(btn1_pressAction);
    btn4.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Mleko 1", 1);
    btn4.setPressAction(btn4_pressAction);

    x += BUTTON_W + 10;
    btn2.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 2", 1);
    btn2.setPressAction(btn2_pressAction);
    btn5.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Mleko 2", 1);
    btn5.setPressAction(btn5_pressAction);

    x += BUTTON_W + 10;
    btn3.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 3", 1);
    btn3.setPressAction(btn3_pressAction);
    btn6.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Mleko 3", 1);
    btn6.setPressAction(btn6_pressAction);

    x = (tft.width() - NAV_BUTTON_W) / 2 - (NAV_BUTTON_W / 2 + 10);
    y += BUTTON_H + 30;
    btn_back.initButtonUL(x, y, NAV_BUTTON_W, NAV_BUTTON_H, TFT_BLACK, TFT_WHITE, TFT_BLACK, "BACK", 1);
    btn_back.setPressAction(back_btn_pressAction);

    x += NAV_BUTTON_W + 20;
    btn_next.initButtonUL(x, y, NAV_BUTTON_W, NAV_BUTTON_H, TFT_BLACK, TFT_WHITE, TFT_BLACK, "NEXT", 1);
    btn_next.setPressAction(next_btn_pressAction);

    x = (tft.width() - OK_BUTTON_W) / 2;
    y = (tft.height() - OK_BUTTON_H) / 2;
    btn_ok.initButtonUL(x, y, OK_BUTTON_W, OK_BUTTON_H, TFT_BLACK, TFT_WHITE, TFT_BLACK, "PROCEED", 1);
    btn_ok.setPressAction(ok_btn_pressAction);
}

void initScreen() {
    start = false;
    cereal_chosen = false;
    milk_chosen = false;
    ready = false;
    scene = 1;
    cereal = -1;
    milk = -1;

    uint32_t initTime = millis();

    tft.fillScreen(TFT_WHITE);
    tft.setFreeFont(FF40);
    tft.setTextColor(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("PLATKOINATOR", tft.width() / 2, tft.height() / 2 - 80);

    logoSprite.pushImage(0, 0, LOGO_W, LOGO_H, logo);
    logoSprite.pushSprite(tft.width() / 2 - LOGO_W / 2, tft.height() / 2 - 50, TFT_BLACK);

    tft.setFreeFont(FF18);
    while (millis() - initTime < 3000);

    tft.drawString("Dotknij ekranu by zaczac", tft.width() / 2, tft.height() / 2 + 120);
    uint16_t tmp;
    initTime = millis();

    while (!tft.getTouch(&tmp, &tmp));

    tft.fillScreen(TFT_WHITE);
    initButtons();
    switchScene(scene);
    delay(200);
}

void touch_calibrate() {
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 0);
    tft.setTextFont(2);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");
}