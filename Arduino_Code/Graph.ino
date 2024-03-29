// Arduino nano "normal"
/*#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)
#define POINTS 300
#else if defined (__megaAVR_ATMEGA168__) || (__megaAVR_ATMEGA4809__)
#define POINTS 720
#endif*/

#define POINTS 300
#define MIN_WAVEFORM 0
#define MAX_WAVEFORM 255

byte allSent = 0;
byte data[3][POINTS];
byte extremum[2][2];
int putIndex = 0;
int sendFrom = 0;
int sendTo = 0;

/*void updateExtremum() {
  for (byte i = 0; i < 2; i++) {
    for (int j = sendFrom; j < sendTo; i++) {
      mini = min(mini, data[j % POINTS]);
      maxi = max(mini, data[j % POINTS]);
    }
  }
}*/

void saveData() {
  data[0][putIndex % POINTS] = mapFloat(tempIn, 20, 35, 0, 255);
  data[1][putIndex % POINTS] = mapFloat(tempTarget, 20, 35, 0, 255);
  data[2][putIndex % POINTS] = map(fanPower, 0, 320, 0, 255);
  putIndex++;
  sendTo = putIndex;
}

void sendGraph() {
  for (int i = sendFrom; i < sendTo; i++) {
    printAdd(0, data[0][i % POINTS]);
    printAdd(1, data[1][i % POINTS]);
    printAdd(2, data[2][i % POINTS]);
  }
  sendFrom = sendTo;
}
