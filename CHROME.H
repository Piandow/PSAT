// Copyright Andy Singleton, 1993,1994
// This code is released for non-commercial use only
// For questions or upgrades contact:
// Andy Singleton, Creation Mechanics Inc.
// PO Box 248, Peterborough, NH 03458
// Internet: p00396@psilink.com
// Compuserve 73313,757
// Phone: (603) 563-7757

// GPQUICK
// C++ prefix stack implementation
// Divides GP system into classes:
//    Chrome-Function  - representation of the genetic program
//    Pop - Runs the GA
//    Problem - fitness and primitive functions


#ifndef _CHROME_H
#define _CHROME_H


#include "selector.h"

#include <ostream>
#include <istream>
using namespace std;
// uniform argument and return type for "closure"
typedef float retval ;

// Define this if you do multiple evals, one Chrome at a time
// It speeds up the EVAL calls significantly
// Penalties are:  Time to expand the chrome once before evaluations
//                 Uses globals, so only one Chrome at a time
#define FASTEVAL


#ifdef FASTEVAL
		// evaluation code with global pointers
	typedef retval (*EVALFUNC)();
#else
		// evaluation code with pointer passing
	typedef retval (*EVALFUNC)(class Chrome*);
#endif



	// two byte instruction node, 8 bits function index, 8 bits operand index
typedef struct {unsigned char op; unsigned char idx;} node;  // node type
	// eval node with pointers for direct function calls
typedef struct {EVALFUNC ef; unsigned char op; unsigned char idx;} evalnode;  // node type

	// Grab the function index
#define FUNCNUM(c) (c.op)
	// argument count for the current function
#define ARGNUM() (funclist[FUNCNUM(expr[ip])]->argnum)
#define PARGNUM(ip) (funclist[ip->op]->argnum)
	// Grab the operand
#define VARNUM(c) (c.idx)
#define PVARNUM(ip) (ip->idx)
	// Build a node "c"
#define SETNODE(c,o,v) c.op=o;c.idx=v
#define SETEVALNODE(ip,o,v) ip->op=o;ip->idx=v;ip->ef=funclist[o]->evalfunc


	// Function evaluation stuff
	// These macros may change internal form for faster evaluation.
	// Arguments will be removed.  Use them.
#ifdef FASTEVAL

		//Define an EVALFUNC with no arguments
#define OPDEF(Op) retval Op()
		//Get a pointer to the chrome being evaluated
#define CHROMEP ChromeGlobal
		//current instruction pointer
#define IP IpGlobal
		// get its argument
#define GETIDX (IP->idx)
		// traverse an unused argument in an eval
#define TRAVERSE() CHROMEP->TraverseGlobal()
		//Evaluate the next expression
#define EVAL ((++IP)->ef)()
#else
		//Define an EVALFUNC
#define OPDEF(Op) retval Op(Chrome* curchrome)
		//Get a pointer to the chrome being evaluated
#define CHROMEP curchrome
		//current instruction pointer
#define IP CHROMEP->ip
		// get its argument
#define GETIDX (CHROMEP->expr[IP].idx)
		// traverse an unused argument in an eval
#define TRAVERSE() CHROMEP->Traverse()
		//Evaluate the next expression
#define EVAL CHROMEP->eval()
#endif

		//function and memory arrays
#define FUNCARRAYSIZE 100
#define CONSTARRAYSIZE 256

// cut numeric overflows.  Bound returns to 10^15
#define BIGFLOAT ( (retval) 1.0e15 )
#define SMALLFLOAT ( (retval) 1.0e-15 )

#define BOUNDF(f) (f==0? f : (f>0 ?((f)>BIGFLOAT ? BIGFLOAT : ((f)<SMALLFLOAT? SMALLFLOAT : (f))) : ((f)<-BIGFLOAT ? -BIGFLOAT : ((f)>-SMALLFLOAT? -SMALLFLOAT : (f))) ))


// Compatibility stuff
#define BOOL int
#define UINT unsigned int



// All primitives in a problem are subclasses of this FUNCTION object

