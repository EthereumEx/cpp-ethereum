/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http://www.gnu.org/licenses/>.
*/

namespace dev
{
namespace eth
{

///////////////////////////////////////////////////////////////////////////////
//
// interpreter configuration macros for optimizations and tracing
//
// EVM_SWITCH_DISPATCH - dispatch via loop and switch
// EVM_JUMP_DISPATCH - dispatch via a jump table - available only on GCC
//
// EVM_USE_CONSTANT_POOL - 256 constants unpacked and ready to assign to stack
//
// EVM_REPLACE_CONST_JUMP - with pre-verified jumps to save runtime lookup
//
// ETH_VMTRACE - set by cmake to enable tracing by testeth, ethvm and others
//
// EVM_TRACE - provides various levels of tracing that override ETH_VMTRACE

#if true
	#define EVM_SWITCH_DISPATCH
#elif defined(__GNUG__)
	#define EVM_JUMP_DISPATCH
#else
	#error Gnu C++ required for EVM_JUMP_DISPATCH
#endif

#if false
	#define EVM_REPLACE_CONST_JUMP
#endif

#if false
	#define EVM_USE_CONSTANT_POOL
#endif

#if	defined(EVM_USE_CONSTANT_POOL) || \
	defined(EVM_REPLACE_CONST_JUMP)
	
		#define EVM_DO_FIRST_PASS_OPTIMIZATION
#endif


#define ETH_VMTRACE 1 // turn on for now until others catch up
#if ETH_VMTRACE
	#define onOperation() doOnOperation()
#else
	#define onOperation()
#endif

// set this to 2, 1, or 0 for more, less, or no tracing to cerr
#define EVM_TRACE 0
#if EVM_TRACE

	#undef onOperation
	#if EVM_TRACE > 1
		#define onOperation() \
			(cerr <<"### "<< ++m_nSteps <<" @"<< m_pc <<" "<< instructionInfo(m_op).name <<endl)
	#else
		#define onOperation()
	#endif
	
	#define TRACE_OP(level, pc, op) \
		if ((level) <= EVM_TRACE) \
			cerr <<"*** "<< (pc) <<" "<< instructionInfo(op).name <<endl;
			
	#define TRACE_PRE_OPT(level, pc, op) \
		if ((level) <= EVM_TRACE) \
			cerr <<"@@@ "<< (pc) <<" "<< instructionInfo(op).name <<endl;
			
	#define TRACE_POST_OPT(level, pc, op) \
		if ((level) <= EVM_TRACE) \
			cerr <<"... "<< (pc) <<" "<< instructionInfo(op).name <<endl;
#else
	#define TRACE_OP(level, pc, op)
	#define TRACE_PRE_OPT(level, pc, op)
	#define TRACE_POST_OPT(level, pc, op)
#endif

// Executive swallows exceptions in some circumstances
#if 0
	#undef BOOST_THROW_EXCEPTION
	#define BOOST_THROW_EXCEPTION(X) \
		((cerr << "EVM EXCEPTION " << (X).what() << endl), abort())
#endif


#if defined(EVM_SWITCH_DISPATCH)

	// build a simple loop-and-switch interpreter

	#define INIT_CASES if (!m_caseInit) { m_caseInit = true; return; }
	#define DO_CASES for(;;) { fetchInstruction(); switch(m_op) {
	#define CASE_BEGIN(name) case Instruction::name:
	#define CASE_END break;
	#define CASE_RETURN return;
	#define CASE_DEFAULT default:
	#define END_CASES } }

#elif defined(EVM_JUMP_DISPATCH)

	// build an indirect-threaded interpreter using a jump table of
	// label addresses (a gcc extension)

