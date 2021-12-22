#include <MQTT.h>
#include <constants.h>

class LightOutside {
   private:
    String id;
    int pin_in;
    int pin_out;

    bool state;
    bool nextState;

    bool lastButtonState;

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
        state = false;
        nextState = false;

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));
        log("State: off");
        client.publish(getMQTTPath("state"), "off");

        lastButtonState = getState();
    }

    void loop(MQTTClient &client) {
        if (lastButtonState != getState()) {
            lastButtonState = getState();
            log("Button pressed");
            nextState = !state;
        }

        if (state != nextState) {
            state = nextState;
            client.publish(getMQTTPath("state"), (state) ? "on" : "off");
            log("State: " + (state) ? "on" : "off");
            if (state) {
                digitalWrite(this->pin_out, 1);
            } else {
                digitalWrite(this->pin_out, 0);
            }
        }
    }

    void onMessage(String &payload) {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("ON"))
            nextState = true;
        else if (payload.equalsIgnoreCase("OFF"))
            nextState = false;
        else
            log("Unknown MQTT command!");
    }

   private:
    bool getState() {
        return (bool)digitalRead(this->pin_in);
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