class Function {
public:
	int serial;             // serial number in universal function list (not implemented)
	char name[30];                  // Function name
	int argnum;             // number of arguments
	int varnum;                             // number of variables in variable table of opcode
	int weight;             // selection frequency relative to other functions
	Function() {};
	Function(int a,int v,int w,EVALFUNC e,char* n)
		{argnum=a;varnum=v;weight=w;evalfunc=e;strcpy(name,n);};
	virtual char* getprint(class Chrome* st);       // Printed name may differ
	char* getname() {return name;};
	EVALFUNC evalfunc;                      // pointer to evaluation code.  Not a virtual for speed reasons
#ifndef FASTEVAL
	retval eval(class Chrome* st) {return (evalfunc)(st);};       // active ingredient
#else
	retval eval() {return (evalfunc)();};
#endif
};

//******************* Parameters
enum {
pMaxExpr,          // Maximum expression size in nodes
pInitExpr,           // Maximum initial expression depth

pMuteRate,          // Node mutation rate per 1000
pCrossSelf,         // Allow self crossover?
pUnRestrictWt,         // any point crossover per 100

pCrossWt,         // Crossover weight on generate
pMuteWt,         // overall Mutation weight on generate
pMuteNodeWt,         // Normal-Mutation weight on generate
pMuteConstWt,         // C-Mutation weight on generate
pMuteShrinkWt,         // Shrink-Mutation weight on generate
pAnnealMuteWt,                 // Mut. Annealing weight
pAnnealCrossWt,         // Crossover Annealing weight
pCopyWt,         // Copy weight on generate

pSelectMethod,         // can be tournament, ranked, proportional
pTournSize,         // tournament size 2-10
pMateRadius,         // mating radius.  0 for panmictic
pGaussRegion,         // 0 for flat selection in region, 1 for gaussian
pRepeatEval,         // repeat evaluation on copy?  For sampled evals

pKillTourn,         // number for anti-tournament
pMaxAge,         // age to start killing off a good guy
pParsimony,

pFitnessCases,     // number of fitness cases
pPopSize,         // population size
PARAM_COUNT }; // total number of parameters

class ChromeParams {    // parameter set for initializing a chrome
public:
	int params[PARAM_COUNT];
	static char *pnames[];                  // names are added in the constructor
	int varcount;
	int funccount;

	ChromeParams();
	virtual ~ChromeParams();
	virtual void Edit() {};                 // Put your own edit code here

};

// Carrier for fitness values.
// Feel free to subclass this if you want to track your fitness cases more closely (by adding data)
// or minimize instead of maximize (by redefining IsBetter)
// or combine multiple scores
class FitnessValue {
public:
	float fvalue;			// Always provide this for reporting reasons
	virtual BOOL IsBetter(FitnessValue* fv) {return fvalue>fv->fvalue;};
	virtual BOOL IsBetter(float fv) {return fvalue>fv;};
	virtual void Copy(FitnessValue* fv) {memcpy(this,fv,sizeof(FitnessValue));};
	operator float() {return fvalue;};
	operator >(FitnessValue& fv) {return IsBetter(&fv);};
	operator <(FitnessValue& fv) {return fv.IsBetter(this);};
};

#define EXPRLEN 1000             // maximum length of expression
enum {LOAD_OK,LOAD_BADFILE,LOAD_BADFUNC,LOAD_TOOFEW,LOAD_TOOMANY,LOAD_BADCONST,LOAD_TOOLONG};   // subload error values
enum {PRETTY_NONE,PRETTY_PARENS,PRETTY_NOPARENS};       // source listing options



// The Chrome is the carrier for a program expression
// It includes the eval, initialization, reading, mutation, crossover, reading and writing

class Chrome     {               // GP program structure
public:
	int ip;                          // Current instruction pointer
	node *expr;                                     // actual code
	int MaxExpr;                                    // expr size
	int ExprBytes;                                  // bytes in expr (MaxExpr * sizeof(node))
	//float lastfitness;                              // fitness on last eval
	INT32 birth;                                             // gencount when generated
	int depth;                                              // depth counter for recursive write

    class Problem* probl;
	CSelector* funcbag;                              // select functions by weight
    class FitnessValue* nfitness;                       // pointer to an allocated object holding the type of fitness
	Function** funclist;            // array of pointers to functions and operations
	int funccount;
	ChromeParams* params;
	retval* constlist;              // array of constants

