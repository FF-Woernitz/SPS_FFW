#include <MQTT.h>
#include <constants.h>

#include <Bounce2.h>

class LightOutside
{
private:
    String id;
    int pin_in;
    int pin_out;

    bool state;
    bool nextState;

    bool lastButtonState;

    Bounce b = Bounce();

public:
    LightOutside(const String id, const uint8_t *config)
    {
        this->id = id;

        this->pin_in = config[0];
        this->pin_out = config[1];
    }

    void setup(MQTTClient &client)
    {
        log("Setup");

        b.attach(this->pin_in, INPUT);
        b.interval(50);

        pinMode(this->pin_out, OUTPUT);
        digitalWrite(this->pin_out, 0);
        state = false;
        nextState = false;

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));
        log("State: off");
        client.publish(getMQTTPath("state"), "off");

        lastButtonState = getInput();
    }

    void loop(MQTTClient &client)
    {
        b.update();
        if (b.rose())
        {
            log("Button pressed");
            nextState = !state;
        }

        if (state != nextState)
        {
            state = nextState;
            if (state)
            {
                client.publish(getMQTTPath("state"), "on");
                digitalWrite(this->pin_out, 1);
                log("State: on");
            }
            else
            {
                client.publish(getMQTTPath("state"), "off");
                digitalWrite(this->pin_out, 0);
                log("State: off");
            }
        }
    }

    void onMessage(String &payload)
    {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("ON"))
            nextState = true;
        else if (payload.equalsIgnoreCase("OFF"))
            nextState = false;
        else
            log("Unknown MQTT command!");
    }

private:
    bool getInput()
    {
        return (bool)digitalRead(this->pin_in);
    }

    String getMQTTPath(String suffix)
    {
        return String(DEVICENAME) + "/light_" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath()
    {
        return getMQTTPath("");
    }

    void log(String text)
    {
        Serial.println("Light " + this->id + ": " + text);
    }
};