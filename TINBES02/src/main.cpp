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
        return true;
      }
    }
    // Complete buffer
    Serial.println(buf);
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
    for (int i = 0; i < sizeof(command) / sizeof(commandType); i++)
    {
      if (strcmp(command[i].name, buf) == 0)
      {
        Serial.println(F("Executing command"));
        Serial.print(strcmp(command[i].name, buf));
        command[i].func();
      }
    }
  }
  else

  {
    Serial.println(F("Command not found"));
  }
}
