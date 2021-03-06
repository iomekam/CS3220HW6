#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <bitset>
#include <stdint.h>
#include <string.h> 
#include <cstring> 
#include <limits.h> 
// #include <cstdint> 
#include "simulator.h"


#define FLOAT_TO_FIXED1114(n) ((int)((n) * (float)(1<<(4)))) & 0xffff
#define FIXED_TO_FLOAT1114(n) ((float)(-1*((n>>15)&0x1)*(1<<11)) + (float)((n&(0x7fff)) / (float)(1<<4)))
#define FIXED1114_TO_INT(n) (( (n>>15)&0x1) ?  ((n>>4)|0xf000) : (n>>4)) 
#define DEBUG

using namespace std;

///////////////////////////////////
///  architectural structures /// 
///////////////////////////////////


ScalarRegister g_condition_code_register; // store conditional code 
ScalarRegister g_scalar_registers[NUM_SCALAR_REGISTER];  
VectorRegister g_vector_registers[NUM_VECTOR_REGISTER];

VertexRegister g_gpu_vertex_registers[NUM_VERTEX_REGISTER]; 
ScalarRegister g_gpu_status_register; 
 
unsigned char g_memory[MEMORY_SIZE]; // data memory 

////////////////////////////////////

vector<TraceOp> g_trace_ops;

unsigned int g_instruction_count = 0;
unsigned int g_vertex_id = 0; 
unsigned int g_current_pc = 0; 
unsigned int g_program_halt = 0; 
unsigned int active_vertex_reg = 0;

////////////////////////////////////////////////////////////////////////
// desc: Set g_condition_code_register depending on the values of val1 and val2
// hint: bit0 (N) is set only when val1 < val2
// bit 2: negative 
// bit 1: zero
// bit 0: positive 
////////////////////////////////////////////////////////////////////////
void SetConditionCodeInt(const int16_t val1, const int16_t val2) 
{
  if(val1 < val2) {
    g_condition_code_register.int_value = 4;
  }
  else if(val1 == val2) {
    g_condition_code_register.int_value = 2;
  }
  else {
    g_condition_code_register.int_value = 1;
  }
}

////////////////////////////////////////////////////////////////////////
// Initialize global variables
////////////////////////////////////////////////////////////////////////
void InitializeGlobalVariables() 
{
  g_vertex_id = 0;  // internal setting variables 
  memset(&g_condition_code_register, 0x00, sizeof(ScalarRegister));
  memset(&g_gpu_status_register, 0x00, sizeof(ScalarRegister));
  memset(g_scalar_registers, 0x00, sizeof(ScalarRegister) * NUM_SCALAR_REGISTER);
  memset(g_vector_registers, 0x00, sizeof(VectorRegister) * NUM_VECTOR_REGISTER);
  memset(g_gpu_vertex_registers, 0x00, sizeof(VertexRegister) * NUM_VERTEX_REGISTER);
  memset(g_memory, 0x00, sizeof(unsigned char) * MEMORY_SIZE);
}

////////////////////////////////////////////////////////////////////////
// desc: Convert 16-bit 2's complement signed integer to 32-bit
////////////////////////////////////////////////////////////////////////
int SignExtension(const int16_t value) 
{
  return (value >> 15) == 0 ? value : ((0xFFFF << 16) | value);
}



