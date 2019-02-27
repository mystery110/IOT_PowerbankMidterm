#include <LBLE.h>
#include <LBLEPeriphral.h>
//Bluetooth
// 在全域空間定義一個只有一個資料特性的服務
LBLEService AService("19B10010-E8F2-537E-4F6C-D104768A1214");
LBLECharacteristicInt ARead("19B10011-E8F2-537E-4F6C-D104768A1214", LBLE_READ | LBLE_WRITE);

int FResult = 0, LResult, RResult, dir, BFResult, BLResult, BRResult, pdir = 0, k, iF1, iF2, iRTurn, iLTurn;
int sec;
//Bluetoothend
#define enA 15
#define enB 10
#define RwheelPin1 11
#define RwheelPin2 12
#define LwheelPin1 13
#define LwheelPin2 14
#define RSonarT 6
#define RSonarE 7
#define FSonarT 2
#define FSonarE 3
#define LSonarT 4
#define LSonarE 5
#define RedPin 16
#define GreenPin 17
#define BluePin 8
#include <LWiFi.h>
#define SSID "CSIE-WLAN"
#define PASSWD "wificsie"
#define TCP_IP "192.168.208.185"
#define TCP_PORT 5000
static char buf[32];
static int messageLen;
WiFiClient wificlient;
void Left_wall();
void getLocation();
int Turn_Made = 0, BTurn = 0, StartX, StartY, FinalX, FinalY, X, Y, PrevX, PrevY;
time_t Current_Wifi_Time = 0, Final_Wifi_Time = 0;

void setup() {
  Current_Wifi_Time = millis();
  Final_Wifi_Time = millis();

  for (int i = 10; i < 18; i++) {
    pinMode(i, OUTPUT);
  }
  digitalWrite(enA, HIGH);
  //pinMode(b_Pin, OUTPUT);
  digitalWrite(enB, HIGH);
  analogWrite(RedPin, 0);
  analogWrite(GreenPin, 0);
  analogWrite(BluePin, 255);
  //Bulutooth
  Serial.begin(9600);
  LBLE.begin(); // 開啟BLE功能
  while (!LBLE.ready()) {
    delay(100);
  }
  Serial.println("BLE ready");
  Serial.print("Device Address = ["); // 印出裝置位址
  Serial.print(LBLE.getDeviceAddress());
  Serial.println("]");
  AService.addAttribute(ARead); //將 ARead 這個資料特性添加到 A 這個服務當中
  //// 將 A 這個服務添加到 GATT 伺服器之中
  LBLEPeripheral.addService(AService);
  LBLEPeripheral.begin();
  LBLEAdvertisementData advertisement;
  advertisement.configAsConnectableDevice("Yz Car");
  LBLEPeripheral.advertise(advertisement);
  while (!Serial);
  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED) {
    status = WiFi.begin(SSID, PASSWD);
    Serial.print("status");
    Serial.print(status);
    analogWrite(RedPin, 0);
    analogWrite(GreenPin, 0);
    analogWrite(BluePin, 255);
    Serial.print("OK");
    const int ip = WiFi.localIP();
    // Conenct to AP successfully
    wificlient.connect(TCP_IP, TCP_PORT);
    //等待開啟 Serial。
    wificlient.write("join monkey B");
  }
  //Bluetooth end

  do {
    LResult = get_distance(LSonarT, LSonarE);
  } while (LResult < 3);
  do {
    RResult = get_distance(RSonarT, RSonarE);
  } while (RResult < 3);
  do {
    FResult = get_distance(FSonarT, FSonarE);
  } while (FResult < 3);

  getLocation();
}

void loop() {
  getLocation();
  StartX = buf[9] - '0';
  StartY = buf[11] - '0';
  if (StartX != 0 || StartY != 0) {
    getLocation();
    StartX = buf[9] - '0';
    StartY = buf[11] - '0';
  }
  time_t Time = millis();
  StartX = buf[9] - '0';
  StartY = buf[11] - '0';
  while (millis()-Time < 50000) {
    Left_wall();
    Time = millis();
  }
  FinalX = 7;
  FinalY = 7;
  Stop();
  getLocation();
  StartX = int(buf[9]) - '0';
  StartY = int(buf[11]) - '0';
  while (StartX != FinalX && StartY != FinalY) {
    Time = millis();
    while (millis() - Time < 200) {
      Left_wall();
      getLocation();
      StartX = int(buf[9]) - '0';
      StartY = int(buf[11]) - '0';
    }
    Stop();
    getLocation();
    StartX = buf[9] - '0';
    StartY = buf[11] - '0';
  }
  Stop();
  delay(10000);
}

