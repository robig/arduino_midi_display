/*  Dot matrix control Scrolling
 *  Tutorial: https://www.electronoobs.com/eng_arduino_tut56.php
    Schematic: https://www.electronoobs.com/eng_arduino_tut56_sch1.php
    LedControl library: https://www.electronoobs.com/eng_arduino_ledcontrol.php
*/


#include <LedControl.h>   //LedControl library: https://www.electronoobs.com/ledcontrol.php
#include <usbmidi.h> //https://github.com/BlokasLabs/usbmidi

#define CLK_PIN   4
#define DATA_PIN  2
#define CS_PIN    3

#define MIDI_NOTE_OFF   0b10000000
#define MIDI_NOTE_ON    0b10010000
#define MIDI_CONTROL    0b10110000
#define MIDI_PITCH_BEND 0b11100000
#define MIDI_SYNC       0b11111000
#define MIDI_CLOCK      0xF8 //248 == MIDI_SYNC = 24 beats per quater note
#define MIDI_TIME       0xF1 //241
#define MIDI_SONG_POINTER  0xF2 //242
#define MIDI_START      0xFA //250
#define MIDI_CONTINUE   0xFB //251
#define MIDI_STOP       0xFC //252

#define DEBUG_MIDI
//#define DEBUG_DISP

const int LED_BLUE = 8;
const int LED_RED = 9;

const int numDevices = 4;      // number of MAX7219s used in this case 2
const long scrollDelay = 150;   // adjust scrolling speed
unsigned long bufferLong [14] = {0};  
LedControl lc=LedControl(DATA_PIN,CLK_PIN,CS_PIN,numDevices);//DATA | CLK | CS/LOAD | number of matrices
const unsigned char scrollText[] PROGMEM ={" Test  "};

unsigned char dispText[6] = {" 1234 "};

long blinkOn = 0;
long blinkTimout = 200;

int notesOn = 0;
int currentKey = 0;

long clearAt=0;
 
void setup() {
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  
  Serial.begin(115200);
  for (int x=0; x<numDevices; x++)
  {
    lc.shutdown(x,false);       //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x,8);       // Set the brightness to default value
    lc.clearDisplay(x);         // and clear the display
  }

  while(!Serial);

  int p=1;
  p+=setBufferLong(p, 'H');
  p+=setBufferLong(p, 'e');
  p+=setBufferLong(p, 'l');
  p+=setBufferLong(p, 'l');
  p+=setBufferLong(p, 'o');
  clearAt=millis()+3000;

  dumpBuffer();
  clearBufferLong(1,6);
  dumpBuffer();
  /*
  setBufferLong(1, 'C');
  dumpBuffer();

  delay(500);
  clearBufferLong(0,6);
  dumpBuffer();*/
}

void dumpBuffer() {
  Serial.println("=============================================");
  for (int a=0;a<7;a++){  
    unsigned long  x = bufferLong [ a ];
    for (int8_t aBit = 31; aBit >= 0; aBit--) {
      Serial.print(bitRead(x, aBit) ? '1' : '0');
    }
    Serial.println();
  }
  Serial.println("=============================================");
}
 
void loop(){
  //Handle USB communication
  USBMIDI.poll();

  parseMidi();
  
  //scrollMessage(scrollText);   //scrollFont();
  //updateDisplay(dispText);

  blinkOff();

  if(clearAt>0 && clearAt<millis()) {
    clearAt=0;
    clearAll();
  }
}

// midi24 = C = nonASCII 35 = ASCII 65
// midi21 = A = nonASCII 32
// midi12 = C 

char mapMidiToAscii(unsigned int key) {
  switch(key) {
    case 12:
    case 24:
    case 36:
    case 48:
      return 'C';
    case 13:
    case 25:
    case 37:
      return 'C';//#
    case 14:
    case 26:
    case 38:
    case 50:
      return 'D';
    case 16:
    case 28:
    case 40:
    case 52:
      return 'E';
    case 17:
    case 29:
    case 41:
    case 53:
      return 'F';
    case 19:
    case 31:
    case 43:
    case 55:
      return 'G';
    case 21:
    case 33:
    case 45:
    case 57:
      return 'A';
    case 23:
    case 35:
    case 47:
    case 59:
      return 'B';
  }

  // Sharps:
  switch(key) {
    case 13:
    case 25:
    case 37:
    case 49:
      return 'C';//#
    case 14+1:
    case 26+1:
    case 38+1:
    case 50+1:
      return 'D';
    
    case 17+1:
    case 29+1:
    case 41+1:
    case 53+1:
      return 'F';
    case 19+1:
    case 31+1:
    case 43+1:
    case 55+1:
      return 'G';
    case 21+1:
    case 33+1:
    case 45+1:
    case 57+1:
      return 'A';
    
  }

  
  return '?';
}

