/* Copyright (c) 2020  Paulo Costa
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in
     the documentation and/or other materials provided with the
     distribution.
   * Neither the name of the copyright holders nor the names of
     contributors may be used to endorse or promote products derived
     from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE. */

/*
Function Code  What the function does                           Value type  Access type
01     (0x01)  Reading DO  Read Coil Status                      Discrete   Reading
02     (0x02)  Reading DI  Read Input Status                     Discrete   Reading
03     (0x03)  Reading AO  Read Holding Registers                16 bit     Reading
04     (0x04)  Reading AI  Read Input Registers                  16 bit     Reading
05     (0x05)  One DO recording  Force Single Coil               Discrete   Recording
06     (0x06)  Recording one AO  Preset Single Register          16 bit     Recording
15     (0x0F)  Multiple DO recording Force Multiple Coils        Discrete   Recording
16     (0x10)  Recording multiple AOs  Preset Multiple Registers 16 bit     Recording
*/

#ifndef MODBUSTCP_H
#define MODBUSTCP_H

#define NUM_COILS  32
#define NUM_INPUTS 32

#define MB_BUF_SIZE 64

#include "stdint.h"


typedef struct {
  uint16_t TransactionIdentifier;  // 2 bytes - For synchronization between messages of server & client
  uint16_t ProtocolIdentifier;     // 2 bytes - Zero for Modbus/TCP
  uint16_t LengthField;            // 2 bytes - Number of remaining bytes in this frame
  uint8_t UnitIdentifier;          // 1 byte - Slave Address (255 if not used)
  uint8_t FunctionCode;            // 1 byte - Function codes as in other variants
  uint8_t BytesNum;
  uint16_t Address;
  uint16_t Num;
} modbusFrame_t;


typedef enum {
  mbsIdle, mbsHeader, msbLen, msbUnitId, msbFunctionCode, msbBytesNum, msbBytes, msbError
} read_state_t;

class modbusTCP_t
{
  public:
    uint32_t Inputs;
    uint32_t Coils;

    uint8_t out_buf[MB_BUF_SIZE];
    uint8_t out_buf_count;

    uint8_t in_buf[MB_BUF_SIZE];
    uint8_t in_buf_count;

    modbusFrame_t Frame;

    read_state_t state;
    uint8_t byteCount;

    uint16_t cur_transaction_id;
    uint16_t ReceivedMessagesCount;
    
    modbusTCP_t();

   void set_coil(uint8_t coil_num, uint8_t value);
   uint8_t get_coil(uint8_t coil_num);

   uint8_t get_input(uint8_t input_num);

   void clear_out_buf(void);
   void clear_in_buf(void);
   void add_out_buf(uint8_t data);
   void add16_out_buf(uint16_t data);

   void write_multiple_coils(void);
   void read_multiple_inputs(void);

   void read_state_machine(uint8_t b);

   void process_input_message(void);
};

#endif // modbusTCP_H
