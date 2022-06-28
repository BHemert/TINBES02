#include <Arduino.h>
#include <EEPROM.h>
#include "instruction_set.h"
#include "test_programs.h"
#define STACKSIZE 32
#define memSize 255
const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

byte sp = 0;

char c;
int j = 0;

EERef noOfFiles = EEPROM[160];

byte memory[256];
byte maxNumberOfVars = 25;
typedef struct
{ // struct for the MEM table
  char name;
  byte type;
  int address;
  byte length;
  byte procesID;
} MEMtype;

MEMtype MEMTABLE[25] = {};
byte noOfVars = 0;
byte stack[STACKSIZE];

typedef struct
{
  char name[BUFFER_SIZE];
  byte id;
  byte state;
  byte PC;
  byte FP;
  byte adres;
  byte stack[STACKSIZE];
  byte sp;
  byte LR;
} ProcesType;

int ProcesID = 0;
int numberOfProcesses = 0;
#define maxNumberOfProcesses 10
ProcesType Proces[maxNumberOfProcesses]{};

typedef struct
{ // struct for the FAT table
  char name[BUFFER_SIZE];
  int startingPoint;
  int lengthOfFile;
} FATtype;

FATtype FAT[10] = {
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0},
    {"", 999, 0}};

void resetSerial()
{
  memset(buf, 0, sizeof(buf));
}
bool input_routine()
/* Reading the serial input and putting it in a buffer. */
{
  // read if any data is available
  while (Serial.available())
  {
    c = Serial.read();

    if (c == ' ' || c == '\n')
    {
      // return true en ga door naar functie
      c = 0x00;
      Serial.print(F("I received: "));
      Serial.println(buf);

      j = 0;
      return true;
    }
    else
    {
      buf[j] = c;
      j++;
      return false;
    }
  }
  return false;
}

//--------------------------------------Instructions------------------------------------------------------------------------
void runProcesses()
{
  for (byte i = 0; i < maxNumberOfProcesses; i++)
  {
    // Serial.println(Proces[i].state);
    if (Proces[i].state == 'r')
    {
      execute(i);
    }
  }
}
//=========================================================================================================================

//---------------------------------------Stack-------------------------------------------------------------------------
void pushByte(byte b, byte id)
{
  Proces[id].stack[Proces[id].sp++] = b;
}
void pushChar(char c, byte id)
{
  Proces[id].stack[Proces[id].sp++] = c;
  Proces[id].stack[Proces[id].sp++] = CHAR;
}
void pushInt(int i, byte id)
{
  Proces[id].stack[Proces[id].sp++] = highByte(i);
  Proces[id].stack[Proces[id].sp++] = lowByte(i);
  Proces[id].stack[Proces[id].sp++] = INT;
}
void pushFloat(float f, byte id)
{
  byte b[4];
  float *pf = (float *)b;
  *pf = f;
  for (int i = 3; i >= 0; i--)
  {
    Proces[id].stack[Proces[id].sp++] = b[i];
  }
  Proces[id].stack[Proces[id].sp++] = FLOAT;
}
void pushString(char *s, byte id)
{
  for (byte i = 0; i < strlen(s); i++)
  {
    Proces[id].stack[Proces[id].sp++] = s[i];
  }
  Proces[id].stack[Proces[id].sp++] = '\0';
  Proces[id].stack[Proces[id].sp++] = strlen(s);
  Proces[id].stack[Proces[id].sp++] = STRING;
}
byte popByte(byte id)
{
  return Proces[id].stack[--Proces[id].sp];
}
int popInt(byte id)
{
  return word(Proces[id].stack[--Proces[id].sp], Proces[id].stack[--Proces[id].sp]);
}
float popFloat(byte id)
{
  byte b[4];
  float *pf = (float *)b;
  for (byte i = 0; i < 4; i++)
  {
    b[i] = Proces[id].stack[--Proces[id].sp];
  }
  return *pf;
}
byte popString(byte id)
{
  byte lengte = Proces[id].stack[--Proces[id].sp];
  return Proces[id].sp - lengte;
}
//=========================================================================================================================

