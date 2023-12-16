#include <CircularBuffer.h>
#include <MQTT.h>
#include <constants.h>

class Gate {
   private:
    int id;
    int pin_cmd_up;
    int pin_cmd_down;
    int pin_cmd_stop;
    int pin_cmd_warn;
    int pin_state_up;
    int pin_state_down;

    int last_state_up;
    int last_state_down;
    unsigned long moving_start;
    bool moving_state;

    enum CMDs { stop_0,
                stop_1,
                warn_0,
                warn_1,
                up_0,
                up_1,
                down_0,
                down_1 };
    CircularBuffer<CMDs, 6> cmd_q;
    unsigned long cmd_start;

   public:
    Gate(int id, const uint8_t *config) {
        this->id = id;

        this->pin_cmd_up = config[0];
        this->pin_cmd_down = config[1];
        this->pin_cmd_stop = config[2];
        this->pin_cmd_warn = config[3];
        this->pin_state_up = config[4];
        this->pin_state_down = config[5];
    }

    void setup(MQTTClient &client) {
        log("SETUP");
        pinMode(this->pin_cmd_up, OUTPUT);
        pinMode(this->pin_cmd_down, OUTPUT);
        pinMode(this->pin_cmd_stop, OUTPUT);
        pinMode(this->pin_cmd_warn, OUTPUT);
        pinMode(this->pin_state_up, INPUT);
        pinMode(this->pin_state_down, INPUT);
        resetCmd();

        //setupAutodiscovery(client);
        Serial.println("Autodiscovery for gates is disabled!");

        client.publish(getMQTTPath("cmd"), "-");
        client.subscribe(getMQTTPath("cmd"));

        this->last_state_up = digitalRead(pin_state_up);
        this->last_state_down = digitalRead(pin_state_down);

        if (this->last_state_down == 1) {
            publishState(client, "closed");
        } else {
            publishState(client, "open");
        }
    }

    void loop(MQTTClient &client) {
        loopCmd();

        int curent_state_up = digitalRead(pin_state_up);
        int curent_state_down = digitalRead(pin_state_down);

        //exit fast. Gate still on the limit switches.
        if ((curent_state_up == 1 && this->last_state_up == 1) || (curent_state_down == 1 && this->last_state_down == 1)) {
            return;
        }

        //Gate is and was not on the limit switches
        if ((curent_state_up == 0 && this->last_state_up == 0 && curent_state_down == 0 && this->last_state_down == 0)) {
            //Gate moving to long, without reaching a limit switch.
            if (this->moving_state && (millis() - this->moving_start > GATE_MOVING_TIME)) {
                this->moving_state = false;
                log("Timeout. Pubish open");
                publishState(client, "open");
            }
        }

        //Now opened
        if (curent_state_up == 1 && this->last_state_up == 0) {
            this->moving_state = false;
            publishState(client, "open");
        }

        //Now closed
        if (curent_state_down == 1 && this->last_state_down == 0) {
            this->moving_state = false;
            publishState(client, "closed");
        }

        //Now closing
        if (curent_state_up == 0 && this->last_state_up == 1) {
            this->moving_state = true;
            this->moving_start = millis();
            publishState(client, "closing");
        }

        //Now opening
        if (curent_state_down == 0 && this->last_state_down == 1) {
            this->moving_state = true;
            this->moving_start = millis();
            publishState(client, "opening");
        }

        this->last_state_up = curent_state_up;
        this->last_state_down = curent_state_down;
    }

    void onMessage(String &payload) {
        log("MQTT command: " + payload);

        if (payload.equalsIgnoreCase("STOP"))
            stop(true);
        else if (payload.equalsIgnoreCase("UP"))
            up();
        else if (payload.equalsIgnoreCase("DOWN"))
            down();
        else
            log("Unknown MQTT command!");
    }

   private:
    String getMQTTPath(String suffix) {
        return String(DEVICENAME) + "/gate" + (String)this->id + "/" + suffix;
    }

    String getMQTTPath() {
        return getMQTTPath("");
    }

    void log(String text) {
        Serial.println("Gate " + (String)this->id + ": " + text);
    }

    void publishState(MQTTClient &client, String payload){
        client.publish(getMQTTPath("state"), payload, true, 0);
        log("MQTT Publish: " + payload);
    }

    void up() {
        if(this->last_state_up == 1){
            log("Gate already up. Ignoring...");
            return;
        }
        stop(true);
        cmd_q.push(up_1);
        cmd_q.push(up_0);
    }

    void down() {
        if(this->last_state_down == 1){
            log("Gate already down. Ignoring...");
            return;
        }
        stop(true);
        warning();
        cmd_q.push(down_1);
        cmd_q.push(down_0);
    }

    void stop(bool reset) {
        if (reset) resetCmd();
        cmd_q.push(stop_1);
        cmd_q.push(stop_0);
    }

    void warning() {
        if (GATE_WARNING_TIME != 0) {
            cmd_q.push(warn_1);
            cmd_q.push(warn_0);
            stop(true);
        }
    }

    void resetCmd() {
        cmd_start = 0;
        cmd_q.clear();
        digitalWrite(this->pin_cmd_up, 0);
        digitalWrite(this->pin_cmd_down, 0);
        digitalWrite(this->pin_cmd_stop, 0);
        digitalWrite(this->pin_cmd_warn, 0);
        log("Output RESET");
    }

    void loopCmd() {
        if (cmd_q.isEmpty())
            return;
        if (cmd_start != 0 && millis() - cmd_start < GATE_CMD_DELAY)
            return;
        if (cmd_q.first() == warn_0 && millis() - cmd_start < GATE_WARNING_TIME)
            return;

        switch (cmd_q.shift()) {
            case stop_0:
                digitalWrite(this->pin_cmd_stop, 0);
                log("Output Stop: 0");
                break;
            case stop_1:
                digitalWrite(this->pin_cmd_stop, 1);
                log("Output Stop: 1");
                break;
            case down_0:
                digitalWrite(this->pin_cmd_down, 0);
                log("Output Down: 0");
                break;
            case down_1:
                digitalWrite(this->pin_cmd_down, 1);
                log("Output Down: 1");
                break;
            case up_0:
                digitalWrite(this->pin_cmd_up, 0);
                log("Output Up: 0");
                break;
            case up_1:
                digitalWrite(this->pin_cmd_up, 1);
                log("Output Up: 1");
                break;
            case warn_0:
                digitalWrite(this->pin_cmd_warn, 0);
                log("Output Warn: 0");
                break;
            case warn_1:
                digitalWrite(this->pin_cmd_warn, 1);
                log("Output Warn: 1");
                break;

            default:
                break;
        }

        cmd_start = millis();
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