boolean isSharp(unsigned int key) {
  switch(key) {
    case 13:
    case 25:
    case 37:
    case 49:
      return true; //'C';//#
    case 14+1:
    case 26+1:
    case 38+1:
    case 50+1:
      return true; //'D';
    
    case 17+1:
    case 29+1:
    case 41+1:
    case 53+1:
      return true; //'F';
    case 19+1:
    case 31+1:
    case 43+1:
    case 55+1:
      return true; //'G';
    case 21+1:
    case 33+1:
    case 45+1:
    case 57+1:
      return true; //'A';
    
  }
  return false;
}

void onReceive() {
  // blockOn
  lc.setLed(0,7,7, true);
  blinkOn = millis();
}

void blinkOff() {
  if(millis() - blinkOn > blinkTimout) {
    lc.setLed(0,7,7, false);
  }
  
}

void onNoteOn(unsigned int key) {
  onReceive();
  Serial.print("NoteOn: ");
  Serial.print(key);
  //char c = key + 32; // note C1 => A ??
  Serial.print(" => ");
  char c = mapMidiToAscii(key);
  Serial.println(c);
  //Serial.println((int)c);

  //if(key<currentKey || currentKey==0) {
  if(notesOn==0) {
    currentKey = key;
    
    boolean sharp=isSharp(key);
    clearBufferLong(1, 6);
    //clearBufferLong(6, 6);
    int s = setBufferLong(1, c);
    
    if(sharp) {
      clearBufferLong(s+1, 6);
      s+=setBufferLong(s+1, '#');
    }else{
      clearBufferLong(s+1, 6);
    }
    clearBufferLong(s, 6+1); //minor
  }else{
    // 2 notes or more

    boolean isMinor=false;
    if(key-currentKey == 3) isMinor=true;
    
    
    int pos=6+1;
    if(isSharp(currentKey)) {
      pos=+pos+6;
    }
    if(isMinor)
      setBufferLong(pos, 'm');
    else
      clearBufferLong(pos, 6+1);
  }
  notesOn++;
  Serial.print("notes that are On:");
  Serial.println(notesOn);
}

void onNoteOff(unsigned int key) {
  Serial.print("NoteOff: ");
  Serial.println(key);
  if(currentKey==key) currentKey=0;
  notesOn--;
  if(notesOn<0) notesOn=0;
  Serial.print("notes that are On:");
  Serial.println(notesOn);
}

int beatMax=4;
int beatCounter=1;
long clockCounter = 0;

void onStart() {
  notesOn=0;
  clockCounter=0;
  clearAll();
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_RED, LOW);
}



void onStop() {
  notesOn=0;
  clockCounter=0;

  displayText("STOP", 4);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_RED, HIGH);

  for(int i=0; i<numDevices; i++) {
    lc.setLed(i, 7, 0, true);
  }
}


void onClock() {
  clockCounter++;

  if(clockCounter % 24) {
    beatCounter++;
    if(beatCounter>beatMax) beatCounter=1;

    lc.setLed(beatCounter-1 ,7,0, true);
    if(beatCounter==1) {
      for(int i=1; i<numDevices; i++) {
        lc.setLed(i, 7, 0, true);
      }
    }
  }
}

void displayText(const unsigned char * messageString, int len) {
  //printBufferLong();
  int counter = 0;
  int myChar=0;
  int pos=1;
  do {
      // read back a char 
      myChar =  pgm_read_byte_near(messageString + counter); 
      if (myChar != 0){
        Serial.print("char: ");
        Serial.println(myChar);
        //loadBufferLong(myChar);

        pos += setBufferLong(pos, myChar);
      }
      counter++;
  } 
  while (myChar != 0 && counter<len);
}