//--------------------------------------MEMORY-------------------------------------------------------------------------
int spaceInTable()
{
  for (byte i = 0; i < maxNumberOfVars; i++)
  {
    if (MEMTABLE[i].name == 0)
    {
      return i;
    }
  }
  return -1;
}
int findVar(byte *id)
{
  for (byte i = 0; i < maxNumberOfVars; i++)
  {
    if (*id == MEMTABLE[i].procesID)
    {
      return i;
    }
  }
  return -1;
}
int findVar(char *name, byte *id)
{
  for (byte i = 0; i < maxNumberOfVars; i++)
  {
    // Serial.println(MEMTABLE[i].name);
    // Serial.println(MEMTABLE[i].procesID);
    if (name == MEMTABLE[i].name && *id == MEMTABLE[i].procesID)
    {
      return i;
    }
  }
  return -1;
}
void sortMEM()
{
  bool sorted = false;
  byte tmp;
  char tempName;
  while (!sorted)
  {
    sorted = true;
    for (int i = 0; i < noOfVars - 1; i++)
    {
      if (MEMTABLE[i].address > MEMTABLE[i + 1].address)
      {
        strcpy(tempName, MEMTABLE[i].name);
        strcpy(MEMTABLE[i].name, MEMTABLE[i + 1].name);
        strcpy(MEMTABLE[i + 1].name, tempName);

        tempName = MEMTABLE[i].type;
        MEMTABLE[i].type = MEMTABLE[i + 1].type;
        MEMTABLE[i + 1].type = tempName;

        tmp = MEMTABLE[i].address;
        MEMTABLE[i].address = MEMTABLE[i + 1].address;
        MEMTABLE[i + 1].address = tmp;

        tmp = MEMTABLE[i].length;
        MEMTABLE[i].length = MEMTABLE[i + 1].length;
        MEMTABLE[i + 1].length = tmp;

        tmp = MEMTABLE[i].procesID;
        MEMTABLE[i].procesID = MEMTABLE[i + 1].procesID;
        MEMTABLE[i + 1].procesID = tmp;
        sorted = false;
      }
    }
  }
}
bool checkAllowed(char *name, byte *id)
{
  for (byte i = 0; i < maxNumberOfVars; i++)
  {
    if (*name == MEMTABLE[i].name && *id == MEMTABLE[i].procesID)
    {
      return true;
    }
  }
  return false;
}
void clearVar(char name, byte id)
{
  int i = findVar(&name, &id);
  MEMTABLE[i].name = 0x00;
  MEMTABLE[i].type = 0;
  MEMTABLE[i].address = 0;
  MEMTABLE[i].length = 0;
  MEMTABLE[i].procesID = 0;
  noOfVars -= 1;
}
void clearAllVars(byte id)
{
  while (true)
  {
    int i = findVar(&id);
    if (i == -1)
    {
      return;
    }
    else
    {
      MEMTABLE[i].name = 0x00;
      MEMTABLE[i].type = 0;
      MEMTABLE[i].address = 0;
      MEMTABLE[i].length = 0;
      MEMTABLE[i].procesID = 0;
      noOfVars -= 1;
    }
  }
}
int MEMspace(byte length)
{
  sortMEM();
  byte prev = 0;
  for (byte i = 0; i < noOfVars; i++)
  {
    if (MEMTABLE[i].address - prev > length)
    {
      return prev;
    }
    prev = MEMTABLE[i].address + MEMTABLE[i].length;
  }
  if (memSize - prev > length)
  {
    return prev;
  }
  return -1;
}
void printStack(byte id)
{
  for (byte i = 0; i < Proces[id].sp; i++)
  {
    Serial.print(Proces[id].stack[i]);
    Serial.print(F(" "));
  }
  Serial.println();
}
void setVar(char name, byte id)
{
  int posInMEMTable;
  int posInMEM;
  byte type;
  byte lengte;
  if (noOfVars >= maxNumberOfVars)
  { // geen variabele meer mogelijk. max bereikt.
    Serial.println(F("Maximum number of variables in use"));
    return;
  }
  if (checkAllowed(&name, &id))
  {
    posInMEMTable = findVar(&name, &id);
    posInMEM = MEMTABLE[posInMEMTable].address;
    type = popByte(id);
    lengte = type;
    if (type == STRING)
    {
      lengte = popByte(id);
      posInMEM = MEMspace(lengte);
    }
    clearVar(name, id); // verwijder oude variabele
  }
  else
  {
    type = popByte(id); // pop type van stack
    lengte = type;
    if (type == STRING)
    {
      lengte = popByte(id);
    }
    posInMEM = MEMspace(lengte);
    if (posInMEM == -1)
    {
      Serial.println(F("not enough space to store variable"));
      pushByte(type, id);
      return;
    }
    posInMEMTable = spaceInTable();
    if (posInMEMTable == -1)
    {
      Serial.println(F("not able to store variable"));
      pushByte(type, id);
      return;
    }
  }
  for (int i = posInMEM + lengte - 1; i >= posInMEM; i--)
  {
    memory[i] = popByte(id);
    // Serial.println(memory[i]);
  }
  MEMTABLE[posInMEMTable].name = name;
  MEMTABLE[posInMEMTable].type = type;
  MEMTABLE[posInMEMTable].address = posInMEM;
  MEMTABLE[posInMEMTable].length = lengte;
  MEMTABLE[posInMEMTable].procesID = id;
  noOfVars += 1;
  printMEMTable();
  return;
}
void getVar(char name, byte id)
{
  int posInMEMTable = findVar(&name, &id);
  if (posInMEMTable == -1)
  {
    Serial.println(F("variable not found"));
    return;
  }
  // Serial.println("Get");
  for (int i = MEMTABLE[posInMEMTable].address; i < MEMTABLE[posInMEMTable].address + MEMTABLE[posInMEMTable].length; i++)
  {
    pushByte(memory[i], id);
    // Serial.println(memory[i]);
  }
  // Serial.println("//////////////////////////////////////////");
  if (MEMTABLE[posInMEMTable].type == STRING)
  {
    pushByte(MEMTABLE[posInMEMTable].length, id);
  }
  pushByte(MEMTABLE[posInMEMTable].type, id);
}
float popVal(byte id)
{
  byte type = Proces[id].stack[Proces[id].sp];
  if (type == INT)
  {
    return popInt(id);
  }
  else
  {
    return popFloat(id);
  }
}
void printMEMTable()
{
  Serial.println(F("======================= MEMTABLE ======================"));
  for (byte i = 0; i < maxNumberOfVars; i++)
  {
    Serial.print(F("name: "));
    Serial.print(MEMTABLE[i].name);
    Serial.print(F(" type: "));
    Serial.print(MEMTABLE[i].type);
    Serial.print(F(" address: "));
    Serial.print(MEMTABLE[i].address);
    Serial.print(F(" length: "));
    Serial.print(MEMTABLE[i].length);
    Serial.print(F(" procesID: "));
    Serial.println(MEMTABLE[i].procesID);
  }
  Serial.println(F("========================================================="));
}
//========================================================================================================================