void Left_wall() {
  LResult = get_distance(LSonarT, LSonarE);
  RResult = get_distance(RSonarT, RSonarE);
  FResult = get_distance(FSonarT, FSonarE);
  if (LResult <= 27) {
    if (FResult <= 20) {
      if (RResult <= 25) {
        dir = 2;
        Serial.print("B");
      }
      else {
        dir = 4;
        Serial.print("R");
      }
    }
    else {
      Serial.print("F");
      dir = 1;
    }
  }
  else {
    Serial.print("L");
    dir = 3;
  }
  BFResult = FResult;
  BRResult = RResult;
  BLResult = LResult;
  //...........................................................................Moving Car.....................................................................................
  switch (dir) {
    case 1:
      MStraight(1);
      Serial.print("FFFFFF");
      break;

    case 2:
      while (get_distance(FSonarT, FSonarE) < 30) {
        TBack();
      }
      for (int i = 0; i < 2; i++) {
        MStraight(0);
      }
      break;

    case 3:
      iF1 = 0;
      iF2 = 0;
      iRTurn = 0;
      iLTurn++;
      if (get_distance(RSonarT, RSonarE) < 3) {
        k = 6;
      }
      else  if (get_distance(RSonarT, RSonarE) < 5) {
        k = 5;
      }
      else if (get_distance(RSonarT, RSonarE) < 9 || get_distance(RSonarT, RSonarE) > 28) {
        k = 4;
      }
      else {
        k = 3;
      }
      if (iLTurn < 5) {
        for (int i = 0; i < 35; i++) {
          if (get_distance(LSonarT, LSonarE) > 15) {
            TLeft(1);
          }
          else {
            i = 90;
          }
        }
        for (int i = 0; i < k; i++) {
          if (i < 4) {
            MStraight(0);
          }
          else {
            MStraight(1);
          }
        }
        Serial.print("F");
      }
      else {
        MBack();

      }
      break;

    case 4:
      iF1 = 0;
      iF2 = 0;
      iLTurn = 0;
      iRTurn++;
      if (get_distance(LSonarT, LSonarE) < 3) {
        k = 6;
      }
      else if (get_distance(LSonarT, LSonarE) < 5) {
        k = 5;
      }
      else if (get_distance(LSonarT, LSonarE) < 9 || get_distance(LSonarT, LSonarE) > 28) {
        k = 4;
      }
      else {
        k = 3;
      }
      if (iRTurn < 5) {
        for (int i = 0; i < 35; i++) {
          if (get_distance(RSonarT, RSonarE) > 15) {
            TRight(1);
          }
          else {
            i = 90;
          }
        }
        for (int i = 0; i < k; i++) {
          if (i < 4) {
            MStraight(0);
          }
          else {
            MStraight(1);
          }
        }
        Serial.print("F");
      }
      else {
        MBack();
      }
      break;

  }
  Serial.print(LResult);
  ARead.setValue(dir);
  Serial.println("cm");

}
void MStraight(int det) {
  if (det == 1) {
    if (get_distance(LSonarT, LSonarE) < 20 && get_distance(RSonarT, RSonarE) < 20) {
      if (0.9 * get_distance(LSonarT, LSonarE) > 10.8) {
        iF2 = 0;
        iLTurn = 0;
        iRTurn = 0;
        iF1++;
        if (iF1 > 4) {
          analogWrite(RwheelPin1, 0);
          analogWrite(RwheelPin2, 140);
          analogWrite(LwheelPin1, 0);
          analogWrite(LwheelPin2, 200);
          Serial.println("pppp");
          delay(300);
          iF1 = 0;
          analogWrite(RwheelPin1, 200);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 60);
          analogWrite(LwheelPin2, 0);
          Serial.println("F1");
          delay(80);
          analogWrite(RwheelPin1, 60);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 200);
          analogWrite(LwheelPin2, 0);
          delay(20);
        }
        else {
          analogWrite(RwheelPin1, 200);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 60);
          analogWrite(LwheelPin2, 0);
          Serial.println("F1");
          delay(70);

          analogWrite(RwheelPin1, 60);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 200);
          analogWrite(LwheelPin2, 0);
          delay(30);
        }

      }
      //}
      else if (0.9 * get_distance(LSonarT, LSonarE) < 10.8) {
        iF1 = 0;
        iLTurn = 0;
        iRTurn = 0;
        iF2++;
        if (iF2 > 4) {
          analogWrite(RwheelPin1, 0);
          analogWrite(RwheelPin2, 200);
          analogWrite(LwheelPin1, 0);
          analogWrite(LwheelPin2, 140);
          Serial.println("kkkk");
          delay(300);
          analogWrite(RwheelPin1, 60);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 200);
          analogWrite(LwheelPin2, 0);
          delay(80);
          analogWrite(RwheelPin1, 200);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 60);
          analogWrite(LwheelPin2, 0);
          iF2 = 0;
          delay(20);
        }
        else {
          analogWrite(RwheelPin1, 60);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 200);
          analogWrite(LwheelPin2, 0);
          Serial.println("F2");
          delay(70);
          analogWrite(RwheelPin1, 200);
          analogWrite(RwheelPin2, 0);
          analogWrite(LwheelPin1, 60);
          analogWrite(LwheelPin2, 0);
          delay(30);
        }
      }
    }
  }
  analogWrite(RwheelPin1, 200);
  analogWrite(RwheelPin2, 0);
  analogWrite(LwheelPin1, 200);
  analogWrite(LwheelPin2, 0);
  Serial.println("F3");
  delay(25);
}


