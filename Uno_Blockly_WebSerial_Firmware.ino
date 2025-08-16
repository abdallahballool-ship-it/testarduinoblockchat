// ===== Arduino UNO Firmware for Blockly Electron/Web Serial Control =====
// Upload once using Arduino IDE (Board: Arduino Uno).
// Baud: 115200. Supported commands (each on its own line):
//   LED <pin> ON|OFF
//   WAIT <ms>
//   ANALOG_READ A0..A5
// Program template:
//   BEGIN
//   ...instructions...
//   END
//   REPEAT <count|-1>
//   RUN
struct Instr { char op; int16_t p1; int16_t p2; };
const int MAXI = 128; Instr prog[MAXI]; int plen=0; long repeatCount=1; bool runFlag=false;
void resetProg(){ plen=0; repeatCount=1; runFlag=false; }
void addLED(int pin, int st){ if(plen<MAXI){ prog[plen++]={'L', (int16_t)pin, (int16_t)st}; } }
void addWAIT(long ms){ if(plen<MAXI){ prog[plen++]={'W', (int16_t)(ms & 0x7FFF), (int16_t)((ms>>15)&0x7FFF)}; } }
void addAREAD(int ap){ if(plen<MAXI){ prog[plen++]={'A', (int16_t)ap, 0}; } }
long waitMsFromInstr(const Instr &ins){ long lo = (uint16_t)ins.p1; long hi = (uint16_t)ins.p2; return lo | (hi<<15); }
void setup(){ Serial.begin(115200); for(int p=2;p<=13;p++){ pinMode(p, OUTPUT); digitalWrite(p, LOW);} Serial.println(F("READY")); }
void execOnce(){ for(int i=0;i<plen;i++){ Instr &in = prog[i]; if(in.op=='L'){ int pin=in.p1; int st=in.p2; digitalWrite(pin, st?HIGH:LOW); } else if(in.op=='W'){ delay(waitMsFromInstr(in)); } else if(in.op=='A'){ int ap = in.p1; int val = analogRead(ap); Serial.print(F("ANALOG ")); Serial.print(ap); Serial.print(F(": ")); Serial.println(val); } if(!runFlag) return; } }
void loop(){ static bool inBlock=false; static String line=""; while(Serial.available()>0){ char c = Serial.read(); if(c=='\n' || c=='\r'){ if(line.length()==0) continue; String s=line; line=""; s.trim(); s.toUpperCase(); if(s=="BEGIN"){ resetProg(); inBlock=true; Serial.println(F("BEGIN OK")); } else if(s=="END"){ inBlock=false; Serial.print(F("END OK PLEN=")); Serial.println(plen); } else if(s.startsWith("REPEAT ")){ long n = s.substring(7).toInt(); repeatCount=n; Serial.print(F("REPEAT=")); Serial.println(repeatCount); } else if(s=="RUN"){ runFlag=true; Serial.println(F("RUNNING")); } else if(s=="STOP"){ runFlag=false; Serial.println(F("STOPPED")); } else if(inBlock){ if(s.startsWith("LED ")){ int sp1=s.indexOf(' ',4); if(sp1>0){ int pin = s.substring(4,sp1).toInt(); String st=s.substring(sp1+1); addLED(pin, st=="ON"); } } else if(s.startsWith("WAIT ")){ long ms = s.substring(5).toInt(); addWAIT(ms); } else if(s.startsWith("ANALOG_READ ")){ int ap = -1; if(s.length()>=12){ char c1=s.charAt(12); if(c1>='0'&&c1<='5') ap = c1-'0'; } if(ap>=0) addAREAD(ap); } } } else { line += c; } } if(runFlag){ if(repeatCount==-1){ execOnce(); } else if(repeatCount>0){ execOnce(); repeatCount--; if(repeatCount==0){ runFlag=false; Serial.println(F("DONE")); }} } }