//Convert MIDI note to frequency
double base_a4=440; //set A4=440Hz
double note_to_freq(double n) {
  if( n>=0 && n<=119 ) {
    return base_a4*pow(2,(n-57)/12);
  } else {
    return -1;
  }
}

void parseMidi() {
  while (USBMIDI.available()) {
    //Parse MIDI
    u8 initial_command=0, command=0, channel=0, key=0, pitchbend=0, pblo=0, pbhi=0, velocity=0;
  
    //Skip to beginning of next message (silently dropping stray data bytes)
    while(!(USBMIDI.peek() & 0b10000000)) USBMIDI.read();
  
    initial_command = USBMIDI.read();
    channel = (initial_command & 0b00001111)+1;
    command = initial_command & 0b11110000;


    //Debug
    #ifdef DEBUG_MIDI
    if(initial_command != MIDI_CLOCK) {
      Serial.print(initial_command);
      Serial.print(" Cmd:");
      Serial.print(command);
      Serial.print(" Chan:");
      Serial.print(channel);
      Serial.print(" K:");
      Serial.print(key);
      Serial.print("\tV:");
      Serial.print(velocity);
      Serial.print("\tPB:");
      Serial.print(pitchbend, BIN);
      Serial.print(" (");
      Serial.print(pblo, BIN);
      Serial.print(" ");
      Serial.print(pbhi, BIN);
      Serial.print('\n');
    }
    #endif

    switch(initial_command) {
      case MIDI_START:
        Serial.println("MIDI_START");
        onStart();
        break;
      case MIDI_STOP:
        Serial.println("MIDI_STOP");
        onStop();
        break;
      case MIDI_CONTINUE:
        Serial.println("MIDI_CONTINUE");
        onStart();
        break;
      case MIDI_CLOCK:
        onClock();
        break;
    }
  
    switch(command) {
      case MIDI_NOTE_ON:
      case MIDI_NOTE_OFF:
        if(USBMIDI.peek() & 0b10000000) continue; key      = USBMIDI.read();
        if(USBMIDI.peek() & 0b10000000) continue; velocity = USBMIDI.read();
        break;
      case MIDI_PITCH_BEND:
        if(USBMIDI.peek() & 0b10000000) return; pblo = USBMIDI.read();
        if(USBMIDI.peek() & 0b10000000) return; pbhi = USBMIDI.read();
        int pitchbend = (pblo << 7) | pbhi;
        //TODO: apply pitchbend to tone
        break;
    }
  
    //Play tones
    //unsigned int pitch = note_to_freq(key);
    if(command == MIDI_NOTE_ON && velocity > 0) onNoteOn(key);
    if(command == MIDI_NOTE_OFF || (command == MIDI_NOTE_ON && velocity == 0)) onNoteOff(key);
  
  }
}



void myClock(){
  Serial.println("MIDI: clock");
}
void myStart(){
  Serial.println("MIDI: start");
}
void myContinue(){
  Serial.println("MIDI: continue");
}
void myStop(){
  Serial.println("MIDI: stop");
}

 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
