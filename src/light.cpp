#include <MQTT.h>
#include <constants.h>

class Light {
   protected:
    String id;
    int pin_in;
    int pin_out;

   public:
    Light(const String id, const uint8_t *config) {
        this->id = id;

        this->pin_in = config[0];
        this->pin_out = config[1];
    }

    void setup(MQTTClient &client) {
        log("Setup");
        pinMode(this->pin_in, INPUT);
        pinMode(this->pin_out, OUTPUT);
        digitalWrite(this->pin_out, 0);

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));

        client.publish(getMQTTPath("state"), (state()) ? "on" : "off");
    }

    void loop(MQTTClient &client) {
        client.publish(getMQTTPath("state"), (state()) ? "on" : "off");
    }

    void onMessage(String &payload) {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("ON"))
            on();
        else if (payload.equalsIgnoreCase("OFF"))
            off();
        else
            log("Unknown MQTT command!");
    }

   protected:
    void on() {
        log("Not implementet");
    };
    void off() {
        log("Not implementet");
    };
    bool state() {
        log("Not implementet");
        return false;
    };

    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/light_" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    void log(String text) {
        Serial.println("Light " + this->id + ": " + text);
    }
};