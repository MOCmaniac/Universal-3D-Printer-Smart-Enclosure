#define MIN_WAVEFORM 0
#define MAX_WAVEFORM 255
#define POINTS 450

byte data[2][POINTS];
int putIndex = 0;
int sendFrom = 0;
int sendTo = 0;

void saveData() {
  //mini = min(mini, data[0][putIndex]);
  //mini = max(maxi, data[0][putIndex]);
  data[0][putIndex%POINTS] = mapFloat(tempIn, 10, 40, 0, 255);
  data[1][putIndex%POINTS] = mapFloat(tempRequested, 10, 40, 0, 255);
  putIndex++;
  sendTo = putIndex;
}

void sendGraph() {
  for (int i = sendFrom; i < sendTo; i++) {
    printAdd(0,  data[0][i%POINTS]);
    printAdd(1,  data[1][i%POINTS]);
  }
  sendFrom = sendTo;
}