const unsigned char font5x7 [] PROGMEM = {      //Numeric Font Matrix (Arranged as 7x font data + 1x kerning data)
    B00000000,  //Space (Char 0x20)
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    3,//size of the space between letters
 
    B01000000,  //!
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B00000000,
    B01000000,
    2,
 
    B10100000,  //"
    B10100000,
    B10100000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    4,
 
    B00000000,  //#
    B00000000,
    B01010000,
    B11111000,
    B01010000,
    B11111000,
    B01010000,
    6,
 
    B00100000,  //$
    B01111000,
    B10100000,
    B01110000,
    B00101000,
    B11110000,
    B00100000,
    6,
 
    B11000000,  //%
    B11001000,
    B00010000,
    B00100000,
    B01000000,
    B10011000,
    B00011000,
    6,
 
    B01100000,  //&
    B10010000,
    B10100000,
    B01000000,
    B10101000,
    B10010000,
    B01101000,
    6,
 
    B11000000,  //'
    B01000000,
    B10000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    3,
 
    B00100000,  //(
    B01000000,
    B10000000,
    B10000000,
    B10000000,
    B01000000,
    B00100000,
    4,
 
    B10000000,  //)
    B01000000,
    B00100000,
    B00100000,
    B00100000,
    B01000000,
    B10000000,
    4,
 
    B00000000,  //*
    B00100000,
    B10101000,
    B01110000,
    B10101000,
    B00100000,
    B00000000,
    6,
 
    B00000000,  //+
    B00100000,
    B00100000,
    B11111000,
    B00100000,
    B00100000,
    B00000000,
    6,
 
    B00000000,  //,
    B00000000,
    B00000000,
    B00000000,
    B11000000,
    B01000000,
    B10000000,
    3,
 
    B00000000,  //-
    B00000000,
    B11111000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    6,
 
    B00000000,  //.
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11000000,
    B11000000,
    3,
 
    B00000000,  ///
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    B00000000,
    6,
 
    B01110000,  //0
    B10001000,
    B10011000,
    B10101000,
    B11001000,
    B10001000,
    B01110000,
    6,
 
    B01000000,  //1
    B11000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B01110000,  //2
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B11111000,
    6,
 
    B11111000,  //3
    B00010000,
    B00100000,
    B00010000,
    B00001000,
    B10001000,
    B01110000,
    6,
 
    B00010000,  //4
    B00110000,
    B01010000,
    B10010000,
    B11111000,
    B00010000,
    B00010000,
    6,
 
    B11111000,  //5
    B10000000,
    B11110000,
    B00001000,
    B00001000,
    B10001000,
    B01110000,
    6,
 
    B00110000,  //6
    B01000000,
    B10000000,
    B11110000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B11111000,  //7
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B01110000,  //8
    B10001000,
    B10001000,
    B01110000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B01110000,  //9
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B00010000,
    B01100000,
    6,
 
    B00000000,  //:
    B11000000,
    B11000000,
    B00000000,
    B11000000,
    B11000000,
    B00000000,
    3,
 
    B00000000,  //;
    B11000000,
    B11000000,
    B00000000,
    B11000000,
    B01000000,
    B10000000,
    3,
 
    B00010000,  //<
    B00100000,
    B01000000,
    B10000000,
    B01000000,
    B00100000,
    B00010000,
    5,
 
    B00000000,  //=
    B00000000,
    B11111000,
    B00000000,
    B11111000,
    B00000000,
    B00000000,
    6,
 
    B10000000,  //>
    B01000000,
    B00100000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    5,
 
    B01110000,  //?
    B10001000,
    B00001000,
    B00010000,
    B00100000,
    B00000000,
    B00100000,
    6,
 
    B01110000,  //@
    B10001000,
    B00001000,
    B01101000,
    B10101000,
    B10101000,
    B01110000,
    6,
 
    B01110000,  //A
    B10001000,
    B10001000,
    B10001000,
    B11111000,
    B10001000,
    B10001000,
    6,
 
    B11110000,  //B
    B10001000,
    B10001000,
    B11110000,
    B10001000,
    B10001000,
    B11110000,
    6,
 
    B01110000,  //C
    B10001000,
    B10000000,
    B10000000,
    B10000000,
    B10001000,
    B01110000,
    6,
 
    B11100000,  //D
    B10010000,
    B10001000,
    B10001000,
    B10001000,
    B10010000,
    B11100000,
    6,
 
    B11111000,  //E
    B10000000,
    B10000000,
    B11110000,
    B10000000,
    B10000000,
    B11111000,
    6,
 
    B11111000,  //F
    B10000000,
    B10000000,
    B11110000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B01110000,  //G
    B10001000,
    B10000000,
    B10111000,
    B10001000,
    B10001000,
    B01111000,
    6,
 
    B10001000,  //H
    B10001000,
    B10001000,
    B11111000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B11100000,  //I
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00111000,  //J
    B00010000,
    B00010000,
    B00010000,
    B00010000,
    B10010000,
    B01100000,
    6,
 
    B10001000,  //K
    B10010000,
    B10100000,
    B11000000,
    B10100000,
    B10010000,
    B10001000,
    6,
 
    B10000000,  //L
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B11111000,
    6,
 
    B10001000,  //M
    B11011000,
    B10101000,
    B10101000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B10001000,  //N
    B10001000,
    B11001000,
    B10101000,
    B10011000,
    B10001000,
    B10001000,
    6,
 
    B01110000,  //O
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B11110000,  //P
    B10001000,
    B10001000,
    B11110000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B01110000,  //Q
    B10001000,
    B10001000,
    B10001000,
    B10101000,
    B10010000,
    B01101000,
    6,
 
    B11110000,  //R
    B10001000,
    B10001000,
    B11110000,
    B10100000,
    B10010000,
    B10001000,
    6,
 
    B01111000,  //S
    B10000000,
    B10000000,
    B01110000,
    B00001000,
    B00001000,
    B11110000,
    6,
 
    B11111000,  //T
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B10001000,  //U
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B10001000,  //V
    B10001000,
    B10001000,
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    6,
 
    B10001000,  //W
    B10001000,
    B10001000,
    B10101000,
    B10101000,
    B10101000,
    B01010000,
    6,
 
    B10001000,  //X
    B10001000,
    B01010000,
    B00100000,
    B01010000,
    B10001000,
    B10001000,
    6,
 
    B10001000,  //Y
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    B00100000,
    B00100000,
    6,
 
    B11111000,  //Z
    B00001000,
    B00010000,
    B00100000,
    B01000000,
    B10000000,
    B11111000,
    6,
 
    B11100000,  //[
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B11100000,
    4,
 
    B00000000,  //(Backward Slash)
    B10000000,
    B01000000,
    B00100000,
    B00010000,
    B00001000,
    B00000000,
    6,
 
    B11100000,  //]
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B00100000,
    B11100000,
    4,
 
    B00100000,  //^
    B01010000,
    B10001000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    6,
 
    B00000000,  //_
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B11111000,
    6,
 
    B10000000,  //`
    B01000000,
    B00100000,
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    4,
 
    B00000000,  //a
    B00000000,
    B01110000,
    B00001000,
    B01111000,
    B10001000,
    B01111000,
    6,
 
    B10000000,  //b
    B10000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B11110000,
    6,
 
    B00000000,  //c
    B00000000,
    B01110000,
    B10001000,
    B10000000,
    B10001000,
    B01110000,
    6,
 
    B00001000,  //d
    B00001000,
    B01101000,
    B10011000,
    B10001000,
    B10001000,
    B01111000,
    6,
 
    B00000000,  //e
    B00000000,
    B01110000,
    B10001000,
    B11111000,
    B10000000,
    B01110000,
    6,
 
    B00110000,  //f
    B01001000,
    B01000000,
    B11100000,
    B01000000,
    B01000000,
    B01000000,
    6,
 
    B00000000,  //g
    B01111000,
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B01110000,
    6,
 
    B10000000,  //h
    B10000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B01000000,  //i
    B00000000,
    B11000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00010000,  //j
    B00000000,
    B00110000,
    B00010000,
    B00010000,
    B10010000,
    B01100000,
    5,
 
    B10000000,  //k
    B10000000,
    B10010000,
    B10100000,
    B11000000,
    B10100000,
    B10010000,
    5,
 
    B11000000,  //l
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B01000000,
    B11100000,
    4,
 
    B00000000,  //m
    B00000000,
    B11010000,
    B10101000,
    B10101000,
    B10001000,
    B10001000,
    6,
 
    B00000000,  //n
    B00000000,
    B10110000,
    B11001000,
    B10001000,
    B10001000,
    B10001000,
    6,
 
    B00000000,  //o
    B00000000,
    B01110000,
    B10001000,
    B10001000,
    B10001000,
    B01110000,
    6,
 
    B00000000,  //p
    B00000000,
    B11110000,
    B10001000,
    B11110000,
    B10000000,
    B10000000,
    6,
 
    B00000000,  //q
    B00000000,
    B01101000,
    B10011000,
    B01111000,
    B00001000,
    B00001000,
    6,
 
    B00000000,  //r
    B00000000,
    B10110000,
    B11001000,
    B10000000,
    B10000000,
    B10000000,
    6,
 
    B00000000,  //s
    B00000000,
    B01110000,
    B10000000,
    B01110000,
    B00001000,
    B11110000,
    6,
 
    B01000000,  //t
    B01000000,
    B11100000,
    B01000000,
    B01000000,
    B01001000,
    B00110000,
    6,
 
    B00000000,  //u
    B00000000,
    B10001000,
    B10001000,
    B10001000,
    B10011000,
    B01101000,
    6,
 
    B00000000,  //v
    B00000000,
    B10001000,
    B10001000,
    B10001000,
    B01010000,
    B00100000,
    6,
 
    B00000000,  //w
    B00000000,
    B10001000,
    B10101000,
    B10101000,
    B10101000,
    B01010000,
    6,
 
    B00000000,  //x
    B00000000,
    B10001000,
    B01010000,
    B00100000,
    B01010000,
    B10001000,
    6,
 
    B00000000,  //y
    B00000000,
    B10001000,
    B10001000,
    B01111000,
    B00001000,
    B01110000,
    6,
 
    B00000000,  //z
    B00000000,
    B11111000,
    B00010000,
    B00100000,
    B01000000,
    B11111000,
    6,
 
    B00100000,  //{
    B01000000,
    B01000000,
    B10000000,
    B01000000,
    B01000000,
    B00100000,
    4,
 
    B10000000,  //|
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    B10000000,
    2,
 
    B10000000,  //}
    B01000000,
    B01000000,
    B00100000,
    B01000000,
    B01000000,
    B10000000,
    4,
 
    B00000000,  //~
    B00000000,
    B00000000,
    B01101000,
    B10010000,
    B00000000,
    B00000000,
    6,
 
    B01100000,  // (Char 0x7F)
    B10010000,
    B10010000,
    B01100000,
    B00000000,
    B00000000,
    B00000000,
    5,
    
    B00000000,  // smiley
    B01100000,
    B01100110,
    B00000000,
    B10000001,
    B01100110,
    B00011000,
    5
};
 
