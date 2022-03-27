#include <MQTT.h>
#include <constants.h>

class Input {
   private:
    String id;
    int pin_in;
    int pin_out;

    bool state;
    bool lastInputState;

   public:
    Input(const String id, const uint8_t pin_in) {
        this->id = id;

        this->pin_in = pin_in;
    }

    void setup(MQTTClient &client) {
        log("Setup");
        pinMode(this->pin_in, INPUT);

        state = lastInputState = getState();
        publishState(client);
    }

    void loop(MQTTClient &client) {
        state = getState();

        if (state != lastInputState) {
            lastInputState = state;
            publishState(client);
        }
    }

   private:
    bool getState() {
        return (bool)digitalRead(this->pin_in);
    }

    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/input_" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    void log(String text) {
        Serial.println("Input " + this->id + ": " + text);
    }

    void publishState(MQTTClient &client) {
        if (state) {
            digitalWrite(this->pin_out, 1);
            client.publish(getMQTTPath("state"), "on", true, 0);
            log("State: on");
        } else {
            digitalWrite(this->pin_out, 0);
            client.publish(getMQTTPath("state"), "off", true, 0);
            log("State: off");
        }
    }
};