//--------------------------------------FATTABLE--------------------------------------------------------------------------
void writeNoOfFiles()
{
  EEPROM.write(160, noOfFiles);
}
void readNoOfFiles()
{
  noOfFiles = EEPROM.read(160);
  Serial.print(F("noOfFiles: "));
  Serial.println(noOfFiles);
}
void writeFATEntry()
{
  for (unsigned int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    EEPROM.put(i * sizeof(FATtype), FAT[i]);
  }
}
void sortFATTable()
{
  bool sorted = false;
  long tmp;
  char s[BUFFER_SIZE];
  while (!sorted)
  {
    sorted = true;
    for (int i = 0; i < 10 - 1; i++) // 10 uiteindelijk numberOfFiles NOITEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    {
      if (FAT[i].startingPoint > FAT[i + 1].startingPoint)
      {

        tmp = FAT[i].startingPoint;
        FAT[i].startingPoint = FAT[i + 1].startingPoint;
        FAT[i + 1].startingPoint = tmp;
        tmp = FAT[i].lengthOfFile;
        FAT[i].lengthOfFile = FAT[i + 1].lengthOfFile;
        FAT[i + 1].lengthOfFile = tmp;
        strcpy(s, FAT[i].name);
        strcpy(FAT[i].name, FAT[i + 1].name);
        strcpy(FAT[i + 1].name, s);
        sorted = false;
      }
    }
  }
}
void readFATEntry()
{
  for (unsigned int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    EEPROM.get(i * sizeof(FATtype), FAT[i]);
  }
}
int fileNameExists(char *inputName)
{
  for (int i = 0; i < 10; i++) // 10 moet numberOfFiles worden
  {
    if (strcmp(FAT[i].name, inputName) == 0)
    {
      return i;
    }
  }
  return -1;
}
int searchEmptyFATEntry()
{
  for (unsigned int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    if (strcmp(FAT[i].name, "") == 0)
    {
      Serial.println(F("Found free FAT entry"));
      return i;
    }
  }
  return -1;
}
int searchFreeFATStartingPoint(int *lengthOfFile)
{
  Serial.println(F("Searching for free FAT starting point"));
  // sortFATTable();
  int prev = 0;

  for (int i = 0; i < 10; i++) // 10 moet number of files worden
  {
    if (FAT[i].startingPoint - prev > *lengthOfFile)
      return prev;

    Serial.print(F("Free starting point found at: "));
    Serial.println(prev);

    prev = FAT[i].startingPoint + FAT[i].lengthOfFile;
  }
  return -1;
}
void printFATTable()
{
  readFATEntry(); // uitcommenten voor testen
  sortFATTable(); // uitcomenten voor testen

  Serial.println(F("======================= FAT TABLE ======================"));
  for (unsigned int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    Serial.print(FAT[i].name);
    Serial.print(F(" "));
    Serial.print(FAT[i].startingPoint);
    Serial.print(F(" "));
    Serial.println(FAT[i].lengthOfFile);
  }
  Serial.println(F("========================================================="));
}
void freespace()
{
  int largestFreeSpace = 0;
  int largestFreeSpaceIndex = 0;
  for (unsigned int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    if (FAT[i].startingPoint + FAT[i].lengthOfFile > largestFreeSpace)
    {
      largestFreeSpace = FAT[i].startingPoint + FAT[i].lengthOfFile;
      largestFreeSpaceIndex = i;
    }
  }
  Serial.print(F("Largest free space found at: "));
  Serial.println(largestFreeSpaceIndex);
  Serial.print(F("Of size: "));
  Serial.println(largestFreeSpace);
}
void retrieve()
{
  Serial.println(F("in retrieve"));

  readFATEntry();
  sortFATTable();
  resetSerial();
  Serial.println(F("Enter file name: "));
  while (input_routine() == false)
  {
    runProcesses();
  }
  char inputName[BUFFER_SIZE];
  strcpy(inputName, buf);
  int k = fileNameExists(inputName);
  if (k == -1)
  {
    Serial.println(F("File does not exist"));
    return;
  }

  Serial.print(F("Starting point: "));
  Serial.println(FAT[k].startingPoint);
  Serial.print(F("File length: "));
  Serial.println(FAT[k].lengthOfFile);
  Serial.print(F("File name: "));
  Serial.println(FAT[k].name);

  for (int i = 0; i < FAT[k].lengthOfFile; i++)

  {
    char c = 0x00;
    EEPROM.get(FAT[k].startingPoint + 161 + i, c);
    Serial.print(c);
    delayMicroseconds(40);
  }
  Serial.println(F(""));
}
void erase()
{
  Serial.println(F("in erase"));
  readFATEntry();
  sortFATTable();
  readNoOfFiles();
  resetSerial();
  Serial.println(F("Enter file name: "));
  while (input_routine() == false)
  {
    runProcesses();
  }
  char inputName[BUFFER_SIZE];
  strcpy(inputName, buf);
  signed int k = fileNameExists(inputName);
  if (k == -1)
  {
    Serial.println(F("File does not exist"));
    return;
  }
  strcpy(FAT[k].name, "");
  FAT[k].startingPoint = 999;
  FAT[k].lengthOfFile = 0;
  noOfFiles--;
  sortFATTable();
  writeFATEntry();
  readFATEntry();
  sortFATTable();
  writeNoOfFiles();
  readNoOfFiles();
}
void store()
{
  readFATEntry();
  sortFATTable();
  readNoOfFiles();
  Serial.println(F("in store"));
  resetSerial();
  Serial.println(F("Enter file name: "));
  while (input_routine() == false)
  {
    runProcesses();
  }
  char inputName[BUFFER_SIZE];
  strcpy(inputName, buf);
  resetSerial();
  Serial.println(F("Enter content length: "));
  while (input_routine() == false)
  {
    runProcesses();
  }
  int inputLength = atoi(buf);
  resetSerial();
  Serial.println(F("Enter content: "));
  while (input_routine() == false)
  {
    runProcesses();
  }
  char inputData[BUFFER_SIZE];
  strcpy(inputData, buf);

  if (searchEmptyFATEntry() == -1)
  {
    Serial.println(F("No free space in FAT"));
    return;
  }
  if (fileNameExists(inputName) != -1)
  {
    Serial.println(F("FileName already exists"));
    return;
  }
  int f = searchFreeFATStartingPoint(&inputLength);
  if (f == -1)
  {
    Serial.println(F("No free space in EEPROM"));
    return;
  }
  Serial.print(F("Free starting point: "));
  Serial.println(f);

  FAT[noOfFiles].startingPoint = f;
  FAT[noOfFiles].lengthOfFile = inputLength;
  strcpy(FAT[noOfFiles].name, inputName);
  noOfFiles++;
  for (int i = 0; i < inputLength; i++)
  {
    // Serial.print(inputData[i]);
    EEPROM.put(f + i + 161, inputData[i]);
  }
  writeNoOfFiles();
  readNoOfFiles();

  sortFATTable();
  writeFATEntry();
  readFATEntry();
  printFATTable();
}
void files()

