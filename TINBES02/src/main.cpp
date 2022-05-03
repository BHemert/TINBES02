#include <Arduino.h>
#include <EEPROM.h>

const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

char c;
int j = 0;

EERef noOfFiles = EEPROM[160];

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

void writeNoOfFiles()
{
  EEPROM.write(160, noOfFiles);
}
void readNoOfFiles()
{
  noOfFiles = EEPROM.read(160);
  Serial.print("noOfFiles: ");
  Serial.println(noOfFiles);
}
void writeFATEntry()
{
  for (int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
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
  for (int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
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
{
  // read if any data is available
  while (Serial.available())
  {
    c = Serial.read();

    if (c == ' ' || c == '\n')
    {
      // return true en ga door naar functie
      c = 0x00;
      Serial.print("I received: ");
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
}
int searchEmptyFATEntry()
{
  for (int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
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
  {
    for (int i = 0; i < 10; i++) // 10 moet number of files worden
    {
      if (FAT[i].startingPoint - prev > *lengthOfFile)
        return prev;

      Serial.print("Free starting point found at: ");
      Serial.println(prev);

      prev = FAT[i].startingPoint + FAT[i].lengthOfFile;
    }
  }
}
void printFATTable()
{
  readFATEntry(); // uitcommenten voor testen
  sortFATTable(); // uitcomenten voor testen

  Serial.println("FAT table:");
  Serial.println("==========================================================");
  for (int i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    Serial.print(FAT[i].name);
    Serial.print(" ");
    Serial.print(FAT[i].startingPoint);
    Serial.print(" ");
    Serial.println(FAT[i].lengthOfFile);
  }
  Serial.println("==========================================================");
}
void retrieve()
{
  Serial.println("in retrieve");
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
  Serial.print("File length: ");
  Serial.println(FAT[k].lengthOfFile);
  Serial.print("File name: ");
  Serial.println(FAT[k].name);

  for (int i = 0; i < FAT[k].lengthOfFile; i++)

  {
    char c = 0x00;
    EEPROM.get(FAT[k].startingPoint + 161 + i, c);
    Serial.print(c);
    delay(40);
  }
  Serial.println("");
}
void erase()
{
  Serial.println("in erase");
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
  int k = fileNameExists(inputName);
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
  Serial.println("in store");
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
    Serial.println("No free space in FAT");
    return;
  }
  if (fileNameExists(inputName) != -1)
  {
    Serial.println("FileName already exists");
    return;
  }
  int f = searchFreeFATStartingPoint(&inputLength);
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
        {"", NULL},
        {"", NULL}};

void proccesInput()
{
  for (int i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    if (strcmp(command[i].name, buf) == 0)
    {
      Serial.println(F("Executing command"));
      command[i].func();
      return;
    }
  }
  Serial.println(F("Given command not found choose one of following: "));
  for (int i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    Serial.print(command[i].name);
    Serial.print(" ");
  }
  return; // return to main loop
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Arduino OS 1.0 Ready");
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
    // Serial.println(F("NON BLOCKING"));
  }
}