////////////////////////////////////////////////////////////////////////
// desc: Decode binary-encoded instruction and Parse into TraceOp structure
//       which we will use execute later
// input: 32-bit encoded instruction
// output: TraceOp structure filled with the information provided from the input
////////////////////////////////////////////////////////////////////////
TraceOp DecodeInstruction(const uint32_t instruction) 
{
  TraceOp ret_trace_op;
  memset(&ret_trace_op, 0x00, sizeof(ret_trace_op));

  uint8_t opcode = (instruction & 0xFF000000) >> 24;
  ret_trace_op.opcode = opcode;

  switch (opcode) {
    
   
   
   case OP_ADD_D: 
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int source_register_2_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.scalar_registers[2] = source_register_2_idx;
    }
    break;
    
    /* fill out all the instruction's decode information */ 

   case OP_ADD_F:  
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int source_register_2_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.scalar_registers[2] = source_register_2_idx;
    }
    break;

    case OP_ADDI_D:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int inst_value = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = inst_value;
    }
    break;

    case OP_ADDI_F:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int inst_value = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = inst_value;
    }
    break;

    case OP_VADD:
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      int source_register_1_idx = (instruction & 0x00003F00) >> 8;
      int source_register_2_idx = (instruction & 0x0000003F);
      ret_trace_op.vector_registers[0] = destination_register_idx;
      ret_trace_op.vector_registers[1] = source_register_1_idx;
      ret_trace_op.vector_registers[2] = source_register_2_idx;
    }
    break;

    case OP_AND_D:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int source_register_2_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.scalar_registers[2] = source_register_2_idx;
    }
    break;

    case OP_ANDI_D:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_MOV: 
    {
      int destination_register_idx = (instruction & 0x000F0000) >> 16;
      int source_register_1_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
    }
    break;

    case OP_MOVI_D:
    {
      int destination_register_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_MOVI_F: 
    {
      int destination_register_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_VMOV:
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      int source_register_1_idx = (instruction & 0x00003F00) >> 8;
      ret_trace_op.vector_registers[0] = destination_register_idx;
      ret_trace_op.vector_registers[1] = source_register_1_idx;
    }
    break;

    case OP_VMOVI: 
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.vector_registers[0] = destination_register_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_CMP: 
    {
      int destination_register_idx = (instruction & 0x000F0000) >> 16;
      int source_register_1_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.scalar_registers[1] = destination_register_idx;
      ret_trace_op.scalar_registers[2] = source_register_1_idx;
    }
    break;

    case OP_CMPI:
    {
      int destination_register_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[1] = destination_register_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_VCOMPMOV: 
    {
      int idx_val = (instruction & 0x00C00000) >> 22;
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      int src_register_idx = (instruction & 0x00000F00) >> 8;
      ret_trace_op.idx = idx_val;
      ret_trace_op.vector_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = src_register_idx;
    }
    break;

    case OP_VCOMPMOVI:  
    {
      int idx_val = (instruction & 0x00C00000) >> 22;
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      int src_register_idx = (instruction & 0x0000FFFF);
      ret_trace_op.idx = idx_val;
      ret_trace_op.vector_registers[0] = destination_register_idx;
      ret_trace_op.int_value = src_register_idx;
    }
    break;

    case OP_LDB:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = imm16;
    }
    break; 

    case OP_LDW:
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_STB:  
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_STW: 
    {
      int destination_register_idx = (instruction & 0x00F00000) >> 20;
      int source_register_1_idx = (instruction & 0x000F0000) >> 16;
      int imm16 = (instruction & 0x0000FFFF);
      ret_trace_op.scalar_registers[0] = destination_register_idx;
      ret_trace_op.scalar_registers[1] = source_register_1_idx;
      ret_trace_op.int_value = imm16;
    }
    break;

    case OP_SETVERTEX: 
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      ret_trace_op.vector_registers[0] = destination_register_idx;
    }
    break;

    case OP_SETCOLOR:
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      ret_trace_op.vector_registers[0] = destination_register_idx;
    }
    break;

    case OP_ROTATE:  // optional 
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      ret_trace_op.vector_registers[0] = destination_register_idx;
    }
    break;

    case OP_TRANSLATE: 
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      ret_trace_op.vector_registers[0] = destination_register_idx;
    }
    break;

    case OP_SCALE:  // optional 
    {
      int destination_register_idx = (instruction & 0x003F0000) >> 16;
      ret_trace_op.vector_registers[0] = destination_register_idx;
    }
    break;
    
    case OP_PUSHMATRIX:       // deprecated 
    case OP_POPMATRIX:   // deprecated 
    case OP_BEGINPRIMITIVE: 
    {
      int prim = (instruction & 0x000F0000) >> 16;
      ret_trace_op.primitive_type = prim;
    }
    break;

    case OP_ENDPRIMITIVE:  //deprecated
    case OP_LOADIDENTITY:  // deprecated 
    case OP_FLUSH: 
    case OP_DRAW: 
    case OP_BRN: 
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_BRZ:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_BRP:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_BRNZ:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_BRNP:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_BRZP:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;
    case OP_BRNZP:
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;
    case OP_JMP:
    {
      int prim = (instruction & 0x000F0000) >> 16;
      ret_trace_op.scalar_registers[0] = prim;
    }
    break;

    case OP_JSR: 
    {
      int prim = (instruction & 0x0000FFFF);
      ret_trace_op.int_value = prim;
    }
    break;

    case OP_JSRR: 
    {
      int prim = (instruction & 0x000F0000) >> 16;
      ret_trace_op.scalar_registers[0] = prim;
    }
    break;
    
    case OP_HALT: 
      break; 
      

    default:
    break;
  }

  return ret_trace_op;
}

