#include <Arduino.h>

const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

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
}
void store()
{
  Serial.println("Hij is in store");
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

void setup()
{
  input_routine() == false;
  Serial.begin(9600);
  // command[0].func();
  Serial.println(F("Arduino OS 1.0 Ready"));
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
