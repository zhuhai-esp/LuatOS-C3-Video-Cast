#include "LuatOS_C3.h"
#include <Arduino.h>
#include <TJpg_Decoder.h>

TFT_eSPI tft = TFT_eSPI();
char buf[32] = {0};
unsigned long lastMs = 0;
long check1s = 0;
int serverPort = 8888;
WiFiServer server(serverPort);
u8_t *netBuf = NULL;
u8_t *bufEnd = NULL;
int tftSize = TFT_WIDTH * TFT_HEIGHT;
int bufSize = tftSize * 6;

bool onJpgDecoded(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bmp) {
  for (int i = 0; i < w * h; i++) {
    u16_t code = bmp[i];
    bmp[i] = (code >> 8 | code << 8);
  }
  tft.pushImage(x, y, w, h, bmp);
  return true;
}

void setup() {
  disableCore0WDT();
  Serial.begin(115200);
  Serial.println("Hello ESP32C3!!");
  initTFT();
  initLEDs();
  tft.println("Start Config WiFi!");
  autoConfigWifi();
  tft.println("Wifi Connected!");
  String ip = WiFi.localIP().toString();
  sprintf(buf, "%s:%d", ip.c_str(), serverPort);
  tft.println(buf);
  configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com");
  delay(2000);
  server.begin();
  netBuf = (u8_t *)malloc(bufSize);
  bufEnd = netBuf + bufSize;
  TJpgDec.setCallback(onJpgDecoded);
}

void inline readTCPJpeg() {
  WiFiClient client = server.available();
  if (client) {
    u8_t *pRead = netBuf;
    u8_t *pDecode = netBuf;
    while (client.connected()) {
      if (client.available()) {
        int capable = bufEnd - pRead;
        int batch = client.read(pRead, capable);
        if (batch == capable) {
          int copySize = bufEnd - pDecode;
          memcpy(netBuf, pDecode, copySize);
          pDecode = netBuf;
          pRead = netBuf + copySize;
        } else {
          pRead += batch;
        }
        if (pRead - pDecode > 1) {
          u8_t *pTail = pDecode;
          for (u8_t *p = pDecode; p < pRead - 1; p++) {
            u8_t *pFEnd = p + 1;
            u8_t *pFNext = p + 2;
            if ((*p) == 0xff && (*pFEnd) == 0xd9) {
              TJpgDec.drawJpg(0, 0, pTail, pFNext - pTail);
              pTail = pFNext;
            }
          }
          pDecode = pTail;
        }
      }
    }
  }
}

void loop() { readTCPJpeg(); }