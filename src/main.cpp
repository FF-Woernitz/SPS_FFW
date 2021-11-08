#include <Arduino.h>
#include <Controllino.h>
#include <Ethernet.h>
#include <MQTT.h>
#include <constants.h>
#include <main.h>

#include <gate.cpp>
#include <light.cpp>

EthernetClient net;
MQTTClient client;
Gate gate1(1, GATE_1_CONFIG);
Gate gate2(2, GATE_2_CONFIG);
Gate gate3(3, GATE_3_CONFIG);
Gate gate4(4, GATE_4_CONFIG);
Gate gate5(5, GATE_5_CONFIG);

unsigned long lastMillis = 0;

bool connect() {
    if (!client.connected()) {
        Serial.println("connecting...");

        static char lwt[30] = {"\0"};
        strncpy(lwt, DEVICENAME, sizeof(lwt) - 1);
        strncat(lwt, "/LWT", sizeof(lwt) - strlen(lwt) - 1);
        client.setWill(lwt, "offline", true, 1);

        client.setKeepAlive(10);
        client.connect(DEVICENAME, MQTT_USER, MQTT_PASS);
        if (client.connected()) {
            initMqtt();
            client.publish(lwt, "online", true, 1);
            Serial.println("connected!");
            return true;
        } else {
            return false;
        }
    }
    return true;
}

void initConnect() {
    for (int i = 0; i < 10; i++) {
        Serial.println("Inital connection. Try " + (String)i + "/10");
        if (connect())
            break;
    }
}

void initMqtt() {
    gate1.setup(client);
    gate2.setup(client);
    gate3.setup(client);
    gate4.setup(client);
    gate5.setup(client);
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
}

void setup() {
    Serial.begin(115200);
    Ethernet.begin(MAC, IPAddress(IP));

    client.begin(IPAddress(MQTT_IP), MQTT_PORT, net);
    client.onMessage(messageReceived);

    initConnect();
}

void loop() {
    connect();
    client.loop();
    gate1.loop(client);
    gate2.loop(client);
    gate3.loop(client);
    gate4.loop(client);
    gate5.loop(client);
}