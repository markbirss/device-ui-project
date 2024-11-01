#ifndef PIO_UNIT_TESTING

#include "Arduino.h"
#include "DeviceScreen.h"
#include "Log.h"
#include "UARTClient.h"

// this is pulled in by the device-ui library
const char *firmware_version = "2.5.7";
static char connectionString[40];

#ifdef USE_DUMMY_SERIAL
class DummyClient : public IClientBase
{
  public:
    DummyClient() = default;
    void init(void) override {}
    bool connect(void) override { return true; }
    bool disconnect(void) override { return true; }
    bool isConnected(void) override { return false; }
    bool send(meshtastic_ToRadio &&to) override { return false; }
    meshtastic_FromRadio receive(void) override
    {
        meshtastic_FromRadio dummy{};
        return dummy;
    }
    ~DummyClient(){};
    bool isActive(void) const { return false; }
    const char *getConnectionInfo(void) const { return "<undefined>"; }
} serial;
#else
UARTClient serial;
#endif

DeviceScreen *screen = nullptr;

void setup()
{
#ifdef KB_POWERON
    digitalWrite(KB_POWERON, HIGH);
    pinMode(KB_POWERON, OUTPUT);
    delay(200); // wait until keyboard mcu startup finished
#endif

#ifdef PWR_ON_PIN
    pinMode(PWR_ON_PIN, OUTPUT);
    digitalWrite(PWR_ON_PIN, HIGH);
#endif

#ifdef PWR_EN_PIN
    pinMode(PWR_EN_PIN, OUTPUT);
    digitalWrite(PWR_EN_PIN, HIGH);
#endif

#ifndef USE_SERIAL0
#ifdef WAIT_FOR_SERIAL0
    delay(2000);
#endif
    Serial.begin(115200);
#ifdef WAIT_FOR_SERIAL0
    time_t timeout = millis();
    while (!Serial && (millis() - timeout) < 2000)
        ;
#endif
    logger.setDebugLevel(ESP_LOG_VERBOSE); // use ESP_LOG_VERBOSE for trace category
#else
    logger.setDebugLevel(ESP_LOG_NONE); // do not log when connected over serial0
#endif

    ILOG_DEBUG("\n*** EEZ-Studio (LovyanGFX) TFT GUI ***\n");
#ifdef ARDUINO_ARCH_ESP32
    uint64_t chipid;
    chipid = ESP.getEfuseMac(); // The chip ID is essentially its MAC address(length: 6 bytes).
    ILOG_DEBUG("  ESP32 Chip ID = %04X\n", (uint16_t)(chipid >> 32)); // print High 2 bytes
    ILOG_DEBUG("%08X\n\r", (uint32_t)chipid);                         // print Low 4bytes.
    ILOG_DEBUG("  Flash size: %8d bytes\n\r", ESP.getFlashChipSize());
    ILOG_DEBUG("  Heap size : %8d bytes\n\r", ESP.getHeapSize());
    ILOG_DEBUG("  Free heap : %8d bytes\n\r", ESP.getFreeHeap());
    ILOG_DEBUG("  PSRAM     : %8d bytes\n\r", ESP.getFreePsram());
    ILOG_DEBUG("*****************************************\n");
#endif

    screen = &DeviceScreen::create();
    screen->init(&serial);
    sprintf(connectionString, "==> connect %s !!!", serial.getConnectionInfo());

#ifdef ARDUINO_ARCH_ESP32
    ILOG_DEBUG("Free heap : %8d bytes\n\r", ESP.getFreeHeap());
    ILOG_DEBUG("PSRAM     : %8d bytes\n\r", ESP.getFreePsram());
#endif

    ILOG_DEBUG("Setup done.\n\r");
}

/*** main loop ***/
void loop()
{
    if (serial.isActive()) {
        firmware_version = "Connected!";
    } else {
        firmware_version = connectionString;
    }
    screen->task_handler();
    delay(5);
}

#if defined(ARCH_PORTDUINO)
void tft_task_handler(void *)
{
    screen->task_handler();
    delay(5);
}
#endif

#endif