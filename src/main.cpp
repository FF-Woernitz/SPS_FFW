#include <Arduino.h>
#include <Controllino.h>
#include <Ethernet.h>
#include <MQTT.h>
#include <ModbusRtu.h>


#include <constants.h>
#include <main.h>

#include <gate.cpp>
#include <input.cpp>
#include <output.cpp>
#include <light_button.cpp>

#define MasterModbusAdd 0
#define RS485Serial 3


EthernetClient net;
MQTTClient client;

Gate gate1(1, GATE_1_CONFIG);
Gate gate2(2, GATE_2_CONFIG);
Gate gate3(3, GATE_3_CONFIG);
Gate gate4(4, GATE_4_CONFIG);
Gate gate5(5, GATE_5_CONFIG);

LightButton light_outside("outside", LIGHT_OUTSIDE_CONFIG);

Input input_light_inside_on("light_inside_on", INPUT_LIGHT_INSIDE_ON);
Input input_light_inside_off("light_inside_off", INPUT_LIGHT_INSIDE_OFF);

Output output_heating_front("heating_front", OUTPUT_HEATING_FRONT);
Output output_heating_back("heating_back", OUTPUT_HEATING_BACK);

Modbus ControllinoModbusMaster = Modbus(MasterModbusAdd, RS485Serial, 0);
modbus_t ModbusQuery[4];
modbus_t ModbusSetId;
uint16_t ModbusSlaveRegisters[2];
int poll = 0;
int device = 0;

unsigned long lastConnectTry = 0;
unsigned long lasttimemodbus = 0;

bool connect(bool initialConnect) {
    if (!client.connected()) {
        #ifdef DEBUG
            Serial.println("RE | " + (String) millis() + " | " + (String) lastConnectTry + " | " + (String) (millis() - lastConnectTry));
        #endif
        if (!initialConnect && (millis() - lastConnectTry) < 5000) {
            return false;
        }
        lastConnectTry = millis();

        Serial.print("connecting...");

        static char lwt[30] = {"\0"};
        strncpy(lwt, DEVICENAME, sizeof(lwt) - 1);
        strncat(lwt, "/status", sizeof(lwt) - strlen(lwt) - 1);
        client.setWill(lwt, "offline", true, 1);

        client.setKeepAlive(10);
        client.connect(DEVICENAME, MQTT_USER, MQTT_PASS);
        if (client.connected()) {
            client.publish(lwt, "online", true, 1);
            Serial.println("connected!");
            initMqtt();
            return true;
        } else {
            Serial.println("failed!");
            return false;
        }
    }
    return true;
}

void initConnect() {
    for (int i = 0; i < 10; i++) {
        Serial.println("Inital connection. Try " + (String)i + "/10");
        if (connect(true)) {
            break;
        }
    }
}

void initMqtt() {
    gate1.setupMQTT(client);
    gate2.setupMQTT(client);
    gate3.setupMQTT(client);
    gate4.setupMQTT(client);
    gate5.setupMQTT(client);

    light_outside.setupMQTT(client);

    input_light_inside_on.setupMQTT(client);
    input_light_inside_off.setupMQTT(client);

    output_heating_front.setupMQTT(client);
    output_heating_back.setupMQTT(client);
}

void messageReceived(String &topic, String &payload) {
    Serial.println("incoming: " + topic + " - " + payload);
    // Note: Do not use the client in the callback to publish, subscribe or
    // unsubscribe as it may cause deadlocks when other things arrive while
    // sending and receiving acknowledgments. Instead, change a global variable,
    // or push to a queue and handle it in the loop after calling `client.loop()`.

    String device = topic.substring(0, topic.length() - 4).substring(sizeof(DEVICENAME));

    if (device == "gate1")
        gate1.onMessage(payload);
    else if (device == "gate2")
        gate2.onMessage(payload);
    else if (device == "gate3")
        gate3.onMessage(payload);
    else if (device == "gate4")
        gate4.onMessage(payload);
    else if (device == "gate5")
        gate5.onMessage(payload);
    else if (device == "light_outside")
        light_outside.onMessage(payload);
    else if (device == "output_heating_front")
        output_heating_front.onMessage(payload);
    else if (device == "output_heating_back")
        output_heating_back.onMessage(payload);
}

    void log(String text) {
        Serial.println("MAIN: " + text);
    }

void setup() {
    Serial.begin(115200);
    Serial.println("Name: " + (String)DEVICENAME);
    Serial.println("Version: " + VERSION);
    Serial.println("Builddate: " + (String)STR(BUILD_DATE));

    Ethernet.begin(MAC, IPAddress(IP));

    client.begin(IPAddress(MQTT_IP), MQTT_PORT, net);
    client.onMessage(messageReceived);

    ControllinoModbusMaster.begin( 9600 );
    ControllinoModbusMaster.setTimeOut( 5000 );

    gate1.setup();
    gate2.setup();
    gate3.setup();
    gate4.setup();
    gate5.setup();

    light_outside.setup();

    input_light_inside_on.setup();
    input_light_inside_off.setup();

    output_heating_front.setup();
    output_heating_back.setup();

    ModbusQuery[0].u8id = 10;
    ModbusQuery[0].u8fct = 4;
    ModbusQuery[0].u16RegAdd = 1;
    ModbusQuery[0].u16CoilsNo = 2;
    ModbusQuery[0].au16reg = ModbusSlaveRegisters;

    ModbusQuery[1] = ModbusQuery[0];
    ModbusQuery[1].u8id = 11;
    ModbusQuery[2] = ModbusQuery[0];
    ModbusQuery[2].u8id = 12;
    ModbusQuery[3] = ModbusQuery[0];
    ModbusQuery[3].u8id = 13;

    initConnect();
}

void loop() {
    connect(false);
    client.loop();
    gate1.loop(client);
    gate2.loop(client);
    gate3.loop(client);
    gate4.loop(client);
    gate5.loop(client);

    light_outside.loop(client);

    input_light_inside_on.loop(client);
    input_light_inside_off.loop(client);

    output_heating_front.loop(client);
    output_heating_back.loop(client);


    //Modbus Start, needs to be moved to extra Class. I love C++
    if (poll == 0){
        if(device >= 4){
            device = 0;
        }
        if(millis() - lasttimemodbus > 5000){
            poll = 1;
            lasttimemodbus = millis();
        }
    } else if (poll == 1){
        ControllinoModbusMaster.query( ModbusQuery[device] );
        poll = 2;
    } else {
        ControllinoModbusMaster.poll(); // check incoming messages
        if (ControllinoModbusMaster.getState() == COM_IDLE) {
            #ifdef DEBUG
            log((String) "Temp " + device + ": " + (ModbusSlaveRegisters[0] / 10.0F) + "°C");
            log((String) "Humd " + device + ": " + (ModbusSlaveRegisters[1] / 10.0F) + "%");
            #endif
            client.publish(String(DEVICENAME) + "/sensor" + (String)ModbusQuery[device].u8id  + "/temp", (String)(ModbusSlaveRegisters[0] / 10.0F), true, 0);
            client.publish(String(DEVICENAME) + "/sensor" + (String)ModbusQuery[device].u8id  + "/humd", (String)(ModbusSlaveRegisters[1] / 10.0F), true, 0);

            poll = 0;
            device++;
        }
    }
}