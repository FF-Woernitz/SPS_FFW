#include <Arduino.h>
#include <Controllino.h>
#include <Ethernet.h>
#include <MQTT.h>
#include <constants.h>
#include <main.h>

#include <gate.cpp>

byte MAC[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress IP = IPAddress(10, 0, 110, 240);

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

        client.connect("arduino", "public", "public");
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
}

void setup() {
    Serial.begin(115200);
    Ethernet.begin(MAC, IP);

    client.begin("10.0.130.10", net);
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

    delay(5);
}