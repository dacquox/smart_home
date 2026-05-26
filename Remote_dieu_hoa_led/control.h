#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

#define AC_TEMP_MIN 20
#define AC_TEMP_MAX 28
#define AC_TEMP_DEFAULT 25



// ===================== RAW data điều hòa từng nhiệt độ =====================
extern uint16_t rawData_ac_on[];
extern uint16_t rawData_ac_off[];
extern uint16_t rawData_ac_temp_20[];
extern uint16_t rawData_ac_temp_21[];
extern uint16_t rawData_ac_temp_22[];
extern uint16_t rawData_ac_temp_23[];
extern uint16_t rawData_ac_temp_24[];
extern uint16_t rawData_ac_temp_25[];
extern uint16_t rawData_ac_temp_26[];
extern uint16_t rawData_ac_temp_27[];
extern uint16_t rawData_ac_temp_28[];

// ===================== Điều hòa =====================
void On_AC();
void Off_AC();
void AC_Temp_Up();           // Tăng 1 độ
void AC_Temp_Down();         // Giảm 1 độ
void AC_Set_Temp(uint8_t t); // Đặt thẳng nhiệt độ (20-28)
uint8_t AC_Get_Temp();       // Lấy nhiệt độ hiện tại

#endif