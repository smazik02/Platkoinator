#include <Arduino.h>
#include <ESP32Servo.h>
#include <TFT_eSPI.h>
#include <TFT_eWidget.h>

#include "Free_Fonts.h"
#include "constants.h"
#include "logo.h"
#include "milk_40.h"
#include "milk_56.h"
#include "milk_72.h"

// Uncomment if you just want to test the screen (and/or WiFi), disables processing in and out pins
// #define TEST

// Comment out to disable screen calibration
// #define SCREEN

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite logoSprite = TFT_eSprite(&tft);
TFT_eSprite milkSprite_40 = TFT_eSprite(&tft);
TFT_eSprite milkSprite_56 = TFT_eSprite(&tft);
TFT_eSprite milkSprite_72 = TFT_eSprite(&tft);

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
uint8_t cereal_leds[] = {LED_CEREAL_1, LED_CEREAL_2, LED_CEREAL_3};
#endif

#ifndef TEST
void main_function(void);
void pump_milk(void);
void deposit_cereal(void);
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
void pushMilkSprites(void);
void touch_calibrate(void);
int toPWM(int val);

void setup() {
    Serial.begin(115200);

#ifndef TEST
    ledcSetup(BELT, FREQ, RES);
    ledcSetup(PUMP, FREQ, RES);
    ledcAttachPin(BELT_EN, BELT);
    ledcAttachPin(PUMP_EN, PUMP);

    pinMode(BELT_FORWARD, OUTPUT);
    pinMode(BELT_BACK, OUTPUT);
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

    ledcWrite(BELT, 0);
    ledcWrite(PUMP, 0);

    digitalWrite(BELT_FORWARD, 1);
    digitalWrite(BELT_BACK, 0);
    digitalWrite(PUMP_FORWARD, 0);
    digitalWrite(PUMP_BACK, 0);

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

    milkSprite_40.createSprite(40, 40);
    milkSprite_40.setSwapBytes(true);

    milkSprite_56.createSprite(56, 56);
    milkSprite_56.setSwapBytes(true);

    milkSprite_72.createSprite(72, 72);
    milkSprite_72.setSwapBytes(true);

#ifdef SCREEN
    touch_calibrate();
#endif
    initScreen();
}

void loop() {
    static uint32_t scanTime = millis();
    uint16_t t_x, t_y;

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
                cereal_chosen ? "true" : "false", cereal, milk_chosen ? "true" : "false",
                milk, scene, (cereal_chosen && milk_chosen) ? "true" : "false");

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
// Main functionality of the program
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

    while (analogRead(SENSOR_START) > SENSOR_SENSITIVITY);

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Nalewanie mleka", tft.width() / 2, tft.height() / 2);

    ledcWrite(BELT, toPWM(BELT_SPEED));
    while (analogRead(SENSOR_MILK) > SENSOR_SENSITIVITY);

    ledcWrite(BELT, 0);
    delay(500);
    pump_milk();
    delay(500);

    uint8_t range;
    switch (cereal) {
        case 0:
            range = LED_CEREAL_1;
            break;
        case 1:
            range = LED_CEREAL_2;
            break;
        case 2:
            range = LED_CEREAL_3;
            break;
    }

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Nasypywanie platkow", tft.width() / 2, tft.height() / 2);

    ledcWrite(BELT, toPWM(BELT_SPEED));
    while (analogRead(cereal_sensors[cereal]) > SENSOR_SENSITIVITY);

    ledcWrite(BELT, 0);
    delay(500);
    deposit_cereal();
    delay(500);

    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_BLUE);
    tft.drawString("Juz prawie gotowe", tft.width() / 2, tft.height() / 2);

    ledcWrite(BELT, toPWM(BELT_SPEED));
    while (analogRead(SENSOR_END) > SENSOR_SENSITIVITY);

    ledcWrite(BELT, 0);
    tft.setTextColor(TFT_BLACK);
    tft.fillRoundRect(100, 100, tft.width() - 200, tft.height() - 200, 5, TFT_GREEN);
    tft.drawString("Odbierz platki", tft.width() / 2, tft.height() / 2 - 18);
    tft.drawString("Smacznego :)", tft.width() / 2, tft.height() / 2 + 18);

    while (analogRead(SENSOR_END) <= SENSOR_SENSITIVITY);

    uint32_t end = millis();
    while (millis() - end < 5000);

    initScreen();
}

