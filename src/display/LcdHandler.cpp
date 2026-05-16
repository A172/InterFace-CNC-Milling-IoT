// #include "LcdHandler.h"

// /*
//    Constructor:
//    clock = LCD_E
//    data  = LCD_RS
//    cs    = LCD_RW
//    reset = LCD_RST
// */

// void LcdHandler::begin() {

//     lcd = new U8G2_ST7920_128X64_F_SW_SPI(
//         U8G2_R0,
//         LCD_E,
//         LCD_RS,
//         LCD_RW,
//         LCD_RST
//     );

//     lcd->begin();
//     lcd->clearBuffer();
//     lcd->sendBuffer();
// }

// void LcdHandler::clear() {
//     lcd->clearBuffer();
//     lcd->sendBuffer();
// }

// void LcdHandler::print(uint8_t x, uint8_t y, const char *text) {
//     lcd->clearBuffer();
//     lcd->setFont(u8g2_font_ncenB08_tr);
//     lcd->drawStr(x, y, text);
//     lcd->sendBuffer();
// }