void scrollFont() {
    for (int counter=0x20;counter<0x80;counter++){
        loadBufferLong(counter);
        delay(500);
    }
}
 
// Scroll Message
void scrollMessage(const unsigned char * messageString) {
    int counter = 0;
    int myChar=0;
    do {
        // read back a char 
        myChar =  pgm_read_byte_near(messageString + counter); 
        if (myChar != 0){
            loadBufferLong(myChar);
        }
        counter++;
    } 
    while (myChar != 0);
}
// Load character into scroll buffer
void loadBufferLong(int ascii){
    if (ascii >= 0x20 && ascii <=0x7f){
        for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
            unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
            unsigned long x = bufferLong [a*2];     // Load current scroll buffer
            x = x | c;                              // OR the new character onto end of current
            bufferLong [a*2] = x;                   // Store in buffer
        }
        byte size = pgm_read_byte_near(font5x7 +((ascii - 0x20) * 8) + 7);     // Index into character table for kerning data
        //for (byte x=0; x<size;x++){
            //rotateBufferLong();
            printBufferLong();
            //delay(scrollDelay);
        //}
    }
}


void clearAll() {
  for (int a=0;a<7;a++){ 
    bufferLong [a] = 0;
  }
  printBufferLong();
}



// byteRead macro definition. Francois Auger, Universite de Nantes, France, November 19, 2016
#define byteRead(x,n) ( * ( (unsigned char*)(&x) + n) )
// ------------------------------------------------------------------------------------------
// byteWrite macro definition. Francois Auger, Universite de Nantes, France, April 20, 2017
#define byteWrite(x,n,b) (*( ( (byte*)(&x)+n) ) ) =b
// ------------------------------------------------------------------------------------------