void pump_milk(void) {
    digitalWrite(PUMP_BACK, 0);
    digitalWrite(PUMP_FORWARD, 1);
    ledcWrite(PUMP, toPWM(PUMP_SPEED));
    delay(3000 * milk + 1);  // TODO - specify delay based on chosen milk amount

    ledcWrite(PUMP, 0);
    delay(500);

    digitalWrite(PUMP_FORWARD, 0);
    digitalWrite(PUMP_BACK, 1);
    ledcWrite(PUMP, toPWM(PUMP_SPEED));
    delay(1000);
    ledcWrite(PUMP, 0);

    digitalWrite(PUMP_BACK, 0);
}

void deposit_cereal(void) {
    servo[cereal].write(100.0f);
    delay(5000);
    servo[cereal].write(90.0f);
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
        pushMilkSprites();
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
        pushMilkSprites();
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
        pushMilkSprites();
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
        case 1: {
            btn1.drawSmoothButton(cereal == 0 ? true : false, 3, TFT_WHITE);
            btn2.drawSmoothButton(cereal == 1 ? true : false, 3, TFT_WHITE);
            btn3.drawSmoothButton(cereal == 2 ? true : false, 3, TFT_WHITE);
            btn_next.drawSmoothButton(false, 3, TFT_WHITE);
            break;
        }

        case 2: {
            btn4.drawSmoothButton(milk == 0 ? true : false, 3, TFT_WHITE);
            btn5.drawSmoothButton(milk == 1 ? true : false, 3, TFT_WHITE);
            btn6.drawSmoothButton(milk == 2 ? true : false, 3, TFT_WHITE);
            btn_back.drawSmoothButton(false, 3, TFT_WHITE);
            btn_next.drawSmoothButton(false, 3, TFT_WHITE);
            pushMilkSprites();
            break;
        }

        case 3: {
            btn_ok.drawSmoothButton(false, 3, TFT_WHITE);
            break;
        }
    }
    Serial.printf("Switching to scene %d\n\n", scene);
}

void initButtons() {
    uint16_t x = (tft.width() - BUTTON_W) / 2 - BUTTON_W - 10;
    uint16_t y = tft.height() / 2 - BUTTON_H;
    btn1.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 1", 1);
    btn1.setPressAction(btn1_pressAction);
    btn4.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "", 1);
    btn4.setPressAction(btn4_pressAction);

    x += BUTTON_W + 10;
    btn2.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 2", 1);
    btn2.setPressAction(btn2_pressAction);
    btn5.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "", 1);
    btn5.setPressAction(btn5_pressAction);

    x += BUTTON_W + 10;
    btn3.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "Platki 3", 1);
    btn3.setPressAction(btn3_pressAction);
    btn6.initButtonUL(x, y, BUTTON_W, BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "", 1);
    btn6.setPressAction(btn6_pressAction);

    x = (tft.width() - NAV_BUTTON_W) / 2 - (NAV_BUTTON_W / 2 + 10);
    y += BUTTON_H + 30;
    btn_back.initButtonUL(x, y, NAV_BUTTON_W, NAV_BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "BACK", 1);
    btn_back.setPressAction(back_btn_pressAction);

    x += NAV_BUTTON_W + 20;
    btn_next.initButtonUL(x, y, NAV_BUTTON_W, NAV_BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "NEXT", 1);
    btn_next.setPressAction(next_btn_pressAction);

    x = (tft.width() - OK_BUTTON_W) / 2;
    y = (tft.height() - OK_BUTTON_H) / 2;
    btn_ok.initButtonUL(x, y, OK_BUTTON_W, OK_BUTTON_H, TFT_BLACK, TFT_BLUE, TFT_WHITE, "PROCEED", 1);
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

void pushMilkSprites(void) {
    uint16_t x = BUTTON_W / 2 + 10;
    uint16_t y = tft.height() / 2 - BUTTON_H + 30;
    milkSprite_40.pushImage(0, 0, 40, 40, milk_40);
    milkSprite_40.pushSprite(x, y, TFT_BLACK);

    x += BUTTON_W + 10 - 8;
    y -= 8;
    milkSprite_56.pushImage(0, 0, 56, 56, milk_56);
    milkSprite_56.pushSprite(x, y, TFT_BLACK);

    x += BUTTON_W + 10 - 8;
    y -= 8;
    milkSprite_72.pushImage(0, 0, 72, 72, milk_72);
    milkSprite_72.pushSprite(x, y, TFT_BLACK);
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

int toPWM(int val) {
    return map(val, 0, 100, 0, pow(2, RES));
}