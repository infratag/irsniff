// IRSniff examines 21-bit F-2F packets transmitted by some laser tag toys.

// The pin to which the active-low IR sensor is attached
const int IR_PIN = 7;

// The pins to blink when a packet is received (or 0 to not use)
const int PACKET_PIN_1 = 52;
const int PACKET_PIN_2 = 50;

// If there is no edge transition within this many milliseconds, the pulse train must be done.
const unsigned long MAX_PULSELENGTH = 200;

// Maximum number of pulses that may be detected before the measurements are printed to serial
const int MAX_PULSES = 100;

// Maximum number of bits per message
const int MAX_MESSAGE_BITS = 100;

// True to print debugging information about invalid packets, false to skip them.
const bool PRINT_INVALID_PACKETS = true;

// Instead of a time interval, write this value to dts to indicate the end of a packet.
const int END_OF_PACKET = -1;

void setup() {
  Serial.begin(115200);
  pinMode(IR_PIN, INPUT);
  if (PACKET_PIN_1 > 0) { pinMode(PACKET_PIN_1, OUTPUT); }
  if (PACKET_PIN_2 > 0) { pinMode(PACKET_PIN_2, OUTPUT); }
  Serial.println("IRSniff ready.");
}

// Microsecond deltas detected between edge transitions
volatile int dts[MAX_PULSES];
volatile int dtIndex = 0;

// F-2F decoded message bits
byte bits[MAX_MESSAGE_BITS];
byte lastBits[MAX_MESSAGE_BITS];

// State variable that manages blinking between the two packet pins
bool packetPin = false;

void loop() {
  // Wait until the first pulse starts
  while (digitalRead(IR_PIN) == HIGH) ;
  unsigned long t0 = micros();

  // Wait until the second pulse starts
  while (digitalRead(IR_PIN) == LOW) ;
  unsigned long t1 = micros();

  // Compute future maximum pulse length (equal to length of first pulse)
  unsigned long dt0 = t1 - t0;
  addDt(dt0);

  // Record pulses
  int currentState;
  while (true) {
    currentState = digitalRead(IR_PIN);
    t0 = t1;
    while (digitalRead(IR_PIN) == currentState && t1 - t0 < dt0) {
      t1 = micros();
    }
    if (t1 - t0 >= dt0) {
      break;
    }
    addDt(t1 - t0);
  }

  // Mark the end of the packet
  addDt(END_OF_PACKET);

  // Blink the packet pins, if specified
  packetPin = !packetPin;
  if (PACKET_PIN_1 > 0) { digitalWrite(PACKET_PIN_1, packetPin ? HIGH : LOW); }
  if (PACKET_PIN_2 > 0) { digitalWrite(PACKET_PIN_2, packetPin ? LOW : HIGH); }

  // Print the packet information to serial
  printResults();
}

// Safely append an edge-to-edge pulse time to dts
void addDt(unsigned long dt) {
  if (dtIndex >= MAX_PULSES) {
    Serial.println("Buffer overrun; too many pulses at once.");
    while (true) ;
  }
  dts[dtIndex++] = (int)dt;
}

// Print out the information for all packets stored in dts
void printResults() {
  // Print out results packet by packet
  int i0 = 0;
  for (int i=0; i<dtIndex; i++) {
    if (dts[i] == END_OF_PACKET) {
      printResult(dts + i0, i - i0);
      i0 = i + 1;
    }
  }

  // Reset buffer
  dtIndex = 0;
}

