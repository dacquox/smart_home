#include "dieukhien.h"
#include "uart.h"

// Kiem tra chuoi co tu khoa khong
static bool hasText(const String &s, const char *key) {
    return s.indexOf(key) >= 0;
}

// Khac cac lenh cho thi tra loi nhu nay
String dieukhienUnknownAnswer() {
    return "Xin lỗi, tôi không thể trả lời vấn đề này.";
}

//static int extractPercentValue(String s) {
//    s.toLowerCase();
//    for (int i = 0; i < s.length(); i++) {
//        if (isDigit(s[i])) {
 //           int value = s.substring(i).toInt();
    //        if (value < 0) value = 0;
    //        if (value > 100) value = 100;
     //       return value;
    //    }
//}

 //   if (hasText(s, "khong")) return 0;
 //   if (hasText(s, "mot tram")) return 100;
 //   if (hasText(s, "chin muoi")) return 90;
  //  if (hasText(s, "tam muoi")) return 80;
  //  if (hasText(s, "bay muoi")) return 70;
   // if (hasText(s, "sau muoi")) return 60;
   // if (hasText(s, "nam muoi")) return 50;
   // if (hasText(s, "bon muoi")) return 40;
   // if (hasText(s, "ba muoi")) return 30;
   // if (hasText(s, "hai muoi")) return 20;
   // if (hasText(s, "muoi")) return 10;

   // return -1;
//}

// Lay so trong cau
// Vi du: "giam nhiet do 1 xuong 3 do" -> lay so 3 sau chu xuong
static int extractNumberAfterKeyword(String s, const char *key1) {
    s.toLowerCase();
    int pos = s.indexOf(key1);
    if (pos < 0) {
        return 1;
    }
    String after = s.substring(pos);
    for (int i = 0; i < after.length(); i++) {
        if (isDigit(after[i])) {
            int value = after.substring(i).toInt();
            if (value > 0) {
                return value;
            }
        }
    }
    if (hasText(after, "mot")) return 1;
    if (hasText(after, "hai")) return 2;
    if (hasText(after, "ba")) return 3;
    if (hasText(after, "bon")) return 4;
    if (hasText(after, "nam")) return 5;
    if (hasText(after, "sau")) return 6;
    if (hasText(after, "bay")) return 7;
    if (hasText(after, "tam")) return 8;
    if (hasText(after, "chin")) return 9;
    if (hasText(after, "muoi")) return 10;
    return 1;
}


// Lay so do muon tang/giam
static int extractTempValue(String s) {
    int y = 1;
    if (hasText(s, "len")) {
        y = extractNumberAfterKeyword(s, "len");
    } else if (hasText(s, "xuong")) {
        y = extractNumberAfterKeyword(s, "xuong");
    }
    if (y <= 0) y = 1;
    return y;
}

// Tao ket qua lenh
static CommandResult makeResult(SmartCommand cmd, int value = 1) {
    CommandResult result;
    result.cmd = cmd;
    result.value = value;
    return result;
}

