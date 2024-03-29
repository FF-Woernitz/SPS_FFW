#include <Bounce2.h>
#include <MQTT.h>
#include <constants.h>

class LightButton {
   private:
    String id;
    int pin_in;
    int pin_out;

    bool state;
    bool nextState;

    Bounce b = Bounce();

   public:
    LightButton(const String id, const uint8_t *config) {
        this->id = id;

        this->pin_in = config[0];
        this->pin_out = config[1];
    }

    void setup() {
        log("Setup");

        b.attach(this->pin_in, INPUT);
        b.interval(50);

        pinMode(this->pin_out, OUTPUT);
        digitalWrite(this->pin_out, 0);
        state = false;
        nextState = false;
    }

    void setupMQTT(MQTTClient &client) {
        log("setupMQTT");

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));
        publishState(client, state ? "on" : "off");
    }

    void loop(MQTTClient &client) {
        b.update();
        if (b.rose()) {
            log("Button pressed");
            nextState = !state;
        }

        if (state != nextState) {
            state = nextState;
            publishState(client, state ? "on" : "off");
            digitalWrite(this->pin_out, state ? 1 : 0);
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

    void publishState(MQTTClient &client, String payload) {
        client.publish(getMQTTPath("state"), payload, true, 0);
        log("MQTT Publish: " + payload);
    }
};