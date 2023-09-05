#pragma once

#include "core.h"

#include <cstdarg>
#include <string>

void usart_isr();
namespace Serial
{
void Init();
char Read();
int Print(const std::string& str);
int Printf(const char* format, ...);
uint8_t Available();
}