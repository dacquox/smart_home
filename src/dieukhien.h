#include <Arduino.h>

enum SmartCommand {
    CMD_NONE,

    CMD_AC_ON,
    CMD_AC_OFF,
    CMD_AC_ON2,
    CMD_AC_OFF2,

    CMD_LIGHT_ON,
    CMD_LIGHT_OFF,
    CMD_LIGHT_BRIGHTNESS,

    CMD_TEMP_GET_1,
    CMD_TEMP_GET_2,

    CMD_AC1_TEMP_UP,
    CMD_AC1_TEMP_DOWN,
    CMD_AC2_TEMP_UP,
    CMD_AC2_TEMP_DOWN,

    CMD_DOOR_OPEN,
    CMD_DOOR_CLOSE
};

struct CommandResult {
    SmartCommand cmd;
    int value;
};

String dieukhienUnknownAnswer();

CommandResult dieukhienDetectCommand(const String &input);

String dieukhienAnswerForCommand(CommandResult result);

String dieukhienCommandName(CommandResult result);

void dieukhienHandleCommand(CommandResult result);
