// Adafruit_NeoTrellis t_array[] = {Adafruit_NeoTrellis(0x2E)};

// pass this matrix to the Adafruit_M4MultiTrellis_8x8 object

#include <Adafruit_Keypad.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoPixel_ZeroDMA.h>
#include <Adafruit_NeoTrellis.h>
#include <Adafruit_NeoTrellisM4.h>

#define MIDI_CHANNEL 0
#define FIRST_MIDI_NOTE 36

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
    uint8_t key = evt.bit.NUM + (int)(evt.bit.NUM / 4.0 + 1) * 8;
    if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_RISING)
    {
        Color color = getColor(counter++);
        Serial.printf("key %d >> %d color: rgb #%x  brightness %d\n", evt.bit.NUM, key, color.value, color.brightness);
        trellis.pixels.setBrightness(color.brightness);
        trellis.pixels.setPixelColor(evt.bit.NUM, color.value);
        trellisM4.noteOn(FIRST_MIDI_NOTE + key, 127);
    }
    else if (evt.bit.EDGE == SEESAW_KEYPAD_EDGE_FALLING)
    {
        trellis.pixels.setPixelColor(evt.bit.NUM, 0);
        trellisM4.noteOff(FIRST_MIDI_NOTE + evt.bit.NUM, 127);
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

    // USB MIDI messages sent over the micro B USB port
    Serial.println("Enabling MIDI on USB");
    trellisM4.enableUSBMIDI(true);
    trellisM4.setUSBMIDIchannel(MIDI_CHANNEL);
}

unsigned long last = 0;
void loop()
{
    trellis.read();

    trellisM4.tick();
    while (trellisM4.available())
    {
        keypadEvent e = trellisM4.read();
        uint8_t key = e.bit.KEY + (int)(e.bit.KEY / 8.0) * 4;
        if (e.bit.EVENT == KEY_JUST_PRESSED)
        {
            Color color = getColor(counter++);
            Serial.printf("key %d >> %d color: rgb #%x  brightness %d\n", e.bit.KEY, key, color.value, color.brightness);
            trellisM4.setBrightness(color.brightness);
            trellisM4.setPixelColor(e.bit.KEY, color.value);
            trellisM4.noteOn(FIRST_MIDI_NOTE + key, 127);
        }
        else if (e.bit.EVENT == KEY_JUST_RELEASED)
        {
            trellisM4.setPixelColor(e.bit.KEY, 0x0);
            trellisM4.noteOff(FIRST_MIDI_NOTE + key, 127);
        }
    }

    // for (;;)
    // {
    //     midiEventPacket_t packet = MidiUSB.read();
    //     if (packet.header == 0)
    //     {
    //         break;
    //     }
    //     Serial.printf("MIDI recv: %02x %02x %02x %02x\n", packet.header, packet.byte1, packet.byte2, packet.byte3);
    // }

    // while (MidiUSB.available())
    // {
    //     midiEventPacket_t packet = MidiUSB.read();
    //     Serial.printf("MIDI recv: %02x %02x %02x %02x\n", packet.header, packet.byte1, packet.byte2, packet.byte3);
    // }

    while (Serial.available())
    {
        uint8_t cmd = Serial.read();
        if (cmd == 0x23) // equivalent to '#' command to set color
        {
            uint8_t key = Serial.read();
            Color color = getColor(Serial.read());
            // Serial.printf("key %d color: rgb #%x  brightness %d\n", key, color.value, color.brightness);

            uint8_t column = key % 12;
            uint8_t row = key / 12;
            if (column < 8)
            {
                trellisM4.setBrightness(color.brightness);
                trellisM4.setPixelColor(column + row * 8, color.value);
            }
            else
            {
                trellis.pixels.setBrightness(color.brightness);
                trellis.pixels.setPixelColor(column - 8 + row * 4, color.value);
                trellis.pixels.show();
            }
        }
    }

    delay(10);

    if (millis() - last > 2000)
    {
        last = millis();
        Serial.printf("loop: %s\n", initialised ? "initialised" : "not initialised");
    }
}