{
  readFATEntry();
  sortFATTable();
  readNoOfFiles();
  Serial.println(F("in files"));
  Serial.println(F("===================== ALL FILES ========================"));
  for (int i = 0; i < noOfFiles; i++)
  {
    Serial.print(F("File name: "));
    Serial.print(FAT[i].name);
    Serial.print(F(" | "));
    Serial.print(F("File Length: "));
    Serial.println(FAT[i].lengthOfFile);
  }
  Serial.println(F("=========================================================="));
}
//=========================================================================================================================

//---------------------------------------Processes-------------------------------------------------------------------------
void changeState(int *index, byte state)
{
  if (Proces[*index].state == state)
  {
    Serial.println(F("proces already in this state"));
    return;
  }
  Proces[*index].state = state;
  Serial.println(F("proces has changed states"));
}
int checkAvailable(byte *id, byte state)
{
  for (int i = 0; i < maxNumberOfProcesses; i++)
  {
    if (Proces[i].state == state and Proces[i].id == *id)
    {
      return i;
    }
  }
  return -1;
}
int checkAvailable(byte *id)
{
  for (int i = 0; i < maxNumberOfProcesses; i++)
  {
    if (Proces[i].id == *id)
    {
      return i;
    }
  }
  return -1;
}
int spaceInProcesTable()
{
  for (int i = 0; i < maxNumberOfProcesses; i++)
  {
    if (strcmp(Proces[i].name, "") == 0)
    {
      return i;
    }
  }
  return -1;
}
void run()
{
  Serial.println(F("in run"));
  Serial.println(F("give the name of the process you want to run"));
  resetSerial();
  while (input_routine() == false)
  { // read the file name
    runProcesses();
  }
  char fileName[BUFFER_SIZE];
  strcpy(fileName, buf);
  resetSerial();
  if (numberOfProcesses >= maxNumberOfProcesses)
  { // geen variabele meer mogelijk. max bereikt.
    Serial.println(F("max number of processes in use"));
    return;
  }
  int fileInFAT = fileNameExists(fileName);
  if (fileInFAT == -1)
  {
    Serial.println(F("no file with this name exists"));
    return;
  }
  numberOfProcesses += 1;
  int spaceInTable = spaceInProcesTable();
  strncpy(Proces[spaceInTable].name, fileName, BUFFER_SIZE);
  Proces[spaceInTable].id = ProcesID;
  changeState(&spaceInTable, 'r');
  Proces[spaceInTable].PC = 0;
  Proces[spaceInTable].adres = FAT[fileInFAT].startingPoint;
  Proces[spaceInTable].sp = 0;
  Serial.println(F("proces started"));
  ProcesID = ProcesID + 1;
}
void suspend()
{
  Serial.println(F("in suspend"));
  Serial.println(F("give the id of the process you want to suspend"));
  resetSerial();
  while (input_routine() == false)
  { // read the file name
    runProcesses();
  }
  byte id = atoi(buf);
  int spaceInTable = checkAvailable(&id, 'r');
  if (spaceInTable == -1)
  {
    Serial.println(F("process not running or non existant"));
    return;
  }
  resetSerial();
  changeState(&spaceInTable, 'p');
}
void resume()
{
  Serial.println(F("in resume"));
  Serial.println(F("give the id of the process you want to resume"));
  resetSerial();
  while (input_routine() == false)
  { // read the file name
    runProcesses();
  }

  byte id = atoi(buf);
  int spaceInTable = checkAvailable(&id, 'p');
  if (spaceInTable == -1)
  {
    Serial.println(F("proces is not paused or does not exist"));
    return;
  }
  resetSerial();
  changeState(&spaceInTable, 'r');
}
void killProces(int pos, int id)
{
  strncpy(Proces[pos].name, "", BUFFER_SIZE);
  Proces[pos].id = 0;
  Proces[pos].PC = 0;
  Proces[pos].adres = 0;
  Proces[pos].sp = 0;
  changeState(&pos, 0);
  numberOfProcesses = numberOfProcesses - 1;
  clearAllVars(id);
}
void kill()
{
  Serial.println(F("kill"));
  Serial.println(F("give the id of the proces you want to kill"));
  resetSerial();
  while (input_routine() == false)
  { // read the file name
    runProcesses();
  }
  byte id = atoi(buf);
  int spaceInTable = checkAvailable(&id);
  if (spaceInTable == -1 || Proces[spaceInTable].state == 0)
  {
    Serial.println(F("Not able to terminate. Program may not be available"));
    return;
  }
  resetSerial();
  killProces(spaceInTable, id);
}
void pushVars(byte id)
{
  byte lengte;
  byte variable;
  EEPROM.get(Proces[id].adres + Proces[id].PC + 161, lengte);
  for (byte i = 0; i < lengte; i++)
  {
    Proces[id].PC++;
    EEPROM.get(Proces[id].adres + Proces[id].PC + 161, variable);
    pushByte(variable, id);
  }
  pushByte(lengte, id);
}
void printCommand(byte id)
{
  //  Serial.println("----------------------------------------------------");
  //  printStack(id);
  //  Serial.println("----------------------------------------------------");

  byte type = popByte(id);
  if (type == CHAR)
  {
    Serial.print((char)popByte(id));
  }
  else if (type == INT)
  {
    Serial.print((int)popInt(id));
  }
  else if (type == FLOAT)
  {
    Serial.print(popFloat(id));
  }
  else if (type == STRING)
  {
    byte pointer = popString(id);
    while (true)
    {
      if (Proces[id].stack[pointer] == 0)
      {
        byte sp = Proces[id].sp;
        Proces[id].sp = sp - pointer;
        return;
      }
      else
      {
        Serial.print((char)Proces[id].stack[pointer]);
        pointer++;
      }
    }
  }
}
void printProcessTable()
{
  Serial.println(F("===================== ALL PROCESSES ========================"));
  for (int i = 0; i < maxNumberOfProcesses; i++)
  {
    Serial.print(F("Process name: "));
    Serial.print(Proces[i].name);
    Serial.print(F(" | "));
    Serial.print(F("Process ID: "));
    Serial.print(Proces[i].id);
    Serial.print(F(" | "));
    Serial.print(F("Process state: "));
    Serial.print(Proces[i].state);
    Serial.print(F(" | "));
    Serial.print(F("Process PC: "));
    Serial.print(Proces[i].PC);
    Serial.print(F(" | "));
    Serial.print(F("Process adres: "));
    Serial.println(Proces[i].adres);
  }
  Serial.println(F("=========================================================="));
}
//=========================================================================================================================

