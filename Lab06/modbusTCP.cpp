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

#include "modbusTCP.h"

// https://ipc2u.com/articles/knowledge-base/detailed-description-of-the-modbus-tcp-protocol-with-command-examples//*

void modbusTCP_t::clear_out_buf(void)
{
  for(uint8_t i = 0; i < MB_BUF_SIZE; i++) out_buf[i] = 0;
  out_buf_count = 0;
}

void modbusTCP_t::clear_in_buf(void)
{
  for(uint8_t i = 0; i < MB_BUF_SIZE; i++) in_buf[i] = 0;
  in_buf_count = 0;
}

void modbusTCP_t::add_out_buf(uint8_t data)
{
  if (out_buf_count >= MB_BUF_SIZE) return;
  out_buf[out_buf_count] = data;
  out_buf_count++;
}

void modbusTCP_t::add16_out_buf(uint16_t data)
{
  if (out_buf_count >= MB_BUF_SIZE - 1) return;
  out_buf[out_buf_count] = (data >> 8) & 0xFF;
  out_buf_count++;
  out_buf[out_buf_count] = data & 0xFF;
  out_buf_count++;
}

void modbusTCP_t::write_multiple_coils(void)
{
  clear_out_buf();
  add16_out_buf(cur_transaction_id);
  add16_out_buf(0);    // Protocol identifier
  add16_out_buf(11);    // Message Length
  add_out_buf(1);      // Device address
  add_out_buf(0x0F);   // Force Multiple Coils
  add16_out_buf(0);   // base coil
  add16_out_buf(32);   // 32 coils
  add_out_buf(4);      // Payload size
  add_out_buf(Coils & 0xFF);
  add_out_buf((Coils >> 8) & 0xFF);
  add_out_buf((Coils >> 16) & 0xFF);
  add_out_buf((Coils >> 24) & 0xFF);
}

void modbusTCP_t::read_multiple_inputs(void)
{
  clear_out_buf();
  add16_out_buf(cur_transaction_id);
  add16_out_buf(0);    // Protocol identifier
  add16_out_buf(6);    // Message Length
  add_out_buf(1);      // Device address
  add_out_buf(0x02);   // Read Multiple Inputs
  add16_out_buf(0);    // base input
  add16_out_buf(32);   // 32 inputs
}

modbusTCP_t::modbusTCP_t()
{
  clear_out_buf();
  cur_transaction_id = 0xB16E;
  ReceivedMessagesCount = 0;
}

void modbusTCP_t::set_coil(uint8_t coil_num, uint8_t value)
{
  if (coil_num >= NUM_COILS) return;
  if (value)  Coils |= 1 << coil_num;
  else Coils &= ~(1 << coil_num);
}

uint8_t modbusTCP_t::get_coil(uint8_t coil_num)
{
  if (coil_num >= NUM_COILS) return 0;
  return !!(Coils & (1 << coil_num));
}


uint8_t modbusTCP_t::get_input(uint8_t input_num)
{
  if (input_num >= NUM_INPUTS) return 0;
  return !!(Inputs & (1 << input_num));
}


//mbsIdle, mbsHeader, msbLen, msbUnitId, msbFunctionCode, msbBytesNum, msbBytes, msbError
void modbusTCP_t::read_state_machine(uint8_t b)
{
  switch(state) {
      case mbsIdle:
        if (b == 0xB1) {
          state = mbsHeader;
          byteCount = 1;
          Frame.TransactionIdentifier = 0xB1 << 8;
        }
      break;

      case mbsHeader:     // B1 6E 00 00
        if ((byteCount == 1) && (b == 0x6E)) {
          Frame.TransactionIdentifier = Frame.TransactionIdentifier | 0x6E;
          byteCount++;
        } else if ((byteCount == 2) && (b == 0)) {
          Frame.ProtocolIdentifier = 0;
          byteCount++;
        } else if ((byteCount == 3) && (b == 0)) {
          Frame.ProtocolIdentifier = 0;
          state = msbLen;
          byteCount = 0;
        } else {   // There was an error: resync
          state = mbsIdle;
          Frame.TransactionIdentifier = -1;
          //continue;
        }
      break;

      case msbLen:
        if (byteCount == 0) {
          Frame.LengthField = b << 8;
          byteCount++;
        } else if (byteCount == 1) {
          Frame.LengthField = Frame.LengthField | b;
          state = msbUnitId;
        }
      break;

      case msbUnitId:
        Frame.UnitIdentifier = b;
        state = msbFunctionCode;
      break;

      case msbFunctionCode: 
        Frame.FunctionCode = b;
        clear_in_buf();  // Clean the input buffer
        // This FunctionCodes report how many more bytes they expect
        if (Frame.FunctionCode == 0x01 || 
            Frame.FunctionCode == 0x02 ||
            Frame.FunctionCode == 0x03 ||
            Frame.FunctionCode == 0x04 ||
            Frame.FunctionCode == 0x02) {
          state = msbBytesNum;
          byteCount = 0;
        // This FunctionCodes dot report how many more bytes they expect
        } else if (Frame.FunctionCode == 0x05 ||
                   Frame.FunctionCode == 0x06 || 
                   Frame.FunctionCode == 0x10 || 
                   Frame.FunctionCode == 0x0F) {
          Frame.BytesNum = 4;
          state = msbBytes;
          byteCount = 0;
        } else {  // Unrecognized Function Code: Resync
          state = mbsIdle;
          Frame.TransactionIdentifier = -1;
        }
      break;

      case msbBytesNum: 
        Frame.BytesNum = b;
        state = msbBytes;
        byteCount = 0;
      break;

      case msbBytes: 
        if (in_buf_count < MB_BUF_SIZE) {
          in_buf[in_buf_count] = b;
          in_buf_count++;
        }

        if (byteCount >= Frame.BytesNum - 1) {  // All bytes read: Resync
          process_input_message();
          ReceivedMessagesCount++;
          state = mbsIdle;
          Frame.TransactionIdentifier = -1;
        }
        byteCount++;
      break;
      
      default:
      break;
  }
  
}


void modbusTCP_t::process_input_message(void)
{
  switch (Frame.FunctionCode) {
    case 0x02:
      uint32_t data = in_buf[3];
      data = (data << 8) | in_buf[2];
      data = (data << 8) | in_buf[1];
      data = (data << 8) | in_buf[0];
      Inputs = data;
    break;
  }
}