byte clearBufferLong(unsigned int pos, int len){
  if(len==0) return 0;

  #ifdef DEBUG_DISP
    Serial.print("clearBufferLong: pos=");
    Serial.print(pos);
    Serial.print(" len=");
    Serial.print(len);
  #endif

    byte mask = 1;
    for(int i=1;i<len;i++) {
      mask = (mask << 1) | 1;
    }
    mask = mask << (8 - len);
    #ifdef DEBUG_DISP
    Serial.print(" mask=");
    Serial.println(mask, BIN);
    
    for (int a=0;a<7;a++){
      unsigned long x = bufferLong [a];
      byte b = byteRead(x,pos);
      Serial.println(x, BIN);
    }
    Serial.println("TO");
    #endif

    int p = 31 - pos;
    //Serial.print("p="); Serial.println(p);
    for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
        unsigned long x = bufferLong [a];     // Load current scroll buffer
        byte b = byteRead(x,p);

        //Serial.print("byte from="); Serial.print(b, BIN);
        b = (b & ~mask);

        //Serial.print(" to "); Serial.println(b, BIN);
        
        //b = ~b; //negate
        //b = b & (~mask);
        //b = ~b;
        //byteWrite(x,p,b);
        
        for(int i=0;i<len;i++) {
          //Serial.println(p+i);
          bitWrite(x,p-i,0);
        }
        
        bufferLong [ a ] = x;                   // Store in buffer

        #ifdef DEBUG_DISP
        Serial.println(x, BIN);
        #endif
    }

    return len;
}