//--------------------------------------Instructions------------------------------------------------------------------------
void storehello()
{
  char fileName[BUFFER_SIZE] = "hello";
  int posInEEPROM = 161;
  int lengthOfFile = sizeof(prog1);
  Serial.println(lengthOfFile);
  strncpy(FAT[0].name, fileName, BUFFER_SIZE);
  FAT[0].startingPoint = posInEEPROM;
  FAT[0].lengthOfFile = lengthOfFile;
  EEPROM.put(posInEEPROM + 161, prog1);
  noOfFiles += 1;
  writeFATEntry();
}
void storeVars()
{
  char fileName[BUFFER_SIZE] = "test_vars";
  int posInEEPROM = 161;
  int lengthOfFile = sizeof(prog2);
  Serial.println(lengthOfFile);
  strncpy(FAT[0].name, fileName, BUFFER_SIZE);
  FAT[0].startingPoint = posInEEPROM;
  FAT[0].lengthOfFile = lengthOfFile;
  EEPROM.put(posInEEPROM + 161, prog2);
  noOfFiles += 1;
  writeFATEntry();
}
void storeLoop()
{
  char fileName[BUFFER_SIZE] = "test_loop";
  int posInEEPROM = 161;
  int lengthOfFile = sizeof(prog3);
  Serial.println(lengthOfFile);
  strncpy(FAT[0].name, fileName, BUFFER_SIZE);
  FAT[0].startingPoint = posInEEPROM;
  FAT[0].lengthOfFile = lengthOfFile;
  EEPROM.put(posInEEPROM + 161, prog3);
  noOfFiles += 1;
  writeFATEntry();
}
void storeif()
{
  char fileName[BUFFER_SIZE] = "test_if";
  int posInEEPROM = 161;
  int lengthOfFile = sizeof(prog4);
  Serial.println(lengthOfFile);
  strncpy(FAT[0].name, fileName, BUFFER_SIZE);
  FAT[0].startingPoint = posInEEPROM;
  FAT[0].lengthOfFile = lengthOfFile;
  EEPROM.put(posInEEPROM + 161, prog4);
  noOfFiles += 1;
  writeFATEntry();
}
void binairOperators(byte command, byte id)
{
  float y;
  byte typeY = popByte(id);
  if (typeY == CHAR)
  {
    char c = (char)popByte(id);
    y = (float)c;
  }
  else if (typeY == INT)
  {
    y = (float)popInt(id);
  }
  else if (typeY == FLOAT)
  {
    y = popFloat(id);
  }
  float x;
  byte typeX = popByte(id);
  if (typeX == CHAR)
  {
    char c = (char)popByte(id);
    x = (float)c;
  }
  else if (typeX == INT)
  {
    x = (float)popInt(id);
  }
  else if (typeX == FLOAT)
  {
    x = popFloat(id);
  }

  // doe de instructies

  switch (command)
  {
  case PLUS:
    x = x + y;
    break;
  case MINUS:
    x = x - y;
    break;
  case TIMES:
    x = x * y;
    break;
  case DIVIDEDBY:
    x = x / y;
    break;
  case MODULUS:
    x = (int)x % (int)y;
    break;
  case EQUALS:
    typeX = CHAR;
    if (x == y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case NOTEQUALS:
    typeX = CHAR;
    if (x == y)
    {
      x = 0;
    }
    else
    {
      x = 1;
    }
    break;
  case LESSTHAN:
    typeX = CHAR;
    if (x < y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case LESSTHANOREQUALS:
    typeX = CHAR;
    if (x <= y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case GREATERTHAN:
    typeX = CHAR;
    if (x > y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case GREATERTHANOREQUALS:
    typeX = CHAR;
    if (x >= y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case MIN:
    if (x < y)
    {
      x = x;
    }
    else
    {
      x = y;
    }
    break;
  case MAX:
    if (x > y)
    {
      x = x;
    }
    else
    {
      x = y;
    }
    break;
  case POW:
    x = pow(x, y);
    break;
  case LOGICALAND:
    typeX = CHAR;
    if (x && y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case LOGICALOR:
    typeX = CHAR;
    if (x || y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case LOGICALXOR:
    typeX = CHAR;
    if (x && !y || !x && y)
    {
      x = 1;
    }
    else
    {
      x = 0;
    }
    break;
  case BITWISEAND:
    x = (int)x & (int)y;
    break;
  case BITWISEOR:
    x = (int)x | (int)y;
    break;
  case BITWISEXOR:
    x = (int)x ^ (int)y;
    break;
  }

  if (typeY == FLOAT || typeX == FLOAT)
  {
    pushFloat(x, id);
  }
  else if (typeY == INT || typeX == INT)
  {
    pushInt((int)x, id);
  }
  else if (typeY == CHAR || typeX == CHAR)
  {
    pushByte((byte)x, id);
  }
}
void unairOperators(byte command, byte id)
{
  float value;
  // pop waarde van stack
  byte type = popByte(id);
  if (type == CHAR)
  {
    char c = (char)popByte(id);
    value = (float)c;
  }
  else if (type == INT)
  {
    value = (float)popInt(id);
  }
  else if (type == FLOAT)
  {
    value = popFloat(id);
  }

  // doe de instructie
  switch (command)
  {
  case INCREMENT:
    value += 1;
    break;
  case DECREMENT:
    value -= 1;
    break;
  case UNARYMINUS:
    value = value * -1;
    break;
  case ABS:
    if (value <= 0)
    {
      value = value * -1;
    }
    break;
  case SQ:
    value = pow(value, 2);
    break;
  case SQRT:
    value = sqrt(value);
    break;
  case ANALOGREAD:
    value = analogRead(value);
    type = INT;
    break;
  case DIGITALREAD:
    value = digitalRead(value);
    type = CHAR;
    break;
  case LOGICALNOT:
    if (value == 0)
    {
      value = 1;
    }
    else
    {
      value = 0;
    }
    type = CHAR;
    break;
  case BITWISENOT:
    value = ~(byte)value;
    break;
  case TOCHAR:
    type = CHAR;
    break;
  case TOINT:
    type = INT;
    break;
  case TOFLOAT:
    type = FLOAT;
    break;
  case ROUND:
    round(value);
    type = INT;
    break;
  case FLOOR:
    value = floor(value);
    type = INT;
    break;
  case CEIL:
    value = ceil(value);
    type = INT;
    break;
  }
  // push waarde
  if (type == CHAR)
  {
    pushChar((char)value, id);
  }
  else if (type == INT)
  {
    pushInt((int)value, id);
  }
  else if (type == FLOAT)
  {
    pushFloat(value, id);
  }
}
void execute(byte id)
{
  // Serial.print("ID: ");
  // Serial.println(id);
  // printProcessTable();
  byte lengthOfString;
  byte x;
  int timer;
  byte command;
  EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
  Serial.println(command);
  switch (command)
  {
  case CHAR:
  case INT:
  case FLOAT:
    pushVars(id);
    break;
  case STRING:
    lengthOfString = 0;
    while (command != 0)
    {
      Proces[id].PC++;
      lengthOfString++;
      EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
      pushByte(command, id);
    }
    pushByte(lengthOfString, id);
    pushByte(STRING, id);
    lengthOfString = 0;
    break;
  case PRINT:
    printCommand(id);
    break;
  case PRINTLN:
    printCommand(id);
    Serial.println();
    break;
  case SET:
    Proces[id].PC++;
    EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
    setVar((char)command, id);
    // printMEMTable();
    break;
  case GET:
    Proces[id].PC++;
    EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
    getVar((char)command, id);
    break;
  case MILLIS:
    pushInt((int)millis(), id);
    break;
  case DELAYUNTIL:
    popByte(id);
    timer = popInt(id);
    if (!((int)millis() > timer))
    {
      pushInt(timer, id);
      return;
    }
    break;
  case LOOP:
    Proces[id].LR = Proces[id].PC;
    break;
  case ENDLOOP:
    Proces[id].PC = Proces[id].LR;
    break;
  case INCREMENT:
  case DECREMENT:
  case UNARYMINUS:
  case ABS:
  case SQ:
  case SQRT:
  case ANALOGREAD:
  case DIGITALREAD:
  case LOGICALNOT:
  case BITWISENOT:
  case TOCHAR:
  case TOINT:
  case TOFLOAT:
  case ROUND:
  case FLOOR:
  case CEIL:
    unairOperators(command, id);
    break;
  case PLUS:
  case MINUS:
  case TIMES:
  case DIVIDEDBY:
  case MODULUS:
  case EQUALS:
  case NOTEQUALS:
  case LESSTHAN:
  case LESSTHANOREQUALS:
  case GREATERTHAN:
  case GREATERTHANOREQUALS:
  case MIN:
  case MAX:
  case POW:
  case LOGICALAND:
  case LOGICALOR:
  case LOGICALXOR:
  case BITWISEAND:
  case BITWISEOR:
  case BITWISEXOR:
    binairOperators(command, id);
    break;
  case STOP:
    killProces(id, Proces[id].id);
    break;
  case IF:
    Proces[id].PC++;
    EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
    x = popByte(id);
    if (x == 0)
    {
      Proces[id].PC = Proces[id].PC + command;
    }
    pushByte(x, id);
    break;
  case ELSE:
    Proces[id].PC++;
    EEPROM.get(Proces[id].adres + Proces[id].PC + 161, command);
    x = popByte(id);
    if (x != 0)
    {
      Proces[id].PC = Proces[id].PC + command;
    }
    pushByte(x, id);
    break;
  case ENDIF:
    x = popByte(id);
    break;
  }
  Proces[id].PC++;
}
//=========================================================================================================================

//--------------------------------------------RESET------------------------------------------------------------------------
void clearTables()
{
  writeFATEntry();
}
//=========================================================================================================================
typedef struct
{
  char name[BUFFER_SIZE];
  void (*func)();

} commandType;
static commandType command[] =
    {
        {"store", &store},
        {"retrieve", &retrieve},
        {"printfat", &printFATTable},
        {"printmem", &printMEMTable},
        {"printproces", &printProcessTable},
        {"erase", &erase},
        {"files", &files},
        {"freespace", &freespace},
        {"suspend", &suspend},
        {"resume", &resume},
        {"kill", &kill},
        {"run", &run},
        {"storehello", &storehello},
        {"storevars", &storeVars},
        {"storeLoop", &storeLoop},
        {"storeif", &storeif},
        {"cleartables", &clearTables},
        {"", NULL}};

void proccesInput()
{
  for (unsigned int i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    if (strcmp(command[i].name, buf) == 0)
    {
      Serial.println(F("Executing command"));
      command[i].func();
      return;
    }
  }
  Serial.println(F("Given command not found choose one of following: "));
  for (unsigned int i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    Serial.print(command[i].name);
    Serial.print(" ");
  }
  return; // return to main loop
}

void setup()
{
  Serial.begin(9600);
  Serial.println(F("Arduino OS 1.0 Ready"));
  // RESET
  writeFATEntry();
  noOfFiles = 0;
  writeNoOfFiles();
  // RESET
  readNoOfFiles();
  readFATEntry();
}

void loop()
{

  if (input_routine() == true)
  {

    proccesInput();
    resetSerial();
  }
  else

  {
    runProcesses();
  }
}
