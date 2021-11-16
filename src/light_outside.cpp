#include <MQTT.h>
#include <constants.h>

class LightOutside {
   private:
    String id;
    int pin_in;
    int pin_out;

    bool lastState;

    bool internalState;
    bool internalStateLast;

   public:
    LightOutside(const String id, const uint8_t *config) {
        this->id = id;

        this->pin_in = config[0];
        this->pin_out = config[1];
    }

    void setup(MQTTClient &client) {
        log("Setup");
        pinMode(this->pin_in, INPUT);
        pinMode(this->pin_out, OUTPUT);
        digitalWrite(this->pin_out, 0);
        internalState = false;

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));

        lastState = state();
        client.publish(getMQTTPath("internalstate"), "off");
        log("State: " + (lastState) ? "on" : "off");

        
        client.publish(getMQTTPath("state"), (lastState) ? "on" : "off");
    }

    void loop(MQTTClient &client) {
        if (lastState != state()) {
            lastState = state();
            client.publish(getMQTTPath("state"), (lastState) ? "on" : "off");
            if (lastState) {
                log("State: on");
            } else {
                log("State: off");
            }
        }
        
        if (internalState != internalStateLast) {
            internalStateLast =internalState;
            client.publish(getMQTTPath("internalstate"), (internalState) ? "on" : "off");
            if (lastState) {
                log("internalState: on");
            } else {
                log("internalState: off");
            }
        }
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

   private:
    bool state() {
        return (bool)digitalRead(this->pin_in);
    }

    void off() {
        if (state() && internalState) {
            digitalWrite(this->pin_out, 0);
            internalState = false;
            log("Output off");
        }
    }

    void on() {
        if (!state() && !internalState) {
            digitalWrite(this->pin_out, 1);
            internalState = true;
            log("Output on");
        }
    }

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