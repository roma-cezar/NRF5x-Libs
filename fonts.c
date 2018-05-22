/**
 * original author:  Tilen Majerle<tilen@majerle.eu>
 * modification for STM32f10x: Alexander Lutsai<s.lyra@ya.ru>
   ----------------------------------------------------------------------
   	Copyright (C) Alexander Lutsai, 2016
    Copyright (C) Tilen Majerle, 2015
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
 */
#include "fonts.h"

const uint8_t Font7x10[] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x87, 0xe3, 0xe3, 0x70, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x15, 0x45, 0x45, 0x41, 0x51, 0x54, 0x00, 0x00, 0x00,  //
0x00, 0x89, 0xc4, 0x23, 0x91, 0xf0, 0x20, 0x10, 0x08, 0x00,  //
0x00, 0x87, 0xc0, 0x20, 0xd0, 0x21, 0x30, 0x08, 0x04, 0x00,  //
0x00, 0x8e, 0x40, 0xc0, 0xe1, 0x90, 0x38, 0x24, 0x12, 0x00,  //
0x00, 0x81, 0x40, 0xe0, 0xe1, 0x11, 0x38, 0x04, 0x02, 0x00,  //
0x00, 0x0e, 0x85, 0xc2, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x02, 0xe1, 0x43, 0x20, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x89, 0xc5, 0xa2, 0x91, 0x10, 0x08, 0x04, 0x1e, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x08, 0x04, 0x02, 0xf1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0xf0, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x00, 0x00, 0x00, 0x80, 0x43, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x08, 0x04, 0x02, 0x81, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x08, 0x04, 0x02, 0xf1, 0x43, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0xf0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x00,  //
0x08, 0x04, 0x02, 0x81, 0x43, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x08, 0x04, 0x02, 0xf1, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x08, 0x04, 0x02, 0xf1, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0xf0, 0x43, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x08, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00,  //
0x00, 0x10, 0x84, 0x20, 0x20, 0x40, 0x40, 0x3e, 0x00, 0x00,  //
0x00, 0x01, 0x01, 0x02, 0x82, 0x10, 0x04, 0x3e, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x47, 0xa1, 0x50, 0x28, 0x00, 0x00, 0x00,  //
0x00, 0x10, 0xc4, 0x87, 0xf0, 0x11, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x0c, 0x89, 0xe0, 0x21, 0x70, 0x6c, 0x04, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x02, 0x81, 0x40, 0x00, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x0a, 0x85, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x0a, 0xc5, 0x47, 0xf1, 0x51, 0x28, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x47, 0xc1, 0x41, 0x71, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x92, 0x8a, 0x82, 0xa0, 0xa8, 0x24, 0x00, 0x00, 0x00,  //
0x00, 0x82, 0x42, 0x41, 0x50, 0x49, 0x58, 0x00, 0x00, 0x00,  //
0x00, 0x0c, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x08, 0x82, 0x40, 0x20, 0x20, 0x20, 0x00, 0x00, 0x00,  //
0x00, 0x02, 0x02, 0x02, 0x81, 0x20, 0x08, 0x00, 0x00, 0x00,  //
0x00, 0x80, 0x88, 0xe2, 0xa3, 0x88, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x02, 0xe1, 0x43, 0x20, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x10, 0x04, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0xe0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x38, 0x08, 0x00, 0x00,  //
0x00, 0x10, 0x08, 0x82, 0x20, 0x08, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x45, 0x24, 0x12, 0x51, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x43, 0x81, 0x40, 0x20, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x08, 0x84, 0x21, 0x08, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x1f, 0x08, 0x82, 0x01, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x08, 0x86, 0x22, 0xf1, 0x41, 0x20, 0x00, 0x00, 0x00,  //
0x00, 0x9f, 0x40, 0x63, 0x02, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x0c, 0x41, 0xa0, 0x31, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x1f, 0x08, 0x02, 0x41, 0x10, 0x08, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0xc4, 0x11, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0xc6, 0x02, 0x41, 0x18, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x82, 0x83, 0x00, 0x20, 0x38, 0x08, 0x00, 0x00,  //
0x00, 0x00, 0x82, 0x83, 0x00, 0x60, 0x10, 0x04, 0x00, 0x00,  //
0x00, 0x10, 0x04, 0x41, 0x40, 0x40, 0x40, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x07, 0xf0, 0x01, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x01, 0x01, 0x01, 0x41, 0x10, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x08, 0x82, 0x40, 0x00, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0xa6, 0xd2, 0x08, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x45, 0x24, 0xf2, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x0f, 0x89, 0xc4, 0x21, 0x91, 0x3c, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0x20, 0x10, 0x88, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x0f, 0x89, 0x44, 0x22, 0x91, 0x3c, 0x00, 0x00, 0x00,  //
0x00, 0x9f, 0x40, 0xe0, 0x11, 0x08, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x9f, 0x40, 0xe0, 0x11, 0x08, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0x20, 0x90, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x48, 0xe4, 0x13, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x0e, 0x02, 0x81, 0x40, 0x20, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x1c, 0x08, 0x04, 0x02, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x44, 0x61, 0x50, 0x48, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x81, 0x40, 0x20, 0x10, 0x08, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0xc8, 0xa6, 0x12, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0xc8, 0xa4, 0x92, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0x24, 0x12, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x8f, 0x48, 0xe4, 0x11, 0x08, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0x24, 0x12, 0xa9, 0x38, 0x20, 0x00, 0x00,  //
0x00, 0x8f, 0x48, 0xe4, 0x51, 0x48, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x48, 0xc0, 0x01, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x1f, 0x02, 0x81, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x48, 0x24, 0x12, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x48, 0x44, 0xa1, 0x50, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x48, 0xa4, 0x52, 0xd9, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x88, 0x82, 0xa0, 0x88, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x91, 0x88, 0x82, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x1f, 0x08, 0x82, 0x20, 0x08, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x0e, 0x81, 0x40, 0x20, 0x10, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x81, 0x80, 0x80, 0x80, 0x80, 0x40, 0x00, 0x00, 0x00,  //
0x00, 0x0e, 0x04, 0x02, 0x81, 0x40, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x45, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00,  //
0x00, 0x06, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x03, 0xe2, 0x89, 0x78, 0x00, 0x00, 0x00,  //
0x00, 0x81, 0x40, 0x63, 0x12, 0x99, 0x34, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x23, 0x12, 0x88, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x10, 0x88, 0x25, 0x13, 0xc9, 0x58, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x23, 0xf2, 0x09, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x0c, 0x89, 0xe0, 0x21, 0x10, 0x08, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x25, 0x61, 0x08, 0x38, 0x22, 0x0e, 0x00,  //
0x00, 0x81, 0x40, 0x63, 0x12, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x80, 0x81, 0x40, 0x20, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x08, 0x00, 0x03, 0x81, 0x40, 0x24, 0x12, 0x06, 0x00,  //
0x00, 0x81, 0x40, 0x24, 0x71, 0x48, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x06, 0x02, 0x81, 0x40, 0x20, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0xa2, 0x52, 0xa9, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x63, 0x12, 0x89, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x23, 0x12, 0x89, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x63, 0x32, 0x69, 0x04, 0x02, 0x01, 0x00,  //
0x00, 0x00, 0x80, 0x25, 0x93, 0xb1, 0x40, 0x20, 0x10, 0x00,  //
0x00, 0x00, 0x40, 0x63, 0x12, 0x08, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x23, 0xe0, 0x80, 0x3c, 0x00, 0x00, 0x00,  //
0x00, 0x02, 0xc1, 0x43, 0x20, 0x90, 0x30, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x24, 0x12, 0xc9, 0x58, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x24, 0xa2, 0x50, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x24, 0x52, 0xa9, 0x28, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x44, 0x41, 0x50, 0x44, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x24, 0x92, 0xb1, 0x40, 0x22, 0x0e, 0x00,  //
0x00, 0x00, 0xc0, 0x07, 0x41, 0x10, 0x7c, 0x00, 0x00, 0x00,  //
0x00, 0x18, 0x02, 0xc2, 0x80, 0x20, 0x60, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x02, 0x81, 0x40, 0x20, 0x10, 0x00, 0x00, 0x00,  //
0x00, 0x03, 0x82, 0x80, 0x21, 0x20, 0x0c, 0x00, 0x00, 0x00,  //
0x00, 0x92, 0x4a, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x14, 0x80, 0x4f, 0xe0, 0x11, 0x08, 0x7c, 0x00, 0x00, 0x00,  //
0x1c, 0x51, 0xb2, 0x5a, 0xcc, 0x8a, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x04, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x0a, 0x80, 0x23, 0xf2, 0x09, 0x38, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //
0x00, 0x82, 0x22, 0x12, 0xf9, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x47, 0x20, 0xf0, 0x88, 0x44, 0x1e, 0x00, 0x00, 0x00,  //
0x80, 0x47, 0x24, 0xf2, 0x88, 0x44, 0x1e, 0x00, 0x00, 0x00,  //
0x80, 0x4f, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00,  //
0x00, 0x0c, 0x85, 0x22, 0x91, 0x48, 0x3e, 0x11, 0x00, 0x00,  //
0x80, 0x4f, 0x20, 0xf0, 0x08, 0x04, 0x3e, 0x00, 0x00, 0x00,  //
0x80, 0x4a, 0xa5, 0xe2, 0xa8, 0x54, 0x2a, 0x00, 0x00, 0x00,  //
0x00, 0x47, 0x04, 0xc2, 0x80, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x24, 0x53, 0x99, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x0a, 0x42, 0x24, 0x53, 0x99, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0xa2, 0x30, 0x28, 0x24, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x8e, 0x44, 0x22, 0x91, 0x48, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x64, 0x53, 0x89, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x24, 0xf2, 0x89, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x47, 0x24, 0x12, 0x89, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x80, 0x4f, 0x24, 0x12, 0x89, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x47, 0x24, 0xf2, 0x08, 0x04, 0x02, 0x00, 0x00, 0x00,  //
0x00, 0x47, 0x24, 0x10, 0x08, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x80, 0x0f, 0x81, 0x40, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x24, 0xa2, 0x20, 0x08, 0x02, 0x00, 0x00, 0x00,  //
0x00, 0x82, 0xa3, 0x52, 0xa9, 0x38, 0x08, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x44, 0x41, 0x50, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x80, 0x44, 0x22, 0x91, 0x48, 0x24, 0x3e, 0x10, 0x08, 0x00,  //
0x80, 0x48, 0x24, 0xe2, 0x81, 0x40, 0x20, 0x00, 0x00, 0x00,  //
0x80, 0x4a, 0xa5, 0x52, 0xa9, 0x54, 0x3e, 0x00, 0x00, 0x00,  //
0x80, 0x4a, 0xa5, 0x52, 0xa9, 0x54, 0x3e, 0x10, 0x08, 0x00,  //
0x80, 0x81, 0x40, 0xe0, 0x90, 0x48, 0x1c, 0x00, 0x00, 0x00,  //
0x80, 0x48, 0x24, 0x72, 0xc9, 0x64, 0x2e, 0x00, 0x00, 0x00,  //
0x00, 0x81, 0x40, 0xe0, 0x90, 0x48, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x47, 0x04, 0xc2, 0x81, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x80, 0x44, 0xa5, 0x72, 0xa9, 0x54, 0x12, 0x00, 0x00, 0x00,  //
0x00, 0x4f, 0x24, 0xe2, 0xa1, 0x48, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x01, 0xf1, 0x44, 0x3c, 0x00, 0x00, 0x00,  //
0x00, 0x4f, 0xc0, 0x11, 0x89, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xe0, 0x11, 0x79, 0x44, 0x1e, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x23, 0x10, 0x08, 0x04, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0xa1, 0x50, 0x28, 0x3e, 0x11, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x11, 0xf9, 0x04, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xa0, 0x52, 0x71, 0x54, 0x2a, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x11, 0x61, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x92, 0xa9, 0x4c, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x05, 0x21, 0x92, 0xa9, 0x4c, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x92, 0x38, 0x24, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x80, 0x23, 0x91, 0x48, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0xb2, 0xa9, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x12, 0xf9, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x11, 0x89, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xe0, 0x13, 0x89, 0x44, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xa0, 0x31, 0x89, 0x4c, 0x1a, 0x81, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x11, 0x09, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xe0, 0x43, 0x20, 0x10, 0x08, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x12, 0x89, 0x78, 0x20, 0x11, 0x07, 0x00,  //
0x00, 0x02, 0xc1, 0x51, 0xa9, 0x54, 0x1c, 0x04, 0x02, 0x00,  //
0x00, 0x00, 0x20, 0xa2, 0x20, 0x28, 0x22, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x91, 0x48, 0x24, 0x3e, 0x10, 0x08, 0x00,  //
0x00, 0x00, 0x20, 0x12, 0xf1, 0x40, 0x20, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xa0, 0x52, 0xa9, 0x54, 0x3e, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xa0, 0x52, 0xa9, 0x54, 0x3e, 0x10, 0x08, 0x00,  //
0x00, 0x00, 0x60, 0x20, 0x70, 0x48, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x12, 0xb9, 0x64, 0x2e, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x40, 0x20, 0x70, 0x48, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x11, 0xe1, 0x44, 0x1c, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0x20, 0x51, 0xb9, 0x54, 0x12, 0x00, 0x00, 0x00,  //
0x00, 0x00, 0xc0, 0x13, 0xf1, 0x44, 0x22, 0x00, 0x00, 0x00,  //
};


FontDef_t Font_7x10 = {
	7,
	10,
	10,
	Font7x10
};

char* FONTS_GetStringSize(char* str, FONTS_SIZE_t* SizeStruct, FontDef_t* Font) {
	/* Fill settings */
	SizeStruct->Height = Font->FontHeight;
	SizeStruct->Length = Font->FontWidth * strlen(str);
	
	/* Return pointer */
	return str;
}