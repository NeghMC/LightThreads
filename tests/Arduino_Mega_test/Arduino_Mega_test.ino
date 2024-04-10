// library to handle system tick (required for delay functionality)
#include "TimerOne.h"

// library include and required functions implementation
extern "C" {
  #include "LightThreads.h"

  void lt_criticalEnter()
  {
    noInterrupts();
  }

  void lt_criticalExit()
  {
    interrupts();
  }

  void lt_idleCallback()
  {
  }
}

// declaration of thread functions
  // first thread (toogles lead every 444 ms)
  lt_context_t blinkThread;
  void blinkThreadFunction(void *arg);
  // second thread (echoes whatever was send to the serial monitor)
  lt_context_t serialThread;
  void serialReadThreadFunction(void *arg);
  void serialWriteThreadFunction(void *arg);

// ----- definition of thread functions ------------

void blinkThreadFunction(void *arg)
{
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  // repeat that function in 444 ms
  lt_delay(&blinkThread, blinkThreadFunction, NULL, 444);
}

void serialReadThreadFunction(void *arg)
{
  if(Serial.available() > 0)
  {
    // read and schedule write
    char c = Serial.read();
    lt_schedule(&serialThread, serialWriteThreadFunction, (void*)c);
  }
  else
  {
    // check again later
    lt_delay(&serialThread, serialReadThreadFunction, arg, 100);
  }
}

void serialWriteThreadFunction(void *arg)
{
  if(Serial.availableForWrite() > 0)
  {
    // write and schedule read
    Serial.write((char)arg);
    lt_schedule(&serialThread, serialReadThreadFunction, arg);
  }
  else
  {
    // check again later
    lt_delay(&serialThread, serialWriteThreadFunction, arg, 100);
  }
}


// --------- main loop -----------

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Start");
  
  // initialize timer to trigger 'lt_tick()' every 1000 us (1 ms)
  Timer1.initialize(1000);
  Timer1.attachInterrupt(lt_tick);

  // schedule all the threads (they will start immedietly)
  lt_schedule(&blinkThread, blinkThreadFunction, NULL);
  lt_schedule(&serialThread, serialReadThreadFunction, NULL);
}

void loop() {
  // handle the scheduler
  lt_runNext();
}