// Phan tich xem co lenh khong
CommandResult dieukhienDetectCommand(const String &input) {
    String s = input;
    s.toLowerCase();

    // ================= ĐIỀU HOÀ SỐ 2 - BẬT =================
    if (
        hasText(s, "bat dieu hoa so hai") ||
        hasText(s, "bat dieu hoa so 2") ||
        hasText(s, "mo dieu hoa so hai") ||
        hasText(s, "mo dieu hoa so 2") ||

        hasText(s, "bật điều hoà số hai") ||
        hasText(s, "bật điều hoà số 2") ||
        hasText(s, "mở điều hoà số hai") ||
        hasText(s, "mở điều hoà số 2")
    ) {
        return makeResult(CMD_AC_ON2);
    }

    // ================= ĐIỀU HOÀ SỐ 2 - TẮT =================
    if (
        hasText(s, "tat dieu hoa so hai") ||
        hasText(s, "tat dieu hoa so 2") ||
        hasText(s, "tat may lanh so hai") ||
        hasText(s, "tat may lanh so 2") ||

        hasText(s, "tat dieu hoa hai") ||
        hasText(s, "tat dieu hoa 2") ||
        hasText(s, "tat may lanh hai") ||
        hasText(s, "tat may lanh 2") ||

        hasText(s, "tắt điều hoà số hai") ||
        hasText(s, "tắt điều hoà số 2") ||
        hasText(s, "tắt máy lạnh số hai") ||
        hasText(s, "tắt máy lạnh số 2") ||

        hasText(s, "tắt điều hoà hai") ||
        hasText(s, "tắt điều hoà 2") ||
        hasText(s, "tắt máy lạnh hai") ||
        hasText(s, "tắt máy lạnh 2")
    ) {
        return makeResult(CMD_AC_OFF2);
    }

    // ================= ĐIỀU HOÀ SỐ 1 - BẬT =================
    if (
        hasText(s, "bat dieu hoa so mot") ||
        hasText(s, "bat dieu hoa so 1") ||
        hasText(s, "mo dieu hoa so mot") ||
        hasText(s, "mo dieu hoa so 1") ||

        hasText(s, "bật điều hoà số một") ||
        hasText(s, "bật điều hoà số 1") ||
        hasText(s, "mở điều hoà số một") ||
        hasText(s, "mở điều hoà số 1")
    ) {
        return makeResult(CMD_AC_ON);
    }

    // ================= ĐIỀU HOÀ SỐ 1 - TẮT =================
    if (
        hasText(s, "tat dieu hoa so mot") ||
        hasText(s, "tat dieu hoa so 1") ||
        hasText(s, "tat may lanh so mot") ||
        hasText(s, "tat may lanh so 1") ||

        hasText(s, "tắt điều hoà số một") ||
        hasText(s, "tắt điều hoà số 1") ||
        hasText(s, "tắt máy lạnh số một") ||
        hasText(s, "tắt máy lạnh số 1")
    ) {
        return makeResult(CMD_AC_OFF);
    }

    // ================= ĐÈN - BẬT =================
    if (
        hasText(s, "bat den") ||
        hasText(s, "mo den") ||
        hasText(s, "bat bong den") ||

        hasText(s, "bật đèn") ||
        hasText(s, "mở đèn") ||
        hasText(s, "bật bóng đèn")
    ) {
        return makeResult(CMD_LIGHT_ON);
    }

    // ================= ĐÈN - TẮT =================
    if (
        hasText(s, "tat den") ||
        hasText(s, "tat bong den") ||

        hasText(s, "tắt đèn") ||
        hasText(s, "tắt bóng đèn")
    ) {
        return makeResult(CMD_LIGHT_OFF);
    }

    // ================= NHIỆT ĐỘ ĐIỀU HOÀ 1 =================
    if (
        hasText(s, "nhiet do cua dieu hoa so mot hien tai") ||
        hasText(s, "nhiet do cua dieu hoa so 1 hien tai") ||
        hasText(s, "nhiet do dieu hoa so mot hien tai") ||
        hasText(s, "nhiet do dieu hoa so 1 hien tai") ||
        hasText(s, "dieu hoa so mot hien tai bao nhieu do") ||
        hasText(s, "dieu hoa so 1 hien tai bao nhieu do") ||
        hasText(s, "dieu hoa so mot bao nhieu do") ||
        hasText(s, "dieu hoa so 1 bao nhieu do") ||

        hasText(s, "nhiệt độ của điều hoà số một hiện tại") ||
        hasText(s, "nhiệt độ của điều hoà số 1 hiện tại") ||
        hasText(s, "nhiệt độ điều hoà số một hiện tại") ||
        hasText(s, "nhiệt độ điều hoà số 1 hiện tại") ||
        hasText(s, "điều hoà số một hiện tại bao nhiêu độ") ||
        hasText(s, "điều hoà số 1 hiện tại bao nhiêu độ") ||
        hasText(s, "điều hoà số một bao nhiêu độ") ||
        hasText(s, "điều hoà số 1 bao nhiêu độ")
    ) {
        return makeResult(CMD_TEMP_GET_1);
    }

    // ================= NHIỆT ĐỘ ĐIỀU HOÀ 2 =================
    if (
        hasText(s, "nhiet do cua dieu hoa so hai hien tai") ||
        hasText(s, "nhiet do cua dieu hoa so 2 hien tai") ||
        hasText(s, "nhiet do dieu hoa so hai hien tai") ||
        hasText(s, "nhiet do dieu hoa so 2 hien tai") ||
        hasText(s, "dieu hoa so hai hien tai bao nhieu do") ||
        hasText(s, "dieu hoa so 2 hien tai bao nhieu do") ||
        hasText(s, "dieu hoa so hai bao nhieu do") ||
        hasText(s, "dieu hoa so 2 bao nhieu do") ||

        hasText(s, "nhiệt độ của điều hoà số hai hiện tại") ||
        hasText(s, "nhiệt độ của điều hoà số 2 hiện tại") ||
        hasText(s, "nhiệt độ điều hoà số hai hiện tại") ||
        hasText(s, "nhiệt độ điều hoà số 2 hiện tại") ||
        hasText(s, "điều hoà số hai hiện tại bao nhiêu độ") ||
        hasText(s, "điều hoà số 2 hiện tại bao nhiêu độ") ||
        hasText(s, "điều hoà số hai bao nhiêu độ") ||
        hasText(s, "điều hoà số 2 bao nhiêu độ")
    ) {
        return makeResult(CMD_TEMP_GET_2);
    }

    // ================= TĂNG NHIỆT ĐỘ ĐIỀU HOÀ 1 =================
    if (
        hasText(s, "tang nhiet do 1") ||
        hasText(s, "tang nhiet do so 1") ||
        hasText(s, "tang nhiet do dieu hoa 1") ||
        hasText(s, "tang nhiet do dieu hoa so 1") ||

        hasText(s, "tăng nhiệt độ 1") ||
        hasText(s, "tăng nhiệt độ số 1") ||
        hasText(s, "tăng nhiệt độ điều hoà 1") ||
        hasText(s, "tăng nhiệt độ điều hoà số 1")
    ) {
        return makeResult(CMD_AC1_TEMP_UP, extractTempValue(s));
    }

    // ================= GIẢM NHIỆT ĐỘ ĐIỀU HOÀ 1 =================
    if (
        hasText(s, "giam nhiet do 1") ||
        hasText(s, "giam nhiet do so 1") ||
        hasText(s, "giam nhiet do dieu hoa 1") ||
        hasText(s, "giam nhiet do dieu hoa so 1") ||

        hasText(s, "giảm nhiệt độ 1") ||
        hasText(s, "giảm nhiệt độ số 1") ||
        hasText(s, "giảm nhiệt độ điều hoà 1") ||
        hasText(s, "giảm nhiệt độ điều hoà số 1")
    ) {
        return makeResult(CMD_AC1_TEMP_DOWN, extractTempValue(s));
    }

    // ================= TĂNG NHIỆT ĐỘ ĐIỀU HOÀ 2 =================
    if (
        hasText(s, "tang nhiet do 2") ||
        hasText(s, "tang nhiet do so 2") ||
        hasText(s, "tang nhiet do dieu hoa 2") ||
        hasText(s, "tang nhiet do dieu hoa so 2") ||

        hasText(s, "tăng nhiệt độ 2") ||
        hasText(s, "tăng nhiệt độ số 2") ||
        hasText(s, "tăng nhiệt độ điều hoà 2") ||
        hasText(s, "tăng nhiệt độ điều hoà số 2")
    ) {
        return makeResult(CMD_AC2_TEMP_UP, extractTempValue(s));
    }

    // ================= GIẢM NHIỆT ĐỘ ĐIỀU HOÀ 2 =================
    if (
        hasText(s, "giam nhiet do 2") ||
        hasText(s, "giam nhiet do so 2") ||
        hasText(s, "giam nhiet do dieu hoa 2") ||
        hasText(s, "giam nhiet do dieu hoa so 2") ||

        hasText(s, "giảm nhiệt độ 2") ||
        hasText(s, "giảm nhiệt độ số 2") ||
        hasText(s, "giảm nhiệt độ điều hoà 2") ||
        hasText(s, "giảm nhiệt độ điều hoà số 2")
    ) {
        return makeResult(CMD_AC2_TEMP_DOWN, extractTempValue(s));
    }
    // ================= MỞ CỬA =================
    if (
        hasText(s, "tuan oi mo cua") ||
        hasText(s, "bong oi mo cua") ||
        hasText(s, "mo cua") ||

        hasText(s, "tuấn ơi mở cửa") ||
        hasText(s, "bống ơi mở cửa") ||
        hasText(s, "mở cửa")
    ) {
        return makeResult(CMD_DOOR_OPEN);
    }

    // ================= ĐÓNG CỬA =================
    if (
        hasText(s, "vung oi dong lai") ||
        hasText(s, "vung oi dong cua lai") ||
        hasText(s, "dong cua") ||

        hasText(s, "vừng ơi đóng lại") ||
        hasText(s, "vừng ơi đóng cửa lại") ||
        hasText(s, "đóng cửa")
    ) {
        return makeResult(CMD_DOOR_CLOSE);
    }

    return makeResult(CMD_NONE);
}

