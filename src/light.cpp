#include <MQTT.h>
#include <constants.h>

class Light{
    private:
        String id;
        int pin_in;
        int pin_out;


    public:
    Light(String id, const uint8_t *config){
        this->id = id;

        this->pin_in = config[0];
        this->pin_out = config[1];
    }

    void setup(MQTTClient &client){
        log("Setup");
        pinMode(this->pin_in, INPUT);
        pinMode(this->pin_out, OUTPUT);
        digitalWrite(this->pin_out, 0);


        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));
    }

    void loop(MQTTClient &client){

    }

        void onMessage(String &payload) {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("ON"))
            on();
        else if (payload.equalsIgnoreCase("OFF"))
            off();
        else if (payload.equalsIgnoreCase("ALARM"))
            alarm();
        else
            log("Unknown MQTT command!");
    }


    private:

    virtual void on();
    virtual void off();
    virtual void alarm();
    virtual int state();

    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/light_" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    void log(String text){
        Serial.println("Light " + this->id + ": " + text);
    }

}