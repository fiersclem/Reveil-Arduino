#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <IRremote.hpp>
#include <SPI.h>
#define RECV_PIN 11     //pin infrarouge  peut être adapté en int const
IRrecv irrecv(RECV_PIN);//initialisation réception
decode_results results; //initialisation décodage
IRsend irsend;          //initialisation envoie

#include "RTClib.h"
RTC_DS1307 rtc;
char daysOfTheWeek[7][9] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char daysOfTheWeekf[7][9] = {"Dimanche", "Lundi", "Mardi", "Mercredi", "Jeudi", "Vendredi", "Samedi"};

const int DS1307 = 0x68; // Address of DS1307 see data sheets
const char* days[] ={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};    //---------peut etre origine bug mercredijeudi anglais
const char* months[] ={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
byte second = 0;
byte minute = 0;
byte hour = 0;
byte weekday = 0;
byte monthday = 0;
byte month = 0;
byte year = 0;

#define colone2 51                                                  //--1--2--3
#define ligne1 49                                                   //--4--5--6
#define colone1 47                                                  //--7--8--9
#define ligne4 31//accueil pin 45                                   //--*--0--#
#define colone3 43                                                  
#define ligne3 41                                                   
#define ligne2 39                                                   

int chrono =0;     //resonner si pas de réponse
int rereveil=0;    //pour resonner meme si réveil éteint mais pas désactiver

int reveil = 0;    //0=accueil / 1=saisie heure réveil / 2=saisie minute réveil / 6=? / 10=dernier chiffre saisie minute réveil / 80=?

int eheure=100;    //heure de réveil -1h
int heure = 100;   //heure de réveil saisie
int chiffre1 = 0;  //premier chiffre saisie heure réveil
int chiffre2 = 0;  //second  chiffre saisie heure réveil

int eminut=100;    //minute de réveil -30 min
int minut = 100;   //minute de réveil saisie
int chifre1 = 0;   //premier chiffre saisie minute réveil
int chifre2 = 0;   //second  chiffre saisie minute réveil

int nbrtouche = 0; //compteur de touche à la saisie du réveil

int accueil = 0;   //permet de réaficher le MENU principal
int alarm = 0;     // 1 2 3 4 selon parametre ou langue ou reveil etc 1 = set alarm     2 = parametre

int sonerie = 0;   //allumage reveil 0=éteint / 1=allumé
int buzzer = 4;    //pin buzzer

int push = 0;      //eviter les spam
int langue = 0;    //0=anglais et 1=français

int delayy = 0;    //delai entre la coupure et le ré allumage du buzzer
int repet = 0;     //repetition du reveil   1=réveil activer 0 = désactiver
int notif = 0;     //évite le spam affichage R

int veil = 1;      //1=chrono en cours et 0= en veille
int infra = 0;     //non fonctionnel 
int limite = 0;    //permet de gerer l'alerte heure > 24 apres le set alarm

int r=23;          //pin LED RGB
int g=25;          //pin LED RGB
int b=27;          //pin LED RGB
int vr=255;        //valeur de couleur rouge
int vg=255;        //valeur de couleur vert
int vb=255;        //valeur de couleur bleu

int appui =0;      //maintenir touche ou pas
int tap =0;        //Pour touche"1" 0=accueil / 1=appui simple fonction basique / 2=appui long donc allumage LED
int switchl=0;     //switch entre étain et allumé la LED RGB

void setup() {
  SPI.begin();
  Serial.begin(9600);
  irrecv.enableIRIn();
  //irsend.begin();
  //irrecv.blink13(true);
  lcd.init();
  lcd.backlight();
  if (! rtc.begin()){
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()){
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  pinMode(r,OUTPUT);
  pinMode(g,OUTPUT);
  pinMode(b,OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(ligne1, OUTPUT);
  pinMode(ligne2, OUTPUT);
  pinMode(ligne3, OUTPUT);
  pinMode(ligne4, OUTPUT);
  pinMode(colone1, INPUT_PULLUP);
  pinMode(colone2, INPUT_PULLUP);
  pinMode(colone3, INPUT_PULLUP);
}

void loop() {
  if (irrecv.decode(&results)){
    if (results.decode_type == NEC){
      Serial.print("NEC: ");
    }
    else if (results.decode_type == SONY){
      Serial.print("SONY: ");
    }
    else if (results.decode_type == RC5){
      Serial.print("RC5: ");
    }
    else if (results.decode_type == RC6){
      Serial.print("RC6: ");
    }
    else if (results.decode_type == UNKNOWN){
      Serial.print("UNKNOWN: ");
    }
    Serial.println(results.value, HEX);
    lcd.print(results.value);
    irrecv.resume();
  }
  constrain(heure, 0, 24);
  constrain(chiffre1, 0, 2);
  constrain(chiffre2, 0, 9);
  constrain(chifre1, 0, 5);
  constrain(chifre2, 0, 9);
  constrain(nbrtouche, 0, 2);
  constrain(second, 0, 59);
  DateTime now = rtc.now();

  if (reveil == 0 && accueil == 0){
    lcd.clear();
    if (langue == 0){
      lcd.setCursor(0, 3);
      lcd.print(" 1:alarm   2:setting");
    }
    if (langue == 1){
      lcd.setCursor(0, 3);
      lcd.print("1:reveil 2:parametre");
    }
    lcd.setCursor(17, 0);
    lcd.print("3:");
    if(repet == 1){
    lcd.setCursor(19,0);
    lcd.print("R");
    }
    lcd.noCursor();
    lcd.noBlink();
    accueil = 1;
  }
  if (reveil == 0){
    lcd.setCursor(5, 0);
    if (langue == 0){
      lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
    }
    if (langue == 1){
      lcd.print(daysOfTheWeekf[now.dayOfTheWeek()]);
    }
    lcd.setCursor(5, 1);
    lcd.print(now.day(), DEC);
    lcd.print('/');
    lcd.print(now.month(), DEC);
    lcd.print('/');
    lcd.print(now.year(), DEC);
    lcd.setCursor(5, 2);
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print(now.second(), DEC);
    lcd.print("       ");
    // calculate a date which is 7 days and 30 seconds into the future
    delay(300);//-------------------------------------------------------------voir enlever delay
  } 
  int ec1 = HIGH;
  int ec2 = HIGH;
  int ec3 = HIGH;
  digitalWrite(ligne1, LOW);
  digitalWrite(ligne2, HIGH);
  digitalWrite(ligne3, HIGH);
  digitalWrite(ligne4, HIGH);
  ec1 = digitalRead(colone1);
  ec2 = digitalRead(colone2);
  ec3 = digitalRead(colone3);
  
    while (!digitalRead(colone1)){     //prototype lumiere selon appui       //----------111111111111111111111111111111111111111111111111111111111
      appui +=1;
      if (digitalRead(colone1)){
        if(appui>=250){
          tap=2;
          appui=0;
        }
      else if(appui<250 && appui!=0){  
        tap=1;
        appui=0;
      }
     }
    }
  if(tap==2 && switchl==0){ 
    digitalWrite(r,vr);
    digitalWrite(g,vg);
    digitalWrite(b,vb);
    switchl=1;
    tap=0;
    appui=0;
  }
    if(tap==2 && switchl==1){ 
    digitalWrite(r,LOW);
    digitalWrite(g,LOW);
    digitalWrite(b,LOW);
    switchl=0;
    tap=0;
    appui=0;
  }
  if(tap==1 && reveil!=10){        //--------------------------------------------------ec1
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;
      irsend.sendNEC(0xFF906F, 32);
    }
    if (reveil == 0 && push == 0){
      lcd.setCursor(0, 3);
      if (langue == 0){
        lcd.print(">1:alarm   2:setting");
      }
      if (langue == 1){
        lcd.print(">1:reveil  2:paramet");
      }
      alarm = 1;
    }
    if (reveil == 3){
      if (langue == 0){
        lcd.home();
        lcd.print(" >1:set time        ");
        lcd.setCursor(0, 1);
        lcd.print("  2:language");
      }
      if (langue == 1){
        lcd.home();
        lcd.print(" >1:mise a l'heure");
        lcd.setCursor(0, 1);
        lcd.print("  2:langue");
      }
      alarm = 3;
    }
    if (reveil == 4){
      lcd.setCursor(0, 1);
      if (langue == 0){  
        lcd.print(" >1:English");
        lcd.setCursor(0, 2);
        lcd.print("  2:French");
      }
      if (langue == 1){
        lcd.print(" >1:Anglais         ");
        lcd.setCursor(0, 2);
        lcd.print("  2:Francais        ");
      }
      alarm = 10;
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 1;
      lcd.print("1");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 1;
      lcd.print("1");
    }

    if (nbrtouche == 0 && reveil == 2){ 
      chifre1 = 1;
      lcd.print("1");
    }
    if (nbrtouche == 1 && reveil == 2){ 
      chifre2 = 1;
      lcd.print("1");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
    appui=0;
    tap=0;
  }
  if (ec2 == LOW && reveil!=10){         //----------------------------------2222222222222222222222222222222222222222222222222222
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){ 
      sonerie = 0; 
      irsend.sendNEC(0xFF906F, 32);
    }
    if (reveil == 0 && push == 0){
      
      lcd.setCursor(0, 3);
      if (langue == 0){
        lcd.print(" 1:alarm  >2:setting");
      }
      if (langue == 1){
        lcd.print("reveil  >2:parametre");
      }
      alarm = 2;
    }
    if (reveil == 3){
      if (langue == 0){
        lcd.home();
        lcd.print("  1:set time        ");
        lcd.setCursor(0, 1);
        lcd.print(" >2:language");
      }
      if (langue == 1){
        lcd.home();
        lcd.print("  1:mise a l'heure  ");
        lcd.setCursor(0, 1);
        lcd.print(" >2:langue");
      }
      alarm = 4;
    }  
    if (reveil == 4){
      lcd.setCursor(0, 1);   
      if (langue == 0){
        lcd.print("  1:English         ");
        lcd.setCursor(0, 2);
        lcd.print(" >2:French          ");
      }
      if (langue == 1){
        lcd.print("  1:Anglais         ");
        lcd.setCursor(0, 2);
        lcd.print(" >2:Francais        ");
      }
      alarm = 11;
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 2;
      lcd.print("2");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 2;
      lcd.print("2");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 2;
      lcd.print("2");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 2;
      lcd.print("2");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }
  if (ec3 == LOW && reveil!=10){                 //------------------------------------33333333333333333333333333333333333333333333333333333333333
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (reveil == 0){
      if (repet == 0 && notif == 0){
        lcd.setCursor(19, 0);
        lcd.print("R");
        repet = 1;
        notif = 1;
      }
      if (repet == 1 && notif == 0){
        lcd.setCursor(19, 0);
        lcd.print(" ");
        repet = 0;
        notif = 1;
        rereveil = 0;
      }
    }
    if (sonerie == 1){
      sonerie = 0;
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 3;
      lcd.print("3");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 3;
      lcd.print("3");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 3;
      lcd.print("3");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 3;
      lcd.print("3");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    notif = 0;
    veil = 1;
    delay(400);

    if (repet == 1 && heure == 100 && minut == 100) { //------------------------------------------notif reveil 
    reveil = 80; //--desactiver l'affichage de l'heure
    lcd.clear();
    lcd.setCursor(3, 1);
    if (langue == 0){
      lcd.print("please enter a");  //-----------------modifier version anglais
      lcd.setCursor(0, 2);
      lcd.print("wake-up time");
    }
    if (langue == 1){
      lcd.print("veuillez saisir");
      lcd.setCursor(0, 2);
      lcd.print("une heure de reveil");  
    }
    delay(2500);
    lcd.clear();
    accueil=0;
    reveil = 0;
    repet = 0;
    notif = 0;
    veil=1;
    lcd.setCursor(19, 0);
    lcd.print(" ");
    }
  }
  digitalWrite(ligne1, HIGH);
  digitalWrite(ligne2, LOW);
  digitalWrite(ligne3, HIGH);
  digitalWrite(ligne4, HIGH);
  ec1 = digitalRead(colone1);
  ec2 = digitalRead(colone2);
  ec3 = digitalRead(colone3);

  if (ec1 == LOW && reveil!=10){          //-------------------------------------44444444444444444444444444444444444444444444444444444444
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;   
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 4;
      lcd.print("4");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 4;
      lcd.print("4");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 4;
      lcd.print("4");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 4;
      lcd.print("4");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400); 
  }

  if (ec2 == LOW && reveil!=10){                //-----------------------------------5555555555555555555555555555555555555555555
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;  
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 5;
      lcd.print("5");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 5;
      lcd.print("5");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 5;
      lcd.print("5");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 5;
      lcd.print("5");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }

  if (ec3 == LOW && reveil!=10){               //-------------------------------666666666666666666666666666666666666666666666666666
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;    
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 6;
      lcd.print("6");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 6;
      lcd.print("6");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 6;
      lcd.print("6");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 6;
      lcd.print("6");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }
  digitalWrite(ligne1, HIGH);
  digitalWrite(ligne2, HIGH);
  digitalWrite(ligne3, LOW);
  digitalWrite(ligne4, HIGH);
  ec1 = digitalRead(colone1);
  ec2 = digitalRead(colone2);
  ec3 = digitalRead(colone3);

  if (ec1 == LOW && reveil!=10){                 //------------------------------77777777777777777777777777777777777777777777777777
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;     
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 7;
      lcd.print("7");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 7;
      lcd.print("7");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 7;
      lcd.print("7");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 7;
      lcd.print("7");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }

  if (ec2 == LOW && reveil!=10){                    //------------------------------------88888888888888888888888888888888888888888888888
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0; 
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 8;
      lcd.print("8");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 8;
      lcd.print("8");
    }

    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 8;
      lcd.print("8");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 8;
      lcd.print("8");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }
  
  if (ec3 == LOW && reveil!=10){                       //---------------------------------999999999999999999999999999999999999999999999999999
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0; 
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 9;
      lcd.print("9");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 9;
      lcd.print("9");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 9;
      lcd.print("9");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 9;
      lcd.print("9");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
    veil = 1;
    delay(400);
  }
  digitalWrite(ligne1, HIGH);
  digitalWrite(ligne2, HIGH);
  digitalWrite(ligne3, HIGH);
  digitalWrite(ligne4, LOW);
  ec1 = digitalRead(colone1);
  ec2 = digitalRead(colone2);
  ec3 = digitalRead(colone3);

  if (ec1 == LOW){   //------------------------------------------------********************************************************
    if(reveil==1 && nbrtouche==0){ //-------------accueil retour arriere saisie reveil
      reveil = 0;
      accueil = 0;
      push = 0;
      alarm = 0;
      delay(100);
    }
    if(reveil==1 && nbrtouche==1){  //-------------accueil retour arriere saisie reveil
        lcd.setCursor(8,0);
        lcd.print(" ");
        lcd.setCursor(8,0);
        nbrtouche=0;
        delay(300);
    }
     if(reveil==1 && nbrtouche==2){ //-------------accueil retour arriere saisie reveil
        lcd.setCursor(9,0);
        lcd.print(" ");
        lcd.setCursor(9,0);
        nbrtouche=1;
        delay(150);
    }
    if(reveil==2 && nbrtouche==0){ //-------------accueil retour arriere saisie reveil
       lcd.setCursor(9,0);
       lcd.print(" ");
       lcd.setCursor(9,0);
       nbrtouche=1;
       reveil = 1;
       delay(150);
    }
     if(reveil==2 && nbrtouche==1){ //-------------accueil retour arriere saisie reveil
        lcd.setCursor(8,1);
        lcd.print(" ");
        lcd.setCursor(9,0);
        lcd.blink();
        lcd.cursor();
        nbrtouche=2;
        reveil=1;
        delay(150);
    }
     if(reveil==10 && nbrtouche==2){   //-------------accueil retour arriere saisie reveil
        lcd.setCursor(9,1);
        lcd.print(" ");
        lcd.setCursor(9,1);
        nbrtouche=1;
        reveil=2;
        lcd.blink();
        lcd.cursor();
        delay(150);
    }
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      //anciennement repet=0
      sonerie = 0;
      reveil = 0;
      accueil = 0;
      nbrtouche = 0;
      push = 0;
      alarm = 0;
      irsend.sendNEC(0xFF906F, 32);
    }
    if (reveil == 0 || reveil ==3 || reveil ==4 || reveil ==6 || reveil ==80 ){
      reveil = 0;
      accueil = 0;
      nbrtouche = 0;
      push = 0;
      alarm = 0;
    }
    veil = 1;
  }

  if (ec2 == LOW && reveil!=10){  //----------------------------------------0000000000000000000000000000000000000
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0; 
      irsend.sendNEC(0xFF906F, 32);
    }
    if (nbrtouche == 0 && reveil == 1){
      chiffre1 = 0;
      lcd.print("0");
    }
    if (nbrtouche == 1 && reveil == 1){
      chiffre2 = 0;
      lcd.print("0");
    }
    if (nbrtouche == 0 && reveil == 2){
      chifre1 = 0;
      lcd.print("0");
    }
    if (nbrtouche == 1 && reveil == 2){
      chifre2 = 0;
      lcd.print("0");
      lcd.noCursor();
      lcd.noBlink(); reveil=10;
    }
    nbrtouche += 1;
     veil = 1;
    delay(400);
  }

  if (ec3 == LOW){ //-----------------------------------------------##############################################
    if (veil == 0){
      veil = 1;
      lcd.backlight();
    }
    if (sonerie == 1){
      sonerie = 0;
      
      irsend.sendNEC(0xFF906F, 32);
    } 
    if (alarm == 1 && reveil == 0){
       lcd.clear();
      if (langue == 0){
        lcd.print("hour:");
        lcd.setCursor(0, 1);
        lcd.print("minute:");
        lcd.setCursor(0, 3);
        lcd.print("*cancel");
        lcd.setCursor(17, 3);
        lcd.print("#OK");
      }
      if (langue == 1){
        lcd.print("heure:");
        lcd.setCursor(0, 1);
        lcd.print("minute:");
        lcd.setCursor(0, 3);
        lcd.print("*retour");
        lcd.setCursor(12, 3);
        lcd.print("#valider");
      }
      lcd.setCursor(8, 0);
      lcd.cursor();
      lcd.blink();
      push = 1;
      reveil = 1;
      alarm = 0;
      nbrtouche = 0;
      chiffre1 = 0;
      chiffre2 = 0;
      chifre1 = 0;
      chifre2 = 0;
    }

    if (alarm == 2 && reveil == 0){
      lcd.clear(); 
      if (langue == 0){               //--------------------------a revoir ligne inutil 1X if(heure !=100 && minut != 100) et langue apres c tout pas opti
         if(heure !=100 && minut != 100){
        lcd.setCursor(15,0);
        lcd.print(heure);
        lcd.print(":");
        lcd.print(minut);
      }
        lcd.setCursor(0,0);
        lcd.print("  1:set time");
        lcd.setCursor(0, 1);
        lcd.print("  2:language");
        lcd.setCursor(0, 3);
        lcd.print("*cancel          #OK");
      }
      if (langue == 1){
         if(heure !=100 && minut != 100){
        lcd.setCursor(15,2);
        lcd.print(heure);
        lcd.print(":");
        lcd.print(minut);
      }
        lcd.setCursor(0,0);
        lcd.print("  1:mise a l'heure");
        lcd.setCursor(0, 1);
        lcd.print("  2:langue");
        lcd.setCursor(0, 3);
        lcd.print("*retour     #valider");
      }
      push = 1;
      reveil = 3;
      nbrtouche = 0;
    }

    if (nbrtouche == 2 && reveil == 10){
      nbrtouche = 0;
      heure = (chiffre1 * 10) + (chiffre2);
      minut = (chifre1 * 10) + (chifre2);
      eminut = minut-30;
      if(eminut <0){
        eminut=60+eminut;
        eheure = heure-1;
      }  
      lcd.clear();
      if (langue == 0){
        lcd.print("alarm clock set at:");
      }
      if (langue == 1){
        lcd.print("reveil programme a:");
      }
      lcd.setCursor(7, 1);
      lcd.print(heure);
      lcd.print(":");
      lcd.print(minut);
      lcd.noCursor();
      lcd.noBlink();
      delay(2500);
      reveil = 0;
      accueil = 0;
      push = 0;
      alarm = 0;
      nbrtouche = 0;
      limite = 1;
    }
    
    if (alarm == 3){
      lcd.clear();
      if (langue == 0){
        lcd.print("please use your");
        lcd.setCursor(5, 1);
        lcd.print("computer");
        lcd.setCursor(9, 1);
      }

      if (langue == 1){
        lcd.print("utilisez votre pc");
        lcd.setCursor(9, 1);
      }
      reveil = 6;
      nbrtouche = 0;
      alarm = 537;
      setTime();
    }

    if (alarm == 4){
      lcd.clear();
      if (langue == 0){
        lcd.print("      language:     ");
        lcd.setCursor(0, 1);
        lcd.print("  1:English         ");
        lcd.setCursor(0, 2);
        lcd.print("  2:French          ");
      }

      if (langue == 1){
        lcd.print("       langue:      ");
        lcd.setCursor(0, 1);
        lcd.print("  1:Anglais         ");
        lcd.setCursor(0, 2);
        lcd.print("  2:Francais        ");
      }
      reveil = 4;
      alarm = 54; //-------peu importe juste eviter sacade
      nbrtouche = 0;
    }

    if (alarm == 10){
      lcd.clear();
      langue = 0;
      reveil = 0;
      push = 0;
      accueil = 0;
      alarm = 0;
      nbrtouche = 0;
    }

    if (alarm == 11){
      lcd.clear();
      langue = 1;
      reveil = 0;
      push = 0;
      accueil = 0;
      alarm = 0;
      nbrtouche = 0;
    }
    veil = 1;
  }

  if ((heure > 24 || minut > 59) && limite==1){      //----input error
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.noCursor();
    lcd.noBlink();
    if (langue == 0){
      lcd.print("  alarm input error ");
      delay(1500);
      lcd.setCursor(0, 2);
      lcd.print("  please try again  ");
    }
    if (langue == 1){
      lcd.print("erreur saisie reveil");
      delay(1500);
      lcd.setCursor(0, 2);
      lcd.print("veuillez recommencer");
    }
    delay(2500);
    reveil = 0;
    push = 0;
    alarm = 0;
    accueil = 0;
    nbrtouche = 0;
    heure = 100;
    minut = 100;
    chiffre1 = 0;
    chiffre2 = 0;
    chifre1 = 0;
    chifre2 = 0;
    limite = 0;
  }

  if ((now.hour() == heure) && (now.minute() == minut) && reveil == 0 && repet == 1 && rereveil==0){
    sonerie = 1;
    reveil = 0;
    push = 0;
    alarm = 0;
    accueil = 0;
    nbrtouche = 0;
  }

  if (sonerie == 1 && infra == 1){           //------------------non fonctionnel
    infra = 0;
    irsend.sendNEC(0xFF601F, 32);//---ON
    delay(20);
    irsend.sendNEC(0xFFF00F, 32);//---MODE
    delay(20);
    irsend.sendNEC(0xFFA05F, 32);//---PLUS FORT
    delay(50);
    irsend.sendNEC(0xFFA05F, 32);
    delay(50);
    irsend.sendNEC(0xFFA05F, 32);
  }

  if (sonerie == 1){
    rereveil=1;
    digitalWrite(buzzer, HIGH);
    delay(delayy);
    digitalWrite(buzzer, LOW);
    delayy = random(1, 500);
    irsend.sendNEC(0xFF601F, 32);//---ON
    delay(20);
    irsend.sendNEC(0xFFF00F, 32);//---MODE
    veil = 0;
    lcd.backlight();
    //delay(500);
  }
  else {
    digitalWrite(buzzer, LOW);
    }

  if(sonerie == 0 && repet == 1 && rereveil==1){
    chrono+=1;
    if(chrono>=150){
      sonerie=1;
      chrono=0;
      rereveil=0;
    }
  }

  if (veil != 0 && reveil == 0){
    veil += 1;
  }
  if (veil >= 30){
    veil = 0;
    lcd.noBacklight();
    appui=0;
    tap=0;
    irsend.sendNEC(0xFF609F, 32);//---OFF
  }
  
  if(eminut>=30 && reveil==0 && repet ==1 && now.hour()==eheure && now.minute()<eminut){    //--------a verifier eminut >30 veut dire que que l'heure voulu est 1h de plus et minute entre 0 et 30 ex 07h00 -30 = 6h30
  vr=255;
  vg=255;
  vb=0;
  }
    else {
    vr=0;
    vg=255;
    vb=0;
  }
  
  if(eminut<30 && reveil==0 && repet ==1 && now.hour()==heure && now.minute()<eminut){   //---a verifier eminut <30 veut dire que l'heure de  reil voulu est entre minute = 30 et 59
    vr=255;
    vg=255;
    vb=0; 
  }
    else {
    vr=0;
    vg=255;
    vb=0;
  }

  if(reveil==0 && repet ==1 && now.hour()==heure && now.minute()<=minut && now.minute()>=eminut){    //marche pas si reveil = 15h00 et time = 14h59 pas meme heur donc pas rouge
    vr=255;
    vg=0;
    vb=0;
  }
  else {
    vr=0;
    vg=255;
    vb=0;
  }
  if(reveil==0 && repet ==0){
    vr=255;
    vg=255;
    vb=255;
  }
}

