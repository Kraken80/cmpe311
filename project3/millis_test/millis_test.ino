#include "millis.h"

#define DIFFS_NUM 150
int diff[DIFFS_NUM];

void setup()
{
  mymillis_init();
  int i = 0;

  int block = 0;
  float mean, std, tmp;

  Serial.begin(9600);
  Serial.println("Making measurements...");

  while(i < DIFFS_NUM) {
    if(millis() < block) continue;
    diff[i++] = millis() - mymillis();
    block = millis() + 100;
    if(diff[i-1] > 1 || diff[i-1] < -1) {
      Serial.println("T2 fail");
    }
  }

  mean = 0;
  for(i = 0; i < DIFFS_NUM; i++) {
    mean += diff[i];
  }
  mean /= DIFFS_NUM;

  tmp = 0;
  std = 0;
  for(i = 0; i < DIFFS_NUM; i++) {
    tmp = diff[i]-mean;
    tmp = tmp * tmp;
    std += tmp;
  }
  std = std / (DIFFS_NUM - 1);
  std = sqrt(std);

  Serial.println("Statistics:");
  Serial.print("~x:");
  Serial.println(mean);
  Serial.print("s:");
  Serial.println(std);
}

void loop()
{
  return;
}