	Chrome(ChromeParams* p,CSelector* cf,Problem* pr,Function** f,retval* c,BOOL doinit=TRUE);     // returns a randomly initialized condition
	virtual Chrome* Copy();                 // copy yourself.  Extend for subclasses, and call base.
	virtual void Dup(Chrome* source);  // make a copy without re-initializing.  Extend for subclass, and call base
	virtual ~Chrome();
	void SubInit(int argstogo,int maxlen,int full=FALSE);     // Initialize a subtree half full
	BOOL IsBetter(Chrome* chrome) {return nfitness->IsBetter(chrome->nfitness);};

#ifndef FASTEVAL
	retval eval() {ip++;return funclist[FUNCNUM(expr[ip])]->eval(this);} ;
#endif
	retval evalAll() ; // eval the whole expression anew
	void SetupEval();                         // expand expression for multiple evals
	void Traverse();              // Skips over next subtree
	void TraverseGlobal();

	Chrome* CrossTree(Chrome* mate);
	int GetIntNode();                                       // get an internal node for crossover
	int GetAnyNode();                               // get any node for crossover
	void Mutate();                                  // mutate nodes with rate r
	void MutateC();
	void MutateShrink();
	int SubLen(int startat=0)
	 {ip=startat;Traverse();return ip-startat;};    // return length of expression at startat

	void write(int pretty,ostream& ofile = cout)
	 {ip=-1;depth=0;SubWrite(ofile,pretty);};       // write the expression
	void SubWrite(ostream& ostr,int pretty = 0);
	int SubLoad(istream& istr,node* buf);   // load an expression.  Return 0 on success or error val
	int FindFunc(char* funcname);                   // find index of a function, or -1 if not found

	virtual int Load(istream& istr,int issource); // load from a stream.  Return success
};

class Problem {         // Sets up a particular GP problem.  GA independent.
					// contains function set, variables, and fitness function
protected:
	CSelector* funcbag;                              // select functions by weight
	retval* constlist;              // array of constants
	retval* varlist;                // array of variables
public:
	Function** funclist;                    // primitive functions

public:
	int funccount;

	Problem();     // allocate the arrays
	virtual ~Problem();
	void AddF(Function* f)          // add a function to the problem
	 {funclist[funccount++]=f;};
	Function** getfuncs() {return funclist;};
	retval* getconsts() {return constlist;};
	CSelector* getfuncbag() ;

	// User hooks
	virtual FitnessValue* GetFitnessObj();	// Can change behavior of FitnessValue
	virtual float fitness(Chrome* chrome);  // The active ingredient.  You add this.
};



class Pop {                     // a population for a particular GA.  Problem independent
					// controls the GA - selection, crossover, etc
public:
	Chrome** pop;           // allocated array holding the actual chromes
	ChromeParams* params;
	Problem* problem;
	UINT popsize;           // size of array
	INT32 gencount;          // number of individuals generated
	time_t start;           // starting time
	time_t elapsed;         // elapsed time for last go_until
	BOOL isAborted;

	FitnessValue* BestFitness;      // Best current fitness value
	int   BestMember;       // Index of best current member
	int initeval;           // Flag: Still evaluating initial population?
	UINT nexteval;          // Next initial pop member to evaluate

	Pop(Problem* prob,ChromeParams* par, UINT size );       // set up a population for a particular problem
	Pop(Problem* prob,ChromeParams* par);   // stub for derived classes
	virtual ~Pop();
    Chrome* best();                         // return the current best chrome
	virtual Chrome* selectParent(int target); // select one parent. Return pointer to selected parent
						 // target identifies selection region in pop
	virtual Chrome* generate();     // generate a new chrome.  Return fitness
								// Alternate steady state GA code can go here
						// generate until reaching time max_time, evaluating maxevals individuals, or reaching maxfitness
	virtual Chrome* go_until(time_t max_time, int maxevals, float maxfitness);
	int GetTarget();                // get a target member to replace with anti-tournament
	int GetAnnealTarget();                  // get a target for annealing, with selection tournament
	virtual BOOL Aborted(){return isAborted;};
	virtual float Fitness(Chrome* chrome);   // Evaluate a member using the current problem
	void InsertMember(int slot,Chrome* NewChrome,int nodelete = FALSE);                     //insert a chrome and update BestMember values
};

// Function to call if there is an error allocating population
void NoMoreMem();

#endif          // #ifndef _CHROME_H
////////////////////////////////////////////////////////////
