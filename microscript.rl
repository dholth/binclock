#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "microscript.h"

enum {
  MAX_VARIABLE_LENGTH = 8   // Copy this number to var rule
};

static uint16_t currentType;
static uint16_t currentNumber;
static uint16_t currentTime[3];
static uint16_t variable_index = 0;

char variable[MAX_VARIABLE_LENGTH + 1];

%%{
  machine microscript;
  
  action ClearNumber { currentNumber = 0; }
  action RecordVariable { variable[variable_index++] = (*p); }
  action ClearVariable { variable_index = 0; memset(variable, 0, MAX_VARIABLE_LENGTH); }
  action ClearTime { memset(currentTime, 0, sizeof(currentTime)); }
  
  action RecordHour { currentTime[0] = currentNumber; }
  action RecordMinute { currentTime[1] = currentNumber; }
  action RecordSecond { currentTime[2] = currentNumber; }
  action RecordAM_PM { }

# For a different strategy where the type is passed to a single function
  action TypeTime { currentType = TYPE_TIME; }
  action TypeNumber { currentType = TYPE_NUMBER; }
  
  action DisplayVariable { var_display(variable); }
  action SetVariable { var_set(variable, currentNumber); }
  action SetTime { time_set(variable, currentTime); }
  
  action ReportError { }
  
  action RecordDigit {
    uint8_t digit = (*p) - '0';
    currentNumber = currentNumber * 10 + digit;
  }
  
  var = (([a-z] @RecordVariable){1,8});
  
  number = ((digit @RecordDigit)+) >ClearNumber %TypeNumber;
  
  time = (number @RecordHour ':' number @RecordMinute 
    ( ':' number @RecordSecond)?
    ( 'a' | 'p' @RecordAM_PM)? %TypeTime) >ClearTime;

  # An assignment looks like "a = 3"
  number_assignment = (var space* '=' space* number) >ClearVariable;
  time_assignment = (var space* '=' space* time) >ClearVariable;
  
  end_command = ';' | '\n';
  
  # We're going to use a semicolon on the end of commands to make the boundaries clear
  display_command = space* var space* end_command >DisplayVariable;
  assign_number = space* number_assignment space* end_command >SetVariable;
  assign_time = space* time_assignment space* end_command >SetTime;
  
  # A program can consist of any number of commands
  main := (assign_number | assign_time | display_command)* $err(ReportError);
}%%

%% write data;

static uint8_t cs; /* The current parser state */

void init_microscript( void ) {
    %% write init;
}

void parse_microscript(const char* p, uint16_t len, uint8_t is_eof) {
    const char* pe = p + len; /* pe points to 1 byte beyond the end of this block of data */
    const char* eof = is_eof ? pe : ((char*) 0); /* Indicates the end of all data, 0 if not in this block */    
    
    %% write exec;
}
