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

struct Rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} colors[] = {
    {0xff, 0xff, 0xff}, // #ffffff
    {0x37, 0x61, 0xa1}, // #3761a1
    {0x37, 0x91, 0xa1}, // #3791a1
    {0x00, 0xFF, 0xEF}, // #00ffef
    {0x00, 0xb3, 0x00}, // #00b300
    {0x9d, 0xfe, 0x86}, // #9dfe86
    {0xff, 0x8d, 0x99}, // #ff8d99
    {0xff, 0x00, 0x00}, // #ff0000
    {0xff, 0x00, 0x77}, // #ff0077
    {0xff, 0x77, 0x00}, // #ff7700
    {0xd0, 0x9f, 0xf5}, // #d09ff5
    {0xa1, 0x9f, 0xfc}, // #a19ffc
    {0x80, 0x00, 0x80}, // #800080
    {0xa7, 0x4b, 0x4b}, // #a7194b
    {0xff, 0xff, 0x00}, // #ffff00
    {0x96, 0x4b, 0x00}, // #964B00
};

const uint8_t colorCount = sizeof(colors) / sizeof(Rgb);
const uint8_t maxColor = colorCount * 10;
const float brightnessRatio = 255.0 / 10.0;

struct Color
{
    Rgb rgb;
    uint8_t brightness;
};

Color getColor(uint8_t color)
{
    if (color > maxColor)
    {
        return {
            .rgb = {0x00, 0x00, 0x00},
            .brightness = 0,
        };
    }

    uint8_t colorIndex = color / 10;
    uint8_t brighness = (color % 10) * brightnessRatio + 5;

    return {
        .rgb = colors[colorIndex],
        .brightness = brighness,
    };
}

uint8_t counter = 0;
// define a callback for key presses
TrellisCallback blink(keyEvent evt)
{
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING)
    {
        Color color = getColor(counter++);

        Serial.printf("color: r %d g %d b %d  brightness %d\n", color.rgb.r, color.rgb.g, color.rgb.b, color.brightness);

        uint32_t c = trellis.pixels.Color(color.rgb.r, color.rgb.g, color.rgb.b);
        trellis.pixels.setBrightness(color.brightness);
        trellis.pixels.setPixelColor(evt.bit.NUM, c);
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
    if (!trellis.begin())
    {
        initialised = false;
    }

    trellisM4.begin();
    // if (!)
    // {
    //     initialised = false;
    // }

    // Serial.println("basic keypad test!");

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
            trellisM4.setPixelColor(e.bit.KEY, 0xFFFFFF);
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
