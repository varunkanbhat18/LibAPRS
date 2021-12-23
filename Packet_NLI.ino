#include <LibAPRS.h>
#include <SoftwareSerial.h>

/* Define reference voltage the ADC */
#define ADC_REFERENCE REF_3V3
#define OPEN_SQUELCH false
#define SATCALL_SIGN "VU2CWN"
#define SATTELEM_SSID 11
#define APRS_TOCALL "CQ"

/* We need to include this function. It will get called by the library every time a packet is received, so we can process incoming packets. */
boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;

void aprs_msg_callback(struct AX25Msg *msg) {
  /* If we already have a packet waiting to be processed, we must drop the new one. */
  if (!gotPacket) {
    gotPacket = true;
    /* The memory referenced as *msg is volatile and we need to copy all the data to a local variable for later processing. */
    memcpy(&incomingPacket, msg, sizeof(AX25Msg));
    /* We need to allocate a new buffer for the data payload of the packet. First we check if there is enough free RAM. */
    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      gotPacket = false;
    }
  }
}

SoftwareSerial radiocom(15, 16);

void setup() {
  radiocom.begin(9600);
  Serial.begin(115200);
  /* Initialise APRS library. */
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  /* Callsign and SSID */
  APRS_setCallsign(SATCALL_SIGN, SATTELEM_SSID);
  /* Symbol we want to use: */
  APRS_setSymbol('O');
  /* Print all the settings */
  APRS_printSettings();
  Serial.print(F("Free RAM:     ")); 
  Serial.println(freeMemory());
}

void locationUpdateExample() {
  /* Set the latitude and longtitude. */
  APRS_setLat("5530.80N");
  APRS_setLon("01143.89E");
  /* Define a comment string */
  char* comment = (char*)"LibAPRS location update";
  /* Send the update */
  APRS_sendLoc(comment, strlen(comment));
}

void messageExample() {
  /* First we need to set the message recipient */
  APRS_setMessageDestination(APRS_TOCALL, 0);
  /* Define a string to send */
  char *message = (char*)"Hello !!";
  APRS_sendMsg(message, strlen(message));
}

/* Function to process incoming packets Remember to call this function often, so we won't miss any packets due to one already waiting to be processed */
void processPacket() {
  if (gotPacket) {
    gotPacket = false;
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));

    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
    }
    Serial.println("");
    /* Remember to free memory for buffer! */
    free(packetData);
  }
}

boolean whichExample = false;
void loop() {
  delay(1000);
  if (whichExample) {
    locationUpdateExample();
  } else {
    messageExample();
  }
  whichExample ^= true;
  delay(500);
  processPacket();
}