// Print out information about a single packet
void printResult(int *pulseLengths, int nPulses) {
  if (nPulses < 3) {
    return;
  }

  // The nominal length of a slow pulse, or the total length of two quick pulses
  unsigned long dtSlow = pulseLengths[0] >> 2; // ((pulseLengths[0] >> 1) + (pulseLengths[1])) >> 2;

  // Minimum time of a valid slow pulse
  unsigned long dtSlow0 = dtSlow * 3 / 4;

  // Maximum time of a valid slow pulse
  unsigned long dtSlow1 = dtSlow * 5 / 4;
  
  int n = 0;

  int i = 2;
  while (i < nPulses) {
    if (i == 2 && pulseLengths[i] < dtSlow * 9 / 10 && pulseLengths[i+1] < dtSlow0) {
      // The first bit on grenade codes is extra long
      bits[n++] = 0;
      i += 2;
    } else if (i == 2 && pulseLengths[i] >= dtSlow0 && pulseLengths[i] < 2 * dtSlow0) {
      // The first bit on grenade codes is extra long
      bits[n++] = 1;
      i++;
    } else if (pulseLengths[i] > dtSlow0 && pulseLengths[i] < dtSlow1) {
      bits[n++] = 1;
      i++;
    } else if (pulseLengths[i] + pulseLengths[i+1] > dtSlow0 && pulseLengths[i] + pulseLengths[i+1] < dtSlow1) {
      bits[n++] = 0;
      i += 2;
    } else {
      if (PRINT_INVALID_PACKETS) {
        Serial.print("Invalid pulse length at pulse index "); Serial.print(i);
        Serial.print(" ("); Serial.print(dtSlow); Serial.print(", "); Serial.print(dtSlow >> 1);
        Serial.print("): ");
        printPulseTimings(pulseLengths, nPulses);
      }
      return;
    }
    if (n >= MAX_MESSAGE_BITS) {
      Serial.println("Buffer overrun; too many bits in one message.");
    }
  }

  bool sameMsg = true;
  for (int i=0; i<n; i++) {
    if (bits[i] != lastBits[i]) {
      sameMsg = false;
      break;
    }
  }

  if (!sameMsg) {
    for (int i=0; i<n; i++) {
      Serial.print(bits[i]);
      lastBits[i] = bits[i];
    }
    Serial.print(' ');
    printPacketInfo(bits);
    Serial.println();
  }
}

void printPulseTimings(int *pulseLengths, int nPulses) {
  for (int i=0; i<nPulses; i++) {
    Serial.print(pulseLengths[i]);
    if (i < nPulses-1) {
      Serial.print(' ');
    } else {
      Serial.println();
    }
  }
}

// Decode the meaning of the bits in a packet into human-readable form
void printPacketInfo(byte *bits) {
  byte a=bits[0], b=bits[1], c=bits[2], d=bits[3], e=bits[4], f=bits[5], g=bits[6], h=bits[7], i=bits[8], j=bits[9], k=bits[10], l=bits[11], m=bits[12], n=bits[13], o=bits[14], p=bits[15], q=bits[16], r=bits[17], s=bits[18], t=bits[19], u=bits[20];
  if (e==0 && f==0 && g==1 && h==0 && i==0 && j==0 && a ^ b ^ c ^ d > 0 && k ^ l ^ m ^ n ^ o ^ p ^ q > 0) {
    Serial.print("gun shot ");
    Serial.print(shotIndex(bits+1));
    Serial.print(", shooter ");
    Serial.print(shooterIndex(bits+11));
  } else if (j==0 && k==0 && l==1 & m==0 && n==1 && o==1 && p==1 && q==1) {
    Serial.print("grenade ");
    if      (a==0 && b==1 && c==0 && d==0) {
      Serial.print("pair0");
      if (e==1 && f==0 && g==0 && h==0 && i==1) { }
      else { Serial.print(" invalid"); }
    } else {
           if (a==1 && b==1 && c==1 && d==0) { Serial.print("disarm"); }
      else if (a==1 && b==0 && c==0 && d==0) { Serial.print("pair1"); }
      else if (a==1 && b==1 && c==0 && d==0) { Serial.print("explode"); bits[4] = 1 - bits[4]; }
      else { Serial.print("invalid"); }
      Serial.print(' '); Serial.print(grenadeId(bits+4));
    }
  } else {
    Serial.print("invalid");
  }
}

int shotIndex(byte *code) {
  if (byteMatch(code, 3, "001")) return 0;
  if (byteMatch(code, 3, "101")) return 1;
  if (byteMatch(code, 3, "111")) return 2;
  if (byteMatch(code, 3, "011")) return 3;
  if (byteMatch(code, 3, "010")) return 4;
  if (byteMatch(code, 3, "110")) return 5;
  if (byteMatch(code, 3, "100")) return 6;
  if (byteMatch(code, 3, "000")) return 7;
  return 999;
}