////////////////////////////////////////////////////////////////////////
// desc: Execute the behavior of the instruction (Simulate)
// input: Instruction to execute 
// output: Non-branch operation ? -1 : OTHER (PC-relative or absolute address)
////////////////////////////////////////////////////////////////////////
int ExecuteInstruction(const TraceOp &trace_op) 
{
  int ret_next_instruction_idx = -1;

  uint8_t opcode = trace_op.opcode;
  switch (opcode) {
    case OP_ADD_D: 
      {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = g_scalar_registers[trace_op.scalar_registers[2]].int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
        source_value_1 + source_value_2;
      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;


    /* fill out instruction behaviors */ 

    case OP_ADD_F:
      {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = g_scalar_registers[trace_op.scalar_registers[2]].int_value;

      g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
        source_value_1 + source_value_2;
      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
      }  
      break;
    case OP_ADDI_D:
      {
        int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
        int source_value_2 = trace_op.int_value;
        g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
          source_value_1 + source_value_2;
        SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
      }

      break;
    case OP_ADDI_F: 
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
        int source_value_2 = trace_op.int_value;


        g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
          source_value_1 + source_value_2;
        SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }
    break;
    case OP_VADD:
    {
      for(int count = 0; count < 4; count++) {
          int source_value_1 = g_vector_registers[trace_op.vector_registers[1]].element[count].int_value;
          int source_value_2 = g_vector_registers[trace_op.vector_registers[2]].element[count].int_value;
          g_vector_registers[trace_op.vector_registers[0]].element[count].int_value = 
            source_value_1 + source_value_2;
        }
    }

    break;

    case OP_AND_D:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = g_scalar_registers[trace_op.scalar_registers[2]].int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
        source_value_1 & source_value_2;
      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;

    case OP_ANDI_D:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = 
        source_value_1 & source_value_2;
      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;

    case OP_MOV:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = source_value_1;

      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;

    case OP_MOVI_D:
    {
      int source_value_1 = trace_op.int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = source_value_1;

      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;
    case OP_MOVI_F: 
    {
      int source_value_1 = trace_op.int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = source_value_1;

      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }
    case OP_VMOV:
    {
      // int idxD = trace_op.vector_registers[0];
      // int idx = trace_op.vector_registers[1];
      // ScalarRegister* dest = g_vector_registers[idxD].element;
      // ScalarRegister* src = g_vector_registers[idx].element;

      // memcpy(dest, src, sizeof(ScalarRegister)*4);
      int idx = trace_op.vector_registers[0];
      for(int count = 0; count < 4; count++) {
           g_vector_registers[idx].element[count].int_value = g_vector_registers[trace_op.vector_registers[1]].element[count].int_value;
      }
    } 

    break;

    case OP_VMOVI: 
    {
      int idx = trace_op.vector_registers[0];
      for(int count = 0; count < 4; count++) {
           g_vector_registers[idx].element[count].int_value = trace_op.int_value;
        }
    }

    break;

    case OP_CMP:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = g_scalar_registers[trace_op.scalar_registers[2]].int_value;

      int value = 0;

      if(source_value_1 < source_value_2) {
        value = -1;
      }
      else if(source_value_1 > source_value_2){
        value = 1;
      }

      SetConditionCodeInt(value, 0);
    } 

    break;
    case OP_CMPI:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;

      int value = 0;

      if(source_value_1 < source_value_2) {
        value = -1;
      }
      else if(source_value_1 > source_value_2) {
        value = 1;
      }

      SetConditionCodeInt(value, 0);
    }

    break;
    case OP_VCOMPMOV: 
    {
      int idx = trace_op.vector_registers[0];
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      g_vector_registers[idx].element[trace_op.idx].int_value = 
        source_value_1;
    }

    break;

    case OP_VCOMPMOVI:  
    {
      int source_value_1 = trace_op.int_value;
      int idx = trace_op.vector_registers[0];
      g_vector_registers[idx].element[trace_op.idx].int_value = 
        source_value_1;
    }

    break;

    case OP_LDB:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = g_memory[source_value_1 + source_value_2];

      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;

    case OP_LDW:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;
      int dest = g_memory[source_value_1 + source_value_2 + 1] << 8 | g_memory[source_value_1 + source_value_2];
      g_scalar_registers[trace_op.scalar_registers[0]].int_value = dest;
      
      SetConditionCodeInt(g_scalar_registers[trace_op.scalar_registers[0]].int_value, 0);
    }

    break;

    case OP_STB:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;
      g_memory[source_value_1 + source_value_2] = g_scalar_registers[trace_op.scalar_registers[0]].int_value;
    }

    break;

    case OP_STW:
    {
      int source_value_1 = g_scalar_registers[trace_op.scalar_registers[1]].int_value;
      int source_value_2 = trace_op.int_value;
      int value = g_scalar_registers[trace_op.scalar_registers[0]].int_value;
      g_memory[source_value_1 + source_value_2 + 1]  = value >> 8;
      g_memory[source_value_1 + source_value_2]  = value & 0x00FF;
    }

    break;

    case OP_SETVERTEX: 
    {
      int x = g_vector_registers[trace_op.vector_registers[0]].element[1].int_value;
      int y = g_vector_registers[trace_op.vector_registers[0]].element[2].int_value;
      int z = g_vector_registers[trace_op.vector_registers[0]].element[3].int_value;


      g_gpu_vertex_registers[active_vertex_reg].x_value = x >> 4;
      g_gpu_vertex_registers[active_vertex_reg].y_value = y >> 4;
      g_gpu_vertex_registers[active_vertex_reg].z_value = z >> 4;
      active_vertex_reg ++;
      if(active_vertex_reg > 2)
      {
        active_vertex_reg = 0;
      }

    }
    break;
    case OP_SETCOLOR:
    {
      int r = g_vector_registers[trace_op.vector_registers[0]].element[0].int_value;
      int g = g_vector_registers[trace_op.vector_registers[0]].element[1].int_value;
      int b = g_vector_registers[trace_op.vector_registers[0]].element[2].int_value;


      g_gpu_vertex_registers[0].r_value = (r >> 4);
      g_gpu_vertex_registers[0].g_value = (g >> 4);
      g_gpu_vertex_registers[0].b_value = (b >> 4);
    }
    break;
    case OP_ROTATE:  // optional 
    break;
    case OP_TRANSLATE:
    {
      g_gpu_vertex_registers[0].x_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[1].int_value);
      g_gpu_vertex_registers[0].y_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[2].int_value);

      g_gpu_vertex_registers[1].x_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[1].int_value);
      g_gpu_vertex_registers[1].y_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[2].int_value);

      g_gpu_vertex_registers[2].x_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[1].int_value);
      g_gpu_vertex_registers[2].y_value += FIXED1114_TO_INT(g_vector_registers[trace_op.vector_registers[0]].element[2].int_value);
    }
    break;
    case OP_SCALE:  // optional 
    break;
    case OP_PUSHMATRIX:       // deprecated 
    break;
    case OP_POPMATRIX:   // deprecated 
    break;
    case OP_BEGINPRIMITIVE: 
    {
      if(trace_op.primitive_type == 0){//line
        g_gpu_status_register.int_value |= 8;
        g_gpu_status_register.int_value &= ~(4); // clear the primitive bit
      }
      else//triangle
      {
        g_gpu_status_register.int_value |= 4;
        g_gpu_status_register.int_value &= ~(8); 
      }
      
    }
    break;
    case OP_ENDPRIMITIVE: //deprecated
    break;
    case OP_LOADIDENTITY:  // deprecated 
    break;
    case OP_FLUSH: 
    {
      g_gpu_status_register.int_value |= 2;
      g_gpu_status_register.int_value &= ~(1);
      
    }
    break;
    case OP_DRAW: 
    {
      g_gpu_status_register.int_value |= 1;
      g_gpu_status_register.int_value &= ~(2);
      
    }
    break;
    case OP_BRN: 
    {
      if(g_condition_code_register.int_value == 4)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 
    case OP_BRZ:
    {
      if(g_condition_code_register.int_value == 2)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 
    case OP_BRP:
    {
      if(g_condition_code_register.int_value == 1)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 

    case OP_BRNZ:
    {
      if(g_condition_code_register.int_value == 2 | g_condition_code_register.int_value == 4)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 

    case OP_BRNP:
    {
      if(g_condition_code_register.int_value == 1 | g_condition_code_register.int_value == 4)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 

    case OP_BRZP:
    {
      if(g_condition_code_register.int_value == 2 | g_condition_code_register.int_value == 1)
      {
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
      }
      else
      {
        ret_next_instruction_idx = 0;
      }
    }
    break; 

    case OP_BRNZP:
        ret_next_instruction_idx = SignExtension(trace_op.int_value);
    break; 

    case OP_JMP:
    {
      ret_next_instruction_idx = g_scalar_registers[trace_op.scalar_registers[0]].int_value >> 2;//trace_op.scalar_registers[0].int_value;
    }
    break;
    case OP_JSR:
    {
      g_scalar_registers[LR_IDX].int_value = g_scalar_registers[PC_IDX].int_value;
      ret_next_instruction_idx = SignExtension(trace_op.int_value);
    }
    break; 
    case OP_JSRR: 
    {
      g_scalar_registers[LR_IDX].int_value = g_scalar_registers[PC_IDX].int_value;
      ret_next_instruction_idx = g_scalar_registers[trace_op.scalar_registers[0]].int_value >> 2;//trace_op.scalar_registers[0].int_value;
    }
    break; 
      

    case OP_HALT: 
    g_program_halt = 1; 
    break; 

    default:
    break;
    }

  return ret_next_instruction_idx;
}

////////////////////////////////////////////////////////////////////////
// desc: Dump given trace_op
////////////////////////////////////////////////////////////////////////
void PrintTraceOp(const TraceOp &trace_op) 
{  
  cout << "  opcode: " << SignExtension(trace_op.opcode);
  cout << ", scalar_register[0]: " << (int) trace_op.scalar_registers[0];
  cout << ", scalar_register[1]: " << (int) trace_op.scalar_registers[1];
  cout << ", scalar_register[2]: " << (int) trace_op.scalar_registers[2];
  cout << ", vector_register[0]: " << (int) trace_op.vector_registers[0];
  cout << ", vector_register[1]: " << (int) trace_op.vector_registers[1];
  cout << ", idx: " << (int) trace_op.idx;
  cout << ", primitive_index: " << (int) trace_op.primitive_type;
  cout << ", int_value: " << (int) SignExtension(trace_op.int_value) << endl; 
  //c  cout << ", float_value: " << (float) trace_op.float_value << endl;
}

////////////////////////////////////////////////////////////////////////
// desc: This function is called every trace is executed
//       to provide the contents of all the registers
////////////////////////////////////////////////////////////////////////
void PrintContext(const TraceOp &current_op)
{

  cout << "--------------------------------------------------" << endl;
  cout << "3220X-Instruction Count: " << g_instruction_count
       << " C_PC: " << (g_current_pc *4)
       << " C_PC_IND: " << g_current_pc 
       << ", Curr_Opcode: " << current_op.opcode
       << " NEXT_PC: " << ((g_scalar_registers[PC_IDX].int_value)<<2) 
       << " NEXT_PC_IND: " << (g_scalar_registers[PC_IDX].int_value)
       << ", Next_Opcode: " << g_trace_ops[g_scalar_registers[PC_IDX].int_value].opcode 
       << endl;
  cout <<"3220X-"; 
  for (int srIdx = 0; srIdx < NUM_SCALAR_REGISTER; srIdx++) {
    cout << "R" << srIdx << ":" 
         << ((srIdx < 8 || srIdx == 15) ? SignExtension(g_scalar_registers[srIdx].int_value) : (float) FIXED_TO_FLOAT1114(g_scalar_registers[srIdx].int_value)) 
         << (srIdx == NUM_SCALAR_REGISTER-1 ? "" : ", ");
  }

  cout << " CC :N: " << ((g_condition_code_register.int_value &0x4) >>2) << " Z: " 
       << ((g_condition_code_register.int_value &0x2) >>1) << " P: " << (g_condition_code_register.int_value &0x1) << "  "; 
  cout << " draw: " << (g_gpu_status_register.int_value &0x01) << " fush: " << ((g_gpu_status_register.int_value & 0x2)>>1) ;
  cout << " prim_type: "<< ((g_gpu_status_register.int_value & 0x4) >> 2)  << " "; 
   
  cout << endl;
  
  // for (int vrIdx = 0; vrIdx < NUM_VECTOR_REGISTER; vrIdx++) {
  
  for (int vrIdx = 0; vrIdx < 6; vrIdx++) {
    cout <<"3220X-"; 
    cout << "V" << vrIdx << ":";
    for (int elmtIdx = 0; elmtIdx < NUM_VECTOR_ELEMENTS; elmtIdx++) { 
      cout << "Element[" << elmtIdx << "] = " 
           << (float)FIXED_TO_FLOAT1114(g_vector_registers[vrIdx].element[elmtIdx].int_value) 
           << (elmtIdx == NUM_VECTOR_ELEMENTS-1 ? "" : ",");
    }
    cout << endl;
  }
  cout << endl;
  cout <<"3220X-"; 
  cout <<" vertices P1_X: " << g_gpu_vertex_registers[0].x_value; 
  cout <<" vertices P1_Y: " << g_gpu_vertex_registers[0].y_value; 
  cout <<" r: " << g_gpu_vertex_registers[0].r_value; 
  cout <<" g: " << g_gpu_vertex_registers[0].g_value; 
  cout <<" b: " << g_gpu_vertex_registers[0].b_value; 
  cout <<" P2_X: " << g_gpu_vertex_registers[1].x_value; 
  cout <<" P2_Y: " << g_gpu_vertex_registers[1].y_value; 
  cout <<" r: " << g_gpu_vertex_registers[1].r_value; 
  cout <<" g: " << g_gpu_vertex_registers[1].g_value; 
  cout <<" b: " << g_gpu_vertex_registers[1].b_value; 
  cout <<" P3_X: " << g_gpu_vertex_registers[2].x_value; 
  cout <<" P3_Y: " << g_gpu_vertex_registers[2].y_value; 
  cout <<" r: " << g_gpu_vertex_registers[2].r_value; 
  cout <<" g: " << g_gpu_vertex_registers[2].g_value; 
  cout <<" b: " << g_gpu_vertex_registers[2].b_value << endl; 
  
  cout << "--------------------------------------------------" << endl;
}

int main(int argc, char **argv) 
{
  ///////////////////////////////////////////////////////////////
  // Initialize Global Variables
  ///////////////////////////////////////////////////////////////
  //
  InitializeGlobalVariables();

  ///////////////////////////////////////////////////////////////
  // Load Program
  ///////////////////////////////////////////////////////////////
  //
  if (argc != 2) {
    cerr << "Usage: " << argv[0] << " <input>" << endl;
    return 1;
  }

  ifstream infile(argv[1]);
  if (!infile) {
    cerr << "Error: Failed to open input file " << argv[1] << endl;
    return 1;
  }
  vector< bitset<sizeof(uint32_t)*CHAR_BIT> > instructions;
  while (!infile.eof()) {
    bitset<sizeof(uint32_t)*CHAR_BIT> bits;
    infile >> bits;
    if (infile.eof())  break;
    instructions.push_back(bits);
  }
  
  infile.close();

#ifdef DEBUG
  cout << "The contents of the instruction vectors are :" << endl;
  for (vector< bitset<sizeof(uint32_t)*CHAR_BIT> >::iterator ii =
      instructions.begin(); ii != instructions.end(); ii++) {
    cout << "  " << *ii << endl;
  }
#endif // DEBUG

  ///////////////////////////////////////////////////////////////
  // Decode instructions into g_trace_ops
  ///////////////////////////////////////////////////////////////
  //
  for (vector< bitset<sizeof(uint32_t)*CHAR_BIT> >::iterator ii =
      instructions.begin(); ii != instructions.end(); ii++) {
    uint32_t inst = (uint32_t) ((*ii).to_ulong());
    TraceOp trace_op = DecodeInstruction(inst);
    g_trace_ops.push_back(trace_op);
  }

#ifdef DEBUG
  cout << "The contents of the g_trace_ops vectors are :" << endl;
  for (vector<TraceOp>::iterator ii = g_trace_ops.begin();
      ii != g_trace_ops.end(); ii++) {
    PrintTraceOp(*ii);
  }
#endif // DEBUG

  ///////////////////////////////////////////////////////////////
  // Execute 
  ///////////////////////////////////////////////////////////////
  //
  g_scalar_registers[PC_IDX].int_value = 0;
  for (;;) {
    TraceOp current_op = g_trace_ops[g_scalar_registers[PC_IDX].int_value];
    int idx = ExecuteInstruction(current_op);
    g_current_pc = g_scalar_registers[PC_IDX].int_value; // debugging purpose only 
    if (current_op.opcode == OP_JSR || current_op.opcode == OP_JSRR)
      g_scalar_registers[LR_IDX].int_value = (g_scalar_registers[PC_IDX].int_value + 1) << 2 ;


    
    g_scalar_registers[PC_IDX].int_value += 1; 
    if (idx != -1) { // Branch
      if (current_op.opcode == OP_JMP || current_op.opcode == OP_JSRR) // Absolute addressing
        g_scalar_registers[PC_IDX].int_value = idx; 
      else // PC-relative addressing (OP_JSR || OP_BRXXX)
        g_scalar_registers[PC_IDX].int_value += idx; 
    }

#ifdef DEBUG
    g_instruction_count++;
    PrintContext(current_op);
#endif // DEBUG

    if (g_program_halt == 1) 
      break;
  }

  return 0;
}
