#include <bluefruit.h>
#include "Adafruit_TinyUSB.h"

// --- SETTINGS ---
BLEClientUart clientUart; // We still need this for the connection later

// --- SHARED DATA ---
volatile bool packetAvailable = false;
uint8_t       packetData[32];
uint8_t       packetLen;
ble_gap_addr_t packetAddr; // We need the full address struct to connect


const uint8_t TARGET_UUID[] = { 
  0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5, 0xA9, 0xE0, 
  0x93, 0xF3, 0xA3, 0xB5, 0x01, 0x00, 0x40, 0x6E 
};

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) delay(10);

  Serial.println("--- Relay: Manual Loop Matching ---");

  Bluefruit.begin(0, 1);
  Bluefruit.setName("Relay_Central");
  
  clientUart.begin(); 

  Bluefruit.Scanner.setRxCallback(scan_callback);
  Bluefruit.Scanner.restartOnDisconnect(true);
  Bluefruit.Scanner.setInterval(160, 80); 
  Bluefruit.Scanner.useActiveScan(true);  
  
  Serial.println("Scanning...");
  Bluefruit.Scanner.start(0);
}

void loop() {
  if (packetAvailable) {
    
    if (checkPacketForUUID()) {
      Serial.println(">>> MATCH FOUND in Loop! <<<");
      Serial.print("Target Address: ");
      Serial.printBufferReverse(packetAddr.addr, 6, ':');
      Serial.println();

      Bluefruit.Scanner.stop();

      Serial.println("Connecting...");
      Bluefruit.Central.connect(&packetAddr);
      
      packetAvailable = false;
      return;
    } 
    else {
      Bluefruit.Scanner.resume();
      packetAvailable = false; 
    }
  }
}

bool checkPacketForUUID() {
  if (packetLen < 16) return false;

  for (int i = 0; i <= packetLen - 16; i++) {
    // Compare memory at index 'i' with our target UUID
    if (memcmp(&packetData[i], TARGET_UUID, 16) == 0) {
      return true; // Found it!
    }
  }
  return false;
}

// --- CALLBACK: Dumb and Fast ---
void scan_callback(ble_gap_evt_adv_report_t* report) {

  if (packetAvailable) {
    Bluefruit.Scanner.resume();
    return;
  }

  packetAddr = report->peer_addr;

  packetLen = report->data.len;
  if (packetLen > 31) packetLen = 31;
  memcpy(packetData, report->data.p_data, packetLen);


  packetAvailable = true;
}