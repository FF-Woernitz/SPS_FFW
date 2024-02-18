#include <MQTT.h>
#include <constants.h>

class Output {
   private:
    String id;
    int pin;
    int def;

    int nextTask = -1;

   public:
    Output(const String id, const uint8_t *config) {
        this->id = id;
        this->pin = config[0];
        this->def = config[1];
    }

    void setup() {
        log("Setup");

        pinMode(this->pin, OUTPUT);
        digitalWrite(this->pin, def);
    }

    void setupMQTT(MQTTClient &client) {
        log("setupMQTT");

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));

        publishState(client, this->def ? "on" : "off");
    }

    void loop(MQTTClient &client) {
        if(nextTask == 0){
            digitalWrite(this->pin, false);
            publishState(client, "off");
        } else if (nextTask == 1){
            digitalWrite(this->pin, true);
            publishState(client, "on");
        }
        nextTask = -1;
    }

    void onMessage(String &payload) {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("on"))
            nextTask = 1;
        else if (payload.equalsIgnoreCase("off"))
            nextTask = 0;
        else
            log("Unknown MQTT command!");
    }

   private:

    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/output_" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    void log(String text) {
        Serial.println("Output " + this->id + ": " + text);
    }

    void publishState(MQTTClient &client, String payload) {
        client.publish(getMQTTPath("state"), payload, true, 0);
        log("MQTT Publish: " + payload);
    }
};