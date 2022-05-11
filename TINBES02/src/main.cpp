#include <Arduino.h>
#include <EEPROM.h>

const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

char c;
int j = 0;

EERef noOfFiles = EEPROM[160];

byte noOfVars = 0;

byte memory[256];

typedef struct
{               // struct for the MEM table
  char name[8]; // 1 byte dus 8 bits?
  char type[BUFFER_SIZE];
  int address;
  int length;
  int procesID;
} MEMtype;

MEMtype MEMTABLE[25] = {
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0},
    {"", "", 0, 0, 0}};

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

#define STACKSIZE 32
byte stack[STACKSIZE];
byte sp = 0;
void pushByte(byte b)
{
  stack[sp++] = b;
}
byte popByte()
{
  return stack[--sp];
}

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
  for (int i = 0; i < noOfFiles; i++) // 10 moet numberOfFiles worden
  {
    if (strcmp(FAT[i].name, inputName) == 0)
    {
      return i;
    }
  }
  return -1;
}
void runProccesses()
{
  int x = 0;
}
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
  while (input_routine() == false)
  {
    runProccesses();
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
    delay(40);
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
  while (input_routine() == false)
  {
    runProccesses();
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
  while (input_routine() == false)
  {
    runProccesses();
  }
  char inputName[BUFFER_SIZE];
  strcpy(inputName, buf);
  resetSerial();
  while (input_routine() == false)
  {
    runProccesses();
  }
  int inputLength = atoi(buf);
  resetSerial();
  while (input_routine() == false)
  {
    runProccesses();
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

void readStack()
{
  for (int i = 0; i < STACKSIZE; i++)
  {
    Serial.print(stack[i]);
    Serial.print(F(" "));
  }
}
void sortMEMTable()
{
  bool sorted = false;
  long tmp;
  char b[BUFFER_SIZE];
  while (!sorted)
  {
    sorted = true;
    for (int i = 0; i < 25 - 1; i++) // 10 uiteindelijk numberOfFiles NOITEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE
    {
      if (MEMTABLE[i].address > MEMTABLE[i + 1].address)
      {

        tmp = MEMTABLE[i].address;
        MEMTABLE[i].address = MEMTABLE[i + 1].address;
        MEMTABLE[i + 1].address = tmp;
        tmp = MEMTABLE[i].length;
        MEMTABLE[i].length = MEMTABLE[i + 1].length;
        MEMTABLE[i + 1].length = tmp;
        strcpy(b, MEMTABLE[i].name);
        strcpy(MEMTABLE[i].name, MEMTABLE[i + 1].name);
        strcpy(MEMTABLE[i + 1].name, b);
        sorted = false;
      }
    }
  }
}
bool searchEmptyMEMEntry()
{
  for (unsigned i = 0; i < sizeof(MEMTABLE) / sizeof(MEMtype); i++)
  {
    if (strcmp(MEMTABLE[i].name, "") == 0)
    {
      return true;
    }
  }
  return false;
}
void checkIfNameExists(char *inputName)
{
  for (unsigned i = 0; i < sizeof(MEMTABLE) / sizeof(MEMtype); i++)
  {
    if (strcmp(MEMTABLE[i].name, inputName) == 0)
    {
      Serial.println(F("Name already exists"));
      strcpy(MEMTABLE[i].name, "");
      MEMTABLE[i].length = 0;
      MEMTABLE[i].procesID = 0;
      MEMTABLE[i].length = 0;
      noOfVars--;
    }
  }
}
void storeVar()
{
  Serial.println(F("in storeVar"));

  resetSerial();
  while (input_routine() == false)
  {
    runProccesses();
  }
  char inputName[8];
  strcpy(inputName, buf);
  resetSerial();
  while (input_routine() == false)
  {
    runProccesses();
  }
  int procesID = atoi(buf);
  resetSerial();

  checkIfNameExists(inputName);
  if (searchEmptyMEMEntry() == false)
  {
    Serial.println(F("No free space in MEM"));
    return;
  }
  readStack();
}

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
        {"erase", &erase},
        {"files", &files},
        {"freespace", &freespace},
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
  // writeFATEntry();
  // noOfFiles = 0;
  // writeNoOfFiles();
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
    // Serial.println(F("NON BLOCKING"));
  }
}
