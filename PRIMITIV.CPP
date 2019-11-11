// Copyright Andy Singleton, 1993,1994
// This code is released for non-commercial use only
// For questions or upgrades contact:
// Andy Singleton, Creation Mechanics Inc.
// PO Box 248, Peterborough, NH 03458
// Internet: p00396@psilink.com
// Compuserve 73313,757
// Phone: (603) 563-7757

//**************************************************************************
//  Standard Functions
//	includes ConstFunc (numeric constants)
//		ADD,SUB,MUL,DIV,SINE,ABS,ADD4,PROG4,BOOL,NOT,AND,OR,IF,IFlte
//**************************************************************************

#include "pch.h"
#include "chrome.h"
#include "primitiv.h"

extern char scratch[100];
#ifdef FASTEVAL
	extern evalnode* IpGlobal;
	extern Chrome* ChromeGlobal;
#endif

OPDEF(ConstEval) {return 127-GETIDX;};
// print the value, not the name
char * ConstFunc::getprint(Chrome* st) {sprintf(scratch,"%d",127-VARNUM(st->expr[st->ip]));return scratch;}

				// Add two arguments.  Note that argnum = 2
OPDEF(AddEval) {return EVAL + EVAL;}

				// Subtract two arguments
OPDEF(SubEval) {return EVAL - EVAL;}

				// Multiply two arguments
OPDEF(MulEval) {retval rval= EVAL * EVAL; return BOUNDF(rval);}

				// "Protected" division
OPDEF(DivEval)
{
	retval numerator,denominator,rval;
	numerator = EVAL;
	denominator = EVAL;
	if (denominator !=0)
		rval = numerator/denominator;
	else
		rval=1;
	return BOUNDF(rval);
}

// Return the Sine of one argument
OPDEF(SineEval) {return sin(EVAL);}

OPDEF(AbsEval)			// Absolute value
{
	retval arg = EVAL;
    return (arg > 0 ? arg : -arg);
}

OPDEF(Add4Eval) {return EVAL+EVAL+EVAL+EVAL;}

OPDEF(Prog4Eval) {EVAL;EVAL;EVAL;return EVAL;}

OPDEF(IfEval)           // Conditional. Thrown in for reference.
					// If(condition>0, dothis, otherwise dothat)
					// Conditionals, loops, subroutines come from
					// manipulating the instruction pointer
{
	retval rval;
	if (EVAL>0)
	{
		rval=EVAL;
		IP++;           // Jump the third expression
		TRAVERSE();
		IP--;           // And back up one (since EVAL increments)
	}
	else {
		IP++;
		TRAVERSE();    // Jump the second expression
		IP--;                                   // Back up for EVAL
		rval=EVAL;
	}
	return rval;
}

OPDEF(IflteEval)	//IfLTE(condition1islessthan,condition2,dothis,dothat)
{
	retval rval;
    rval=EVAL;
	if (rval<=EVAL)
	{
		rval=EVAL;
		IP++;           // Jump the third expression
		TRAVERSE();
		IP--;           // And back up one (since EVAL increments)
	}
	else {
		IP++;
		TRAVERSE();    // Jump the second expression
		IP--;                                   // Back up for EVAL
		rval=EVAL;
	}
	return rval;
}

OPDEF(BoolEval)	// Convert number to boolean value 0 or 1 - BOOL(arg)
{
	return (EVAL > 0 ? 1 : 0);
}

OPDEF(NotEval)		// NOT(arg)
{
	return (EVAL > 0 ? 0 : 1);
}

// watch out here - your compiler may optimize away an EVAL
OPDEF(AndEval)		// AND(arg1,arg2)
{
	retval arg1 = EVAL;
	retval arg2 = EVAL;
	return (arg1 > 0 && arg2 > 0 ? 1 : 0);
}

OPDEF(OrEval)		// OR(arg1,arg2)
{
	retval arg1 = EVAL;
	retval arg2 = EVAL;
	return (arg1 > 0 || arg2 > 0 ? 1 : 0);
}
