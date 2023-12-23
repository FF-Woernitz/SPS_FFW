#include <Bounce2.h>
#include <MQTT.h>
#include <constants.h>

class Input {
   private:
    String id;
    int pin_in;

    Bounce b = Bounce();

   public:
    Input(const String id, const uint8_t pin_in) {
        this->id = id;
        this->pin_in = pin_in;
    }

    void setup() {
        log("Setup");

        b.attach(this->pin_in, INPUT);
        b.interval(50);
    }

    void setupMQTT(MQTTClient &client) {
        log("setupMQTT");

        publishState(client, b.read() ? "on" : "off");
    }

    void loop(MQTTClient &client) {
        b.update();
        if (b.rose()) {
            publishState(client, "on");
        }
        if (b.fell()) {
            publishState(client, "off");
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

    void publishState(MQTTClient &client, String payload) {
        client.publish(getMQTTPath("state"), payload, true, 0);
        log("MQTT Publish: " + payload);
    }
};