	#define INIT_CASES  \
	\
		static const void * const jumpTable[256] =  \
		{  \
			&&STOP,          /* 00 */  \
			&&ADD,  \
			&&MUL,  \
			&&SUB,  \
			&&DIV,  \
			&&SDIV,  \
			&&MOD,  \
			&&SMOD,  \
			&&ADDMOD,  \
			&&MULMOD,  \
			&&EXP,  \
			&&SIGNEXTEND,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&LT,            /* 10, */  \
			&&GT,  \
			&&SLT,  \
			&&SGT,  \
			&&EQ,  \
			&&ISZERO,  \
			&&AND,  \
			&&OR,  \
			&&XOR,  \
			&&NOT,  \
			&&BYTE,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&SHA3,          /* 20, */  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&ADDRESS,       /* 30, */  \
			&&BALANCE,  \
			&&ORIGIN,  \
			&&CALLER,  \
			&&CALLVALUE,  \
			&&CALLDATALOAD,  \
			&&CALLDATASIZE,  \
			&&CALLDATACOPY,  \
			&&CODESIZE,  \
			&&CODECOPY,  \
			&&GASPRICE,  \
			&&EXTCODESIZE,  \
			&&EXTCODECOPY,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&BLOCKHASH,     /* 40, */  \
			&&COINBASE,  \
			&&TIMESTAMP,  \
			&&NUMBER,  \
			&&DIFFICULTY,  \
			&&GASLIMIT,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&POP,           /* 50, */  \
			&&MLOAD,  \
			&&MSTORE,  \
			&&MSTORE8,  \
			&&SLOAD,  \
			&&SSTORE,  \
			&&JUMP,  \
			&&JUMPI,  \
			&&PC,  \
			&&MSIZE,  \
			&&GAS,  \
			&&JUMPDEST,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&PUSH1,         /* 60, */  \
			&&PUSH2,  \
			&&PUSH3,  \
			&&PUSH4,  \
			&&PUSH5,  \
			&&PUSH6,  \
			&&PUSH7,  \
			&&PUSH8,  \
			&&PUSH9,  \
			&&PUSH10,  \
			&&PUSH11,  \
			&&PUSH12,  \
			&&PUSH13,  \
			&&PUSH14,  \
			&&PUSH15,  \
			&&PUSH16,  \
			&&PUSH17,         /* 70, */  \
			&&PUSH18,  \
			&&PUSH19,  \
			&&PUSH20,  \
			&&PUSH21,  \
			&&PUSH22,  \
			&&PUSH23,  \
			&&PUSH24,  \
			&&PUSH25,  \
			&&PUSH26,  \
			&&PUSH27,  \
			&&PUSH28,  \
			&&PUSH29,  \
			&&PUSH30,  \
			&&PUSH31,  \
			&&PUSH32,  \
			&&DUP1,          /* 80, */  \
			&&DUP2,  \
			&&DUP3,  \
			&&DUP4,  \
			&&DUP5,  \
			&&DUP6,  \
			&&DUP7,  \
			&&DUP8,  \
			&&DUP9,  \
			&&DUP10,  \
			&&DUP11,  \
			&&DUP12,  \
			&&DUP13,  \
			&&DUP14,  \
			&&DUP15,  \
			&&DUP16,  \
			&&SWAP1,         /* 90, */  \
			&&SWAP2,  \
			&&SWAP3,  \
			&&SWAP4,  \
			&&SWAP5,  \
			&&SWAP6,  \
			&&SWAP7,  \
			&&SWAP8,  \
			&&SWAP9,  \
			&&SWAP10,  \
			&&SWAP11,  \
			&&SWAP12,  \
			&&SWAP13,  \
			&&SWAP14,  \
			&&SWAP15,  \
			&&SWAP16,  \
			&&LOG0,          /* A0, */  \
			&&LOG1,  \
			&&LOG2,  \
			&&LOG3,  \
			&&LOG4,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&PUSHC,  \
			&&JUMPV,  \
			&&JUMPVI,  \
			&&BAD,  \
			&&INVALID,       /* B0, */  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,       /* C0, */  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,       /* D0, */  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,       /* E0, */  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&CREATE,        /* F0, */  \
			&&CALL,  \
			&&CALLCODE,  \
			&&RETURN,  \
			&&DELEGATECALL,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&INVALID,  \
			&&SUICIDE,  \
		};  \
		if (!m_caseInit) {  \
			c_jumpTable = jumpTable;  \
			c_invalid = &&INVALID; \
			m_caseInit = true;  \
			return;  \
		}

	#define DO_CASES fetchInstruction(); goto *jumpTable[(byte)m_op];
	#define CASE_BEGIN(label) label:
	#define CASE_END fetchInstruction(); goto *jumpTable[m_code[m_pc]];
	#define CASE_RETURN return;
	#define CASE_DEFAULT INVALID:
	#define END_CASES
	
#endif

}}
