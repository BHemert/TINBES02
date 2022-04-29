#include <Arduino.h>
#include <EEPROM.h>

const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

EERef noOfFiles = EEPROM[160];

typedef struct
{ // struct for the FAT table
  char name[BUFFER_SIZE];
  int startingPoint;
  int lengthOfFile;
} FATtype;

FATtype FAT[10] = {
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0},
    {"", 0, 0}};

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
  if (Serial.available() > 0)
  {
    // read the length of incoming bytes:
    int bufferLen = Serial.readBytes(buf, BUFFER_SIZE);
    Serial.print("I received: ");
    for (int i = 0; i < bufferLen; i++)
    {
      if (buf[i] == ' ' || buf[i] == '\n')
      {
        // return true en ga door naar functie
        buf[i] = '\0';
        Serial.print(buf);
        Serial.print(F("\n"));
        return true;
      }
    }
    // Complete buffer
  }
  return false;
}
int searchFreeFATPlace()
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

byte sortFAT(){ // Sort the FAT table
  byte i, j;
  for (i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    for (j = 0; j < sizeof(FAT) / sizeof(FATtype); j++)
    {
      if (strcmp(FAT[i].name, FAT[j].name) < 0)
      {
        FATtype temp = FAT[i];
        FAT[i] = FAT[j];
        FAT[j] = temp;
      }
    }
  }
  return 0;

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

  if (searchFreeFATPlace() == -1)
  {
    Serial.println("No free space in FAT");
    return;
  }
  if (fileNameExists())
  {
    Serial.println("File name already exists");
    return;
  }
}
void retrieve()
{
  Serial.println("Hij is in retrieve");
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
    else
    {
      Serial.println(F("Given command not found choose one of following: "));
      for (byte i = 0; i < sizeof(command) / sizeof(commandType); i++)
      {
        Serial.print(command[i].name);
        Serial.print(" ");
      }
      return; // return to main loop
    }
  }
}

void writeFATEntry()
{
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    EEPROM.put(i * sizeof(FATtype), FAT[i]);
  }
}

void readFATEntry()
{
  for (byte i = 0; i < sizeof(FAT) / sizeof(FATtype); i++)
  {
    EEPROM.get(i * sizeof(FATtype), FAT[i]);
  }
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
  }
  else

  {
    // Serial.println(F("NON BLOCKING"));
  }
}
