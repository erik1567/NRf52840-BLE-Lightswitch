#include "Adafruit_TinyUSB.h"
#include <bluefruit.h>    // Adafruit nRF52 Bluefruit core BLE API

// BLE objects
BLEDis bledis;                     // Device Information Service
BLEUart bleuart;                   // Nordic UART Service (NUS)-like

void setup() {
  // --- USB Serial ---
  while(!Serial) delay(10);
  Serial.begin(115200);
  // Give USB time to enumerate (especially after reset)
  delay(2000);
  Serial.println("Starting BLE UART example...");

  // --- Initialize Bluefruit ---
  Bluefruit.begin();
  Bluefruit.setName("Supermini_UART");   // Name shown in nRF Connect

  // Set maximum power and connection settings (optional but nice)
  Bluefruit.setTxPower(4);               // +4 dBm
  Bluefruit.Periph.setConnInterval(6, 12);  // 7.5-15 ms

  // --- Device Information Service (optional) ---
  bledis.setManufacturer("MyCompany");
  bledis.setModel("Supermini nRF52840");
  bledis.begin();

  // --- UART-like Service ---
  bleuart.begin();

  // --- Set up advertising ---
  startAdv();

  Serial.println("BLE UART is ready. Open nRF Connect, connect, and write data.");
}

void loop() {
  // If not connected: do nothing special
  if (!Bluefruit.connected()) {
    return;
  }

  /*// Check if there is data from the phone
  while (bleuart.available()) {
    int c = bleuart.read();
    // Print received bytes to Serial Monitor
    Serial.write(c);
  }*/
    while (Serial.available())
  {
    // Delay to wait for enough input, since we have a limited transmission buffer
    delay(2);

    uint8_t buf[64];
    int count = Serial.readBytes(buf, sizeof(buf));
    bleuart.write( buf, count );
  }

  // Small delay to avoid busy-waiting
  delay(5);
}

void startAdv() {
  // Advertising packet
  Bluefruit.Advertising.stop();
  Bluefruit.ScanResponse.clearData();

  // Advertise as general discoverable, no BR/EDR
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  // Advertise the UART service UUID
  Bluefruit.Advertising.addService(bleuart);

  // Include name in the scan response
  Bluefruit.ScanResponse.addName();

  // Advertising parameters
  Bluefruit.Advertising.setInterval(32, 244);  // 20 ms to 152.5 ms
  Bluefruit.Advertising.setFastTimeout(30);    // fast mode for 30 seconds
  Bluefruit.Advertising.start(0);              // 0 = advertise forever

  Serial.println("Advertising started.");
}
void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
}