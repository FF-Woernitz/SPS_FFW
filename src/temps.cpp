#include <MQTT.h>
#include <constants.h>




class Temps {
    private:
        //modbus_t ModbusQuery;
        //uint16_t ModbusSlaveRegisters[2];
        bool poll = false;


    public:
        Temps() {}

        void setup() {
            log("Setup");

           /* ModbusQuery.u8id = 1; // slave address
            ModbusQuery.u8fct = 3; // function code (this one is registers read)
            ModbusQuery.u16RegAdd = 0; // start address in slave
            ModbusQuery.u16CoilsNo = 2; // number of elements (coils or registers) to read
            ModbusQuery.au16reg = ModbusSlaveRegisters;
*/
        }

        void setupMQTT(MQTTClient &client) {
            log("setupMQTT");
        }

        void loop(MQTTClient &client, String ModbusClient) {
        if (!poll){
                //ModbusClient.query( ModbusQuery );
                poll = true;
            } else {
                /*ModbusClient.poll(); // check incoming messages
                if (ModbusClient.getState() == COM_IDLE) {
                    log((String) ModbusSlaveRegisters[0]);
                    log((String) ModbusSlaveRegisters[1]);
                    poll = false;
                }*/
            }
        }

    private:
        void log(String text) {
            Serial.println("Temps Test: " + text);
        }
};