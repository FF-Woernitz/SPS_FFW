#include <MQTT.h>
#include <constants.h>

class Gate {
   private:
    int id;
    int pin_cmd_up;
    int pin_cmd_down;
    int pin_cmd_stop;
    int pin_state_up;
    int pin_state_down;

    int last_state_up;
    int last_state_down;
    unsigned long moving_start;
    bool moving_state;

   public:
    Gate(int id, const uint8_t *config) {
        this->id = id;

        this->pin_cmd_up = config[0];
        this->pin_cmd_down = config[1];
        this->pin_cmd_stop = config[2];
        this->pin_state_up = config[4];
        this->pin_state_down = config[5];
    }

    void setup(MQTTClient &client) {
        //setupAutodiscovery(client);
        Serial.println("Autodiscovery for gates is disabled!");

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));

        this->last_state_up = digitalRead(pin_state_up);
        this->last_state_down = digitalRead(pin_state_down);

        if (this->last_state_up == 1) {
            client.publish(getMQTTPath("state"), "open");
        } else if (this->last_state_down == 1) {
            client.publish(getMQTTPath("state"), "closed");
        } else {
            client.publish(getMQTTPath("state"), "stopped");
        }
    }

    void loop(MQTTClient &client) {
        int curent_state_up = digitalRead(pin_state_up);
        int curent_state_down = digitalRead(pin_state_down);

        //exit fast. Gate still on the limit switches.
        if ((curent_state_up == 1 && this->last_state_up == 1) || (curent_state_down == 1 && this->last_state_down == 1)) {
            return;
        }

        //Gate is and was not on the limit switches
        if ((curent_state_up == 0 && this->last_state_up == 0 && curent_state_down == 0 && this->last_state_down == 0)) {
            //Gate moving to long, without reaching a limit switch.
            if (this->moving_state && (millis() - this->moving_start > 3000)) {
                this->moving_state = false;
                client.publish(getMQTTPath("state"), "stopped");
            }
        }

        //Now opened
        if (curent_state_up == 1 && this->last_state_up == 0) {
            this->moving_state = false;
            client.publish(getMQTTPath("state"), "open");
        }

        //Now closed
        if (curent_state_down == 1 && this->last_state_down == 0) {
            this->moving_state = false;
            client.publish(getMQTTPath("state"), "closed");
        }

        //Now closing
        if (curent_state_up == 0 && this->last_state_up == 1) {
            this->moving_state = true;
            this->moving_start = millis();
            client.publish(getMQTTPath("state"), "closing");
        }

        //Now opening
        if (curent_state_down == 0 && this->last_state_down == 1) {
            this->moving_state = true;
            this->moving_start = millis();
            client.publish(getMQTTPath("state"), "opening");
        }

        this->last_state_up = curent_state_up;
        this->last_state_down = curent_state_down;
    }

    void message() {
    }

   private:
    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/gate" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    /*void setupAutodiscovery(MQTTClient &client) {
        StaticJsonDocument<256> doc;
        doc["name"] = "Gate " + (String)this->id;
        doc["unique_id"] = String(DEVICENAME) + "_gate" + (String)this->id;
        doc["cmd_t"] = getMQTTPath("cmd");
        doc["stat_t"] = getMQTTPath("state");
        doc["schema"] = "json";
        doc["brightness"] = true;

        String x;
        serializeJson(doc, x);
        Serial.println(x);

        Serial.println("Sending autodiscovery config of gate: " + (String)this->id);
        client.publish(getMQTTPath("config"), x);
        Serial.println("Sent autodiscovery config of gate: " + (String)this->id);
    }*/
};