int shooterIndex(byte *code) {
  if (byteMatch(code, 6, "000001")) return 0;
  if (byteMatch(code, 6, "100001")) return 1;
  if (byteMatch(code, 6, "110001")) return 2;
  if (byteMatch(code, 6, "010001")) return 3;
  if (byteMatch(code, 6, "011001")) return 4;
  if (byteMatch(code, 6, "111001")) return 5;
  if (byteMatch(code, 6, "101001")) return 6;
  if (byteMatch(code, 6, "001001")) return 7;
  if (byteMatch(code, 6, "001101")) return 8;
  if (byteMatch(code, 6, "101101")) return 9;
  if (byteMatch(code, 6, "111101")) return 10;
  if (byteMatch(code, 6, "011101")) return 11;
  if (byteMatch(code, 6, "010101")) return 12;
  if (byteMatch(code, 6, "110101")) return 13;
  if (byteMatch(code, 6, "100101")) return 14;
  if (byteMatch(code, 6, "000101")) return 15;
  if (byteMatch(code, 6, "000111")) return 16;
  if (byteMatch(code, 6, "100111")) return 17;
  if (byteMatch(code, 6, "110111")) return 18;
  if (byteMatch(code, 6, "010111")) return 19;
  if (byteMatch(code, 6, "011111")) return 20;
  if (byteMatch(code, 6, "111111")) return 21;
  if (byteMatch(code, 6, "101111")) return 22;
  if (byteMatch(code, 6, "001111")) return 23;
  if (byteMatch(code, 6, "001011")) return 24;
  if (byteMatch(code, 6, "101011")) return 25;
  if (byteMatch(code, 6, "111011")) return 26;
  if (byteMatch(code, 6, "011011")) return 27;
  if (byteMatch(code, 6, "010011")) return 28;
  if (byteMatch(code, 6, "110011")) return 29;
  if (byteMatch(code, 6, "100011")) return 30;
  if (byteMatch(code, 6, "000011")) return 31;
  if (byteMatch(code, 6, "000010")) return 32;
  if (byteMatch(code, 6, "100010")) return 33;
  if (byteMatch(code, 6, "110010")) return 34;
  if (byteMatch(code, 6, "010010")) return 35;
  if (byteMatch(code, 6, "011010")) return 36;
  if (byteMatch(code, 6, "111010")) return 37;
  if (byteMatch(code, 6, "101010")) return 38;
  if (byteMatch(code, 6, "001010")) return 39;
  if (byteMatch(code, 6, "001110")) return 40;
  if (byteMatch(code, 6, "101110")) return 41;
  if (byteMatch(code, 6, "111110")) return 42;
  if (byteMatch(code, 6, "011110")) return 43;
  if (byteMatch(code, 6, "010110")) return 44;
  if (byteMatch(code, 6, "110110")) return 45;
  if (byteMatch(code, 6, "100110")) return 46;
  if (byteMatch(code, 6, "000110")) return 47;
  if (byteMatch(code, 6, "000100")) return 48;
  if (byteMatch(code, 6, "100100")) return 49;
  if (byteMatch(code, 6, "110100")) return 50;
  if (byteMatch(code, 6, "010100")) return 51;
  if (byteMatch(code, 6, "011100")) return 52;
  if (byteMatch(code, 6, "111100")) return 53;
  if (byteMatch(code, 6, "101100")) return 54;
  if (byteMatch(code, 6, "001100")) return 55;
  if (byteMatch(code, 6, "001000")) return 56;
  if (byteMatch(code, 6, "101000")) return 57;
  if (byteMatch(code, 6, "111000")) return 58;
  if (byteMatch(code, 6, "011000")) return 59;
  if (byteMatch(code, 6, "010000")) return 60;
  if (byteMatch(code, 6, "110000")) return 61;
  if (byteMatch(code, 6, "100000")) return 62;
  if (byteMatch(code, 6, "000000")) return 63;
  return 999;
}

int grenadeId(byte *code) {
  if (byteMatch(code, 5, "10001")) return 0;
  if (byteMatch(code, 5, "00000")) return 1;
  if (byteMatch(code, 5, "00011")) return 2;
  if (byteMatch(code, 5, "00101")) return 3;
  if (byteMatch(code, 5, "00110")) return 4;
  if (byteMatch(code, 5, "01001")) return 5;
  if (byteMatch(code, 5, "01010")) return 6;
  if (byteMatch(code, 5, "01100")) return 7;
  if (byteMatch(code, 5, "01111")) return 8;
  if (byteMatch(code, 5, "10010")) return 9;
  if (byteMatch(code, 5, "10100")) return 10;
  if (byteMatch(code, 5, "10111")) return 11;
  if (byteMatch(code, 5, "11000")) return 12;
  if (byteMatch(code, 5, "11011")) return 13;
  if (byteMatch(code, 5, "11101")) return 14;
  if (byteMatch(code, 5, "11110")) return 15;
  return 999;
}

bool byteMatch(byte* code, int len, const char* s) {
  for (int i=0; i<len; i++) {
    if (code[i] != s[i] - '0') {
      return false;
    }
  }
  return true;
}