// Tra loi cac lenh
String dieukhienAnswerForCommand(CommandResult result) {
    switch (result.cmd) {
        case CMD_AC_ON:
            return "Vâng, tôi sẽ bật điều hòa số 1 cho bạn đây.";
        case CMD_AC_OFF:
            return "Vâng, tôi sẽ tắt điều hòa số 1 cho bạn đây.";
        case CMD_AC_ON2:
            return "Vâng, tôi sẽ bật điều hòa số 2 cho bạn đây.";
        case CMD_AC_OFF2:
            return "Vâng, tôi sẽ tắt điều hòa số 2 cho bạn đây.";
        case CMD_LIGHT_ON:
            return "Vâng, tôi sẽ bật đèn cho bạn đây.";
        case CMD_LIGHT_OFF:
            return "Vâng, tôi sẽ tắt đèn cho bạn đây.";
   //     case CMD_LIGHT_BRIGHTNESS:
       //     return "Vâng, tôi sẽ chỉnh độ sáng của đèn thành " + String(result.value) + " phần trăm.";
        case CMD_TEMP_GET_1:
            return "Vâng, tôi sẽ kiểm tra nhiệt độ điều hòa số 1 hiện tại.";
        case CMD_TEMP_GET_2:
            return "Vâng, tôi sẽ kiểm tra nhiệt độ điều hòa số 2 hiện tại.";
        case CMD_AC1_TEMP_DOWN:
            return "Vâng, tôi sẽ giảm nhiệt độ điều hòa số 1 xuống " + String(result.value) + " độ.";
        case CMD_AC1_TEMP_UP:
            return "Vâng, tôi sẽ tăng nhiệt độ điều hòa số 1 lên " + String(result.value) + " độ.";
        case CMD_AC2_TEMP_DOWN:
            return "Vâng, tôi sẽ giảm nhiệt độ điều hòa số 2 xuống " + String(result.value) + " độ.";
        case CMD_AC2_TEMP_UP:
            return "Vâng, tôi sẽ tăng nhiệt độ điều hòa số 2 lên " + String(result.value) + " độ.";
        case CMD_DOOR_OPEN:
            return "Vâng, tôi sẽ mở cửa cho bạn đây.";
        case CMD_DOOR_CLOSE:
            return "Vâng, tôi sẽ đóng cửa cho bạn đây.";
        default:
            return dieukhienUnknownAnswer();
    }
}