void TRight(int det) {
  if (det == 1) {
    //if(get_distance(FSonarT,FSonarE)>=10&get_distance(LSonarT,LSonarE)>=10){
    analogWrite(RwheelPin1, 0);
    analogWrite(RwheelPin2, 80);
    analogWrite(LwheelPin1, 255);
    analogWrite(LwheelPin2, 0);
    //}
    /*
      else if(get_distance(FSonarT,FSonarE)<10|get_distance(LSonarT,LSonarE)<10){
      analogWrite(RwheelPin1,0);
      analogWrite(RwheelPin2,60);
      analogWrite(LwheelPin1,255);
      analogWrite(LwheelPin2,0);}

      else if(get_distance(FSonarT,FSonarE)<5|get_distance(LSonarT,LSonarE)<5){
      analogWrite(RwheelPin1,0);
      analogWrite(RwheelPin2,70);
      analogWrite(LwheelPin1,255);
      analogWrite(LwheelPin2,0);
      }
    */
    delay(10);
  }

}

void TLeft(int det ) {

  if (det == 1) {
    //if(get_distance(FSonarT,FSonarE)>=10&get_distance(RSonarT,RSonarE)>=10){
    analogWrite(RwheelPin1, 255);
    analogWrite(RwheelPin2, 0);
    analogWrite(LwheelPin1, 0);
    analogWrite(LwheelPin2, 80);
  }
  /*
    else if(get_distance(FSonarT,FSonarE)<10|get_distance(RSonarT,RSonarE)<10){
    analogWrite(RwheelPin1,255);
    analogWrite(RwheelPin2,0);
    analogWrite(LwheelPin1,0);
    analogWrite(LwheelPin2,60);
    }

    else if(get_distance(FSonarT,FSonarE)<5|get_distance(RSonarT,RSonarE)<5){
    analogWrite(RwheelPin1,255);
    analogWrite(RwheelPin2,0);
    analogWrite(LwheelPin1,0);
    analogWrite(LwheelPin2,70);
    }

    }
  */
  delay(10);

}

int MBack() {
  iF2 = 0;
  iF1 = 0;
  iRTurn = 0;
  iLTurn = 0;
  analogWrite(RwheelPin1, 0);
  analogWrite(RwheelPin2, 255);
  analogWrite(LwheelPin1, 0);
  analogWrite(LwheelPin2, 255);
  delayMicroseconds(200);
}
void TBack() {
  analogWrite(RwheelPin1, 255);
  analogWrite(RwheelPin2, 0);
  analogWrite(LwheelPin1, 0);
  analogWrite(LwheelPin2, 250);
  delay(80);

}
float get_distance(int trig, int echo) { //計算距離
  int duration;
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);//給予trig 10us TTL pulse,讓模組發射聲波
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH, 5000000);//紀錄echo電位從high到low的時間，就是超音波來回的時間，若5秒內沒收到超音波則回傳0
  return duration / 29 / 2;// 聲速340m/s ，換算後約每29微秒走一公分，超音波來回所以再除2
}

void getLocation() {
  int i = 0;
  while (wificlient.available()) {
    buf[i++] = wificlient.read();
    delayMicroseconds(10);
  }
  /*
    if (buf[9] - '0' == PrevX && buf[11] == PrevY) {
    //Repeat++;
    }
  */
  if (i != 0) {
    buf[i] = '\0';
    Serial.println(buf);
  }
  for (int j = 0; j <= i; j++)
  {
    Serial.print(buf[j]);
    Serial.print("      ");
    Serial.print(j);
    Serial.print("\n");
  }

  i = 0;
  wificlient.write("position");
  Current_Wifi_Time = millis();
  delay(500);
}

void Stop() {
  analogWrite(RwheelPin1, 0);
  analogWrite(RwheelPin2, 0);
  analogWrite(LwheelPin1, 0);
  analogWrite(LwheelPin2, 0);

}
