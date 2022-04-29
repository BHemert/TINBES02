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

void store()
{
  input_routine();
  
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
  for (int i = 0; i < sizeof(command) / sizeof(commandType); i++)
  {
    if (strcmp(command[i].name, buf) == 0)
    {
      Serial.println(F("Executing command"));
      command[i].func();
      return;
    }
    else
    {
      Serial.println("Given command not found choose one of following: ");
      for (int i = 0; i < sizeof(command) / sizeof(commandType); i++)
      {
        Serial.print(command[i].name);
        Serial.print(" ");
      }
      return; // return to main loop
    }
  }
}

void writeFATEntry(int address, RfObject value)
{
  EEPROM.put(address, value);
}

void readFATEntry(int address, RfObject value)
{
  EEPROM.get(address, value);
  Serial.print("Value at address ");
  Serial.print(address);
  Serial.print(" is ");
  Serial.print(value.beginPositie);
  Serial.print(value.bestandNaam);
  Serial.print(value.lengte);
}

void searchFAT(String naam)
{
  for (int i = 0; i < EEPROM.length(); i++)
  {
    if (EEPROM.read(i) != 0)

      Serial.print("Found entry at address ");
    Serial.print(i);
    Serial.print(" with value ");
    Serial.println(EEPROM.read(i));
  }
}

void setup()
{
  Serial.begin(9600);
  FATtype test = {"test", 0, 0};
  writeFATEntry(1, object1);
  writeFATEntry(100, object2);
  readFATEntry(1, object1);
  readFATEntry(1, object2);
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