// Doi lenh thanh chuoi gui qua UART
String dieukhienCommandName(CommandResult result) {
    switch (result.cmd) {
        case CMD_AC_ON:
            return "AC1_ON";
        case CMD_AC_OFF:
            return "AC1_OFF";
        case CMD_AC_ON2:
            return "AC2_ON";
        case CMD_AC_OFF2:
            return "AC2_OFF";
        case CMD_LIGHT_ON:
            return "LIGHT_ON";
        case CMD_LIGHT_OFF:
            return "LIGHT_OFF";
       // case CMD_LIGHT_BRIGHTNESS:
          //  return "LIGHT_BRIGHTNESS:" + String(result.value);
        case CMD_TEMP_GET_1:
            return "AC1_TEMP_GET";
          case CMD_TEMP_GET_2:
            return "AC2_TEMP_GET";
        case CMD_AC1_TEMP_DOWN:
            return "AC1_TEMP_DOWN:" + String(result.value);
        case CMD_AC1_TEMP_UP:
            return "AC1_TEMP_UP:" + String(result.value);
        case CMD_AC2_TEMP_DOWN:
            return "AC2_TEMP_DOWN:" + String(result.value);
        case CMD_AC2_TEMP_UP:
            return "AC2_TEMP_UP:" + String(result.value);
        case CMD_DOOR_OPEN:
            return "DOOR_OPEN";
        case CMD_DOOR_CLOSE:
            return "DOOR_CLOSE";
        default:
            return "";
    }
}

// Xu ly lenh
void dieukhienHandleCommand(CommandResult result) {
    if (result.cmd == CMD_NONE) return;
    String frame = dieukhienCommandName(result);
    if (frame.length() > 0) {
        uartSendCommand(frame);
    }
}