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
    {"TEST2", 15, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"TEST", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"TEST3", 50, 0},
    {"", 0, 0},
    {"", 0, 0}};

void writeFATEntry()
{
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
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
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    EEPROM.get(i * sizeof(FATtype), FAT[i]);
    // Serial.print(FAT[i].name);
    // Serial.print(" ");
    // Serial.print(FAT[i].startingPoint);
    // Serial.print(" ");
    // Serial.println(FAT[i].lengthOfFile);
  }
}
bool fileNameExists()
{
  for (int i = 0; i < noOfFiles; i++)
  {
    if (strcmp(FAT[i].name, buf) == 0)
    {
      return true;
    }
  }
  return false;
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
int searchFreeFATEntry()
{
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    if (strcmp(FAT[i].name, "") == 0)
    {
      Serial.println(F("Found free FAT entry"));
      return i;
    }
  }
  return -1;
}
void printFATTable()
{
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    Serial.print(FAT[i].name);
    Serial.print(" ");
    Serial.print(FAT[i].startingPoint);
    Serial.print(" ");
    Serial.println(FAT[i].lengthOfFile);
  }
}
void retrieve()
{
  writeFATEntry();
}

void store()
{
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

  if (searchFreeFATEntry() == -1)
  {
    Serial.println("No free space in FAT");
    return;
  }
  readFATEntry();
  sortFATTable();
  printFATTable();
  if (fileNameExists())
  {
    Serial.println("File name already exists");
    return;
  }
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
        {"", NULL}};

void proccesInput()
{
  for (byte i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    if (strcmp(command[i].name, buf) == 0)
    {
      Serial.println(F("Executing command"));
      command[i].func();
      return;
    }
  }
  Serial.println(F("Given command not found choose one of following: "));
  for (byte i = 0; i < sizeof(command) / sizeof(commandType); i++)
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
  FATtype test = {"test", 0, 0};
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
