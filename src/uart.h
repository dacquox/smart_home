#include <Arduino.h>

void uartLinkBegin(int rxPin, int txPin, uint32_t baud);
void clearUartReplyBuffer() ;
void uartSendLine(const String &line);
void uartSendCommand(const String &cmdName);
String uartReadLine();
