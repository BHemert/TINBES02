#include <Arduino.h>

const int BUFFER_SIZE = 12;
char buf[BUFFER_SIZE];

void store()
{
  Serial.println("store");
}

typedef struct
{
  char name[BUFFER_SIZE];
  void (*func)();

} commandType;
static commandType command[] =
    {
        {"store", &store},
};

void setup()
{
  Serial.begin(9600);
  command[0].func();
  Serial.println(F("Arduino OS 1.0 Ready"));
}

void loop()
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
      }
    }
    // Complete buffer
    Serial.println(buf);
  }
}
