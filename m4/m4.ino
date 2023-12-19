// Adafruit_NeoTrellis t_array[] = {Adafruit_NeoTrellis(0x2E)};

// pass this matrix to the Adafruit_M4MultiTrellis_8x8 object

#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_NeoTrellisM4.h>

// #include <Arduino.h>

Adafruit_NeoTrellis trellis;
Adafruit_NeoTrellisM4 trellisM4 = Adafruit_NeoTrellisM4();

bool initialised = true;

uint32_t colors[] = {
    0xffffff,
    0x3761a1,
    0x3791a1,
    0x00ffef,
    0x00b300,
    0x9dfe86,
    0xff8d99,
    0xff0000,
    0xff0077,
    0xff7700,
    0xd09ff5,
    0xa19ffc,
    0x800080,
    0xa7194b,
    0xffff00,
    0x964B00,
};

const uint8_t colorCount = sizeof(colors) / sizeof(uint32_t);
const uint8_t maxColor = colorCount * 10;
const float brightnessRatio = 255.0 / 10.0;

struct Color
{
    uint32_t value;
    uint8_t brightness;
};

Color getColor(uint8_t color)
{
    if (color > maxColor)
    {
        return {.value = 0, .brightness = 0};
    }

    uint8_t colorIndex = color / 10;
    uint8_t brighness = (color % 10) * brightnessRatio + 5;

    return {.value = colors[colorIndex], .brightness = brighness};
}

uint8_t counter = 0;
// define a callback for key presses
TrellisCallback blink(keyEvent evt)
{
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING)
    {
        Color color = getColor(counter++);
        Serial.printf("color: rgb #%x  brightness %d\n", color.value, color.brightness);
        trellis.pixels.setBrightness(color.brightness);
        trellis.pixels.setPixelColor(evt.bit.NUM, color.value);
    }
    else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING)
    {
        trellis.pixels.setPixelColor(evt.bit.NUM, 0);
    }

    trellis.pixels.show();

    return 0;
}

void setup()
{
    Serial.begin(115200);

    trellisM4.begin();

    if (!trellis.begin())
    {
        initialised = false;
    }

    // activate all keys and set callbacks
    for (int i = 0; i < NEO_TRELLIS_NUM_KEYS; i++)
    {
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_RISING);
        trellis.activateKey(i, SEESAW_KEYPAD_EDGE_FALLING);
        trellis.registerCallback(i, blink);
    }
}

unsigned long last = 0;
void loop()
{
    trellis.read();

    trellisM4.tick();
    while (trellisM4.available())
    {
        keypadEvent e = trellisM4.read();
        Serial.print((int)e.bit.KEY);
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            Serial.println(" pressed");
            // trellisM4.setPixelColor(e.bit.KEY, 0xFFFFFF);

            Color color = getColor(counter++);
            Serial.printf("color: rgb #%x  brightness %d\n", color.value, color.brightness);
            trellisM4.setBrightness(color.brightness);
            trellisM4.setPixelColor(e.bit.KEY, color.value);
        }
        else if (e.bit.EVENT == KEY_JUST_RELEASED)
        {
            Serial.println(" released");
            trellisM4.setPixelColor(e.bit.KEY, 0x0);
        }
    }

    delay(10);

    if (millis() - last > 2000)
    {
        last = millis();
        Serial.printf("loop: %s\n", initialised ? "initialised" : "not initialised");
    }
}
