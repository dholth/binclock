/**
 * Ragel microscript implementation. Set or view named variables.
 */

#pragma once

void init_microscript(void);
void parse_microscript(const char* p, uint16_t len, uint8_t is_eof);
void var_display(const char *variable);
void var_set(const char *variable, uint16_t currentNumber);
void time_set(const char *variable, uint16_t currentTime[]);

enum {
  TYPE_NUMBER,
  TYPE_TIME
} microscript_types;
