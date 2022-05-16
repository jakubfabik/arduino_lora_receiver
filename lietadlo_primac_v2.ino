#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
const int butD2 = 2; //D2 gnd
const int butD3 = 3;
const int led = 8;
bool onepacket = 0;   //prisiel aspon jeden pacet
bool redBut = 0;
bool ledStat = 0;
bool loraInit = 1;    //lora modul zapnuty
float data_arr[9];            // pole pre data
bool setupStage1 = 0;
bool setupStage2 = 0;
int vysledok = 0;   //setup hodnota pociatocnej vysky nad morom v m n.m.

void(* resetFunc) (void) = 0;

int bluebut(){
  return digitalRead(butD2);
}
int redbut(){
  return digitalRead(butD3);
}

int header(String textIn, int start){    //UI
  lcd.setCursor(start,0);
  lcd.print(textIn);
}

int leftBut(String textIn, int start){   //UI
  lcd.setCursor(start,3);
  lcd.print(textIn);
}

int rightBut(String textIn, int start){  //UI
  lcd.setCursor(start,3);
  lcd.print(textIn);
}

int clearMnmSave(){   //UI
  for(int i = 0; i < 10; i++){   //vymaz stary vysledok
    lcd.setCursor(i + 8, 2); 
    lcd.print(" ");
    lcd.setCursor(i+2,1);
    lcd.print(" ");
  }
}

void setupSTAGE1(){
  if(!setupStage1){
    setupStage1 = 1;
    lcd.clear();
    header("Measuring", 6);
    leftBut("pressure", 0);
    rightBut("altitude", 12);
  }
  delay(130);   //bezpecna doba potrebna pre uvolnenie tlacidla
  if(redbut()){
    int cislo = -1;
    int saveVys = 0;
    int poradie = 0;
    lcd.clear();
    header("Input start altitude", 0);
    rightBut("change", 14);
    leftBut("enter", 0);
    lcd.setCursor(0,1);
    while(true){
      if(poradie > 2){ poradie = 0; } // metre nad morom iba trojciferne
      if(redbut()){
        clearMnmSave();
        lcd.blink();
        cislo += 1;
        if(cislo > 9){ cislo = 0; }
        lcd.setCursor(poradie,1);
        lcd.print(cislo);
      }
      if(bluebut()){
        if(cislo != -1){  // posun na index je mozny iba ak bolo vybrane cislo
          if(poradie == 2){
            vysledok += cislo; 
            saveVys = 1;  // vypis vysledok
          }
          if(poradie == 1){
            if(cislo != 0){ vysledok += 10 * cislo; }
          }
          if(poradie == 0){
            vysledok = 100 * cislo;
          }
          poradie++;
          cislo = -1;
        }
      }
      delay(130);   //bezpecna doba potrebna pre uvolnenie tlacidla  
      if(saveVys){
        lcd.noBlink();
        for(int i = 0; i < 3; i++){ lcd.setCursor(i,1); lcd.print(" ");} //vymaz input
        lcd.setCursor(8,2);
        lcd.print(vysledok);
        lcd.print(" m n.m.");
        lcd.setCursor(7,1);
        lcd.print("SAVE?");
        saveVys = 0;
      }
    }
  }
  if(bluebut()){
    resetFunc();
  }
  setupSTAGE1();
}



int get_info(float data_arr[]){
  int packetSize = LoRa.parsePacket();  //pokus o nacitanie dat z modulu lora
  if (packetSize == 0) return 0;
  if (packetSize) {
    digitalWrite(7, HIGH);
    String data;
    String data_str_arr[8];
    while (LoRa.available()) {                  //pokial vysiela citaj data
      data += (char)LoRa.read();
      //Serial.print((char)LoRa.read());
    }
    for(int j = 0, i = 0; i < data.length(); i++){ //citaj znaky po endl a ukladaj do pola 
      if(data[i] != '\n'){
        //Serial.println(data);
        data_str_arr[j] += data[i];
      }
      else j++;
      if(j == 8) break;                         //necitaj viac ako 8 udajov, dalsie su nepodstatne
    }
    for (int x; x< 8; x++){
      data_arr[x] = data_str_arr[x].toFloat();  //potrebujem desatinne cisla
      }

  }
  return 1;
}


void setup() {       //inicializacia
  lcd.begin(20,4);
  lcd.setCursor(3,0);
  lcd.print("Lora telemetry");
  lcd.setCursor(8,1);
  lcd.print("v2.0");
  lcd.setCursor(0,3);
  lcd.print("setup");
  lcd.setCursor(15,3);
  lcd.print("start");
  pinMode(butD2, INPUT);
  pinMode(butD3, INPUT);
  pinMode(led, OUTPUT);
  //Serial.begin(9600);
  //while (!Serial);
  //Serial.println("LoRa Receiver");           //diagnosticky vypis
  lcd.setCursor(0,2);
  if (!LoRa.begin(433E6)) {                  //LORA kontrola modulu
    //Serial.println("Lora init faild!");
    lcd.print("Lora init failed");
    while (1);                               // KONIEC
  }
  if(get_info(data_arr)){
    onepacket = 1;
    lcd.clear();
  }
}

void loop() {
  if(redBut){
    if(get_info(data_arr)){
      digitalWrite(led,true);
      lcd.clear();
      for(int i=0;i<9;i++){
        //Serial.println(data_arr[i]);
        if(data_arr[0]<10) lcd.setCursor(1,0);
        else lcd.setCursor(0,0);
        lcd.print(data_arr[0], 2);
        if(data_arr[1]<10) lcd.setCursor(7,0);
        else lcd.setCursor(6,0);
        lcd.print(data_arr[1], 2);
        if(data_arr[2]<10) lcd.setCursor(13,0);
        else lcd.setCursor(12,0);
        lcd.print(data_arr[2], 2);
        if(data_arr[3]<10) lcd.setCursor(1,1);
        else lcd.setCursor(0,0);
        lcd.print(data_arr[3], 2);
        if(data_arr[4]<10) lcd.setCursor(7,1);
        else lcd.setCursor(4,0);
        lcd.print(data_arr[4], 2);
        if(data_arr[5]<10) lcd.setCursor(13,1);
        else lcd.setCursor(12,0);
        lcd.print(data_arr[5], 2);
        lcd.setCursor(0,3);
        lcd.print("Temp:");
        lcd.print(data_arr[6], 0);
        lcd.setCursor(10,3);
        lcd.print("Pre:");
        lcd.print(data_arr[7]);
      }
    }
    else if(!onepacket) {
      lcd.print("Waiting for signal..");
      onepacket = 1;
    }
  }
  //else Serial.println("No incoming data");
  digitalWrite(led,false);
  if(bluebut()) setupSTAGE1();    //digitalWrite(led, true);
  if(redbut()) redBut=1;
}