byte decToBcd(byte val){
  return ((val / 10 * 16) + (val % 10));
}
byte bcdToDec(byte val) {
  return ((val / 16 * 10) + (val % 16));
}
// This set of codes is allows input of data
void setTime() {
  Serial.print("Please enter the current year 00-99. - ");
  year = readByte();
  Serial.println(year);
  Serial.print("Please enter the current month, 1-12. - ");
  month = readByte();
  Serial.println(months[month - 1]);
  Serial.print("Please enter the current day of the month, 1-31. - ");
  monthday = readByte();
  Serial.println(monthday);
  Serial.println("Please enter the current day of the week, 1-7.");
  Serial.print("1 Sun | 2 Mon | 3 Tues | 4 Weds | 5 Thu | 6 Fri | 7 Sat - ");
  weekday = readByte();
  Serial.println(days[weekday - 1]);
  Serial.print("Please enter the current hour in 24hr format, 0-23. - ");
  hour = readByte();
  Serial.println(hour);
  Serial.print("Please enter the current minute, 0-59. - ");
  minute = readByte();
  Serial.println(minute);
  second = 0;
  Serial.println("The data has been entered.");
  lcd.clear();
  if (langue == 1){
    lcd.print("date modifie");
    lcd.setCursor(0, 1);
    lcd.print("presse la touche *");
  }

  if (langue == 0){
    lcd.print("The data has been");
    lcd.setCursor(5, 1);
    lcd.print("entered");
    lcd.setCursor(5, 2);
    lcd.print("please press *");
  }
  // The following codes transmits the data to the RTC
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(weekday));
  Wire.write(decToBcd(monthday));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.write(byte(0));
  Wire.endTransmission();
  // Ends transmission of data
}
byte readByte() {
  while (!Serial.available()) delay(10);
  byte reading = 0;
  byte incomingByte = Serial.read();
  while (incomingByte != '\n') {
    if (incomingByte >= '0' && incomingByte <= '9')
      reading = reading * 10 + (incomingByte - '0');
    else;
    incomingByte = Serial.read();
  }
  Serial.flush();
  return reading;
}
void readTime() {
  Wire.beginTransmission(DS1307);
  Wire.write(byte(0));
  Wire.endTransmission();
  Wire.requestFrom(DS1307, 7);
  second = bcdToDec(Wire.read());
  minute = bcdToDec(Wire.read());
  hour = bcdToDec(Wire.read());
  weekday = bcdToDec(Wire.read());
  monthday = bcdToDec(Wire.read());
  month = bcdToDec(Wire.read());
  year = bcdToDec(Wire.read());
}