byte setBufferLong(unsigned int pos, int ascii){
    if (ascii >= 0x20 && ascii <=0x7f){
      #ifdef DEBUG_DISP
        Serial.print("setBufferLong: i=");
        Serial.print(pos);
        Serial.print(" ascii=");
        Serial.println((char)ascii);
      #endif
        for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
            unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
            #ifdef DEBUG_DISP
            Serial.println(c, BIN);
            #endif
            unsigned long y = bufferLong [a];     // Load current scroll buffer
            //x = x | c;                              // OR the new character onto end of current
            int off = pos*7;
            unsigned long x = c;
            int p = 31 - pos - 7;
            x = x << p;
            y = y | x;
            bufferLong [ a ] = y;                   // Store in buffer
        }
        
        byte count = pgm_read_byte_near(font5x7 +((ascii - 0x20) * 8) + 7);     // Index into character table for kerning data

        #ifdef DEBUG_DISP
        Serial.print("size="); Serial.println(count);
        #endif

        printBufferLong();
    
        return count;
    }
    return 0;
}

// Rotate the buffer
void rotateBufferLong(){
    for (int a=0;a<7;a++){                      // Loop 7 times for a 5x7 font
        unsigned long x = bufferLong [a*2];     // Get low buffer entry
        byte b = bitRead(x,31);                 // Copy high order bit that gets lost in rotation
        x = x<<1;                               // Rotate left one bit
        bufferLong [a*2] = x;                   // Store new low buffer
        x = bufferLong [a*2+1];                 // Get high buffer entry
        x = x<<1;                               // Rotate left one bit
        bitWrite(x,0,b);                        // Store saved bit
        bufferLong [a*2+1] = x;                 // Store new high buffer
    }
}

// Display Buffer on LED matrix
void printBufferLong(){
  //Serial.println("print: ");
  for (int a=0;a<7;a++){                    // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a*2+1];   // Get high buffer entry
    byte y = x;                             // Mask off first character
    //Serial.print(y);
    x = bufferLong [a];                   // Get low buffer entry

    byte reversed = ((x&0x01)<<7)|((x&0x02)<<5)|((x&0x04)<<3)|((x&0x08)<<1)|((x&0x10)>>1)|((x&0x20)>>3)|((x&0x40)>>5)|((x&0x80)>>7);
    
    lc.setRow(3,6-a,reversed);                       // Send row to relevent MAX7219 chip

    y = (x>>8);
    reversed = ((y&0x01)<<7)|((y&0x02)<<5)|((y&0x04)<<3)|((y&0x08)<<1)|((y&0x10)>>1)|((y&0x20)>>3)|((y&0x40)>>5)|((y&0x80)>>7);
    //Serial.print(" y=");
    //Serial.println(y, BIN);
    //y = bufferLong [7 + a];
                                // Mask off second character
    lc.setRow(2,6-a,reversed);                       // Send row to relevent MAX7219 chip
    y = (x>>16);                            // Mask off third character
    reversed = ((y&0x01)<<7)|((y&0x02)<<5)|((y&0x04)<<3)|((y&0x08)<<1)|((y&0x10)>>1)|((y&0x20)>>3)|((y&0x40)>>5)|((y&0x80)>>7);
    lc.setRow(1,6-a,reversed);                       // Send row to relevent MAX7219 chip
    y = (x>>24);                             // Mask off forth character
    reversed = ((y&0x01)<<7)|((y&0x02)<<5)|((y&0x04)<<3)|((y&0x08)<<1)|((y&0x10)>>1)|((y&0x20)>>3)|((y&0x40)>>5)|((y&0x80)>>7);
    lc.setRow(0,6-a,reversed);                       // Send row to relevent MAX7219 chip
  }
  //Serial.println("Printed");
}
