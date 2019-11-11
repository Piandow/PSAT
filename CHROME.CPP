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


#include "pch.h"
#include "chrome.h"

// Global variables
char scratch[100];

#ifdef FASTEVAL
	// Global evaluation variables.  Eliminates parameter passing
	// If you can get IpGlobal into a register, you will run near the speed
	// limit for S-expression evaluation
	Chrome * ChromeGlobal;                  // currently evaluating Chrome
	evalnode ExprGlobal[EXPRLEN];   // expression expanded to evalnodes
	evalnode * IpGlobal;            // Current instruction pointer
#endif


//**************************************************************************
//////////////////////////////////// Utility functions

void NoMoreMem()		// Out of memory exit
{
	cout << "\nOut of Memory.  Aborting Execution.\n";
	exit(1);
}

int min(int value1, int value2)
   {
	  return ( (value1 < value2) ? value1 : value2);
   };
int max(int value1, int value2)
   {
	  return ( (value1 > value2) ? value1 : value2);
   };

// Comment out if included in your implementation (e.g. Borland)
	// upper case string
char* strupr(char* s)
{
	int i=0;
	while (s[i])
		if (s[i]>='a' && s[i]<='z') s[i]=s[i]-32;
    i++;
	return s;
}

	// compare strings without case sensitivity
int strcmpi(char* s1, char* s2)         // only checks equality
{
	int mismatch=0;
	while (*s1 && *s2 && !mismatch)
	{
		if (*s1>='a' && *s1<='z' && *s2<='Z')
			mismatch = *s1-32 != *s2;
		else if (*s1>='A' && *s1<='Z' && *s2>='a')
		mismatch = *s1+32 !=*s2;

		else
			mismatch = *s1!=*s2;
		s1++;s2++;
	}
	return mismatch || *s1 || *s2;
}


//**************************************************************************
//////////////Function member function (separated for use in a DLL)
char* Function::getprint(Chrome* chrome)
{
	// NUMBER has its own getprint to write the correct number
	// This is fancy footwork for other functions with operands
	// these are written as <name>_<operand number>
	if (varnum) sprintf(scratch,"%s_%-d",name,VARNUM(chrome->expr[chrome->ip]));
	return (varnum? scratch : name);
}



//**************************************************************************
/// Chrome functions ///////////////////////////////////////////

Chrome::Chrome(ChromeParams* p,CSelector* cf,Problem* pr,Function** f,retval* c,BOOL doinit) {
// initialize a expr using the functions in array f

	int depth = p->params[pInitExpr];
	funcbag=cf;
		params = p;
	probl = pr;
	funclist = f;
	constlist = c;
	MaxExpr=params->params[pMaxExpr];
	ExprBytes=MaxExpr*sizeof(node);
	expr=new node[MaxExpr];
    if (expr==NULL) NoMoreMem();		// Out of memory?
	ip=0;
	funccount = params->funccount;          // how many primitives?

	birth = 0;
	if (doinit)
	{
		// DO "ramped half and half from 2 to depth"
		if (depth > 0)
		{
						SubInit(1,rnd(depth-1)+1,rnd(2));       // go to some depth less than max, full or half full
		}
		else
			SETNODE(expr[ip],0,0);                                  // stick in a stub constant
	}
		nfitness  = probl->GetFitnessObj();
		// allocates a FitnessValue object;  GetFitnessObj  calls new
}

Chrome* Chrome::Copy()
{
	Chrome* nw=new Chrome(params,funcbag,probl,funclist,constlist,FALSE);
	if (nw==NULL) NoMoreMem();		// Out of memory?
	nw->Dup(this);
	return nw;
}

void Chrome::Dup(Chrome* source)                // make a copy nondestructively
{
	funcbag=source->funcbag;
	params=source->params;
	funclist=source->funclist;
	funccount=source->funccount;
		constlist=source->constlist;

		memcpy(expr,source->expr,ExprBytes);
	nfitness->Copy(source->nfitness);
	ip=0;
}

retval Chrome::evalAll() // eval the whole expression anew
{
#ifndef FASTEVAL
	ip=-1;
	return eval();
#else

	IP=ExprGlobal;
	//cout<<"FUNC="<< IP->op;
	return (IP->ef)();
#endif
}

void Chrome::SetupEval()
{
#ifdef FASTEVAL
		// expand stack into evalnodes in global eval stack
	int args;
	node* spp=expr;
	evalnode* ip=ExprGlobal;
	args=1;
	while (args > 0)
	{
		SETEVALNODE(ip,spp->op,spp->idx);
		args=args+PARGNUM(spp)-1;
		ip++;
		spp++;
	}
		// set global eval pointers
	ChromeGlobal=this;
#endif
}

void Chrome::SubInit(int argstogo,int depth,int isfull)     // Initialize a subtree half full
{
	int i;
	int maxargs,thisargs;
	maxargs = MaxExpr -(ip+argstogo);       // max number of args to add
	i=funcbag->roulette();

	// terminal required
	if (maxargs == 0 || depth == 0 )
		while (funclist[i]->argnum>0) i=funcbag->roulette();

	// node required
	else if (isfull || ip==0)
		if (maxargs > 5)
			while (funclist[i]->argnum == 0) i=funcbag->roulette();
		else                    // not enough room.  Take what you can get.
			while (funclist[i]->argnum>maxargs) i=funcbag->roulette();

	// terminal allowed 50% of the time
	else
		if (rnd(2))             // terminal required
			while (funclist[i]->argnum>0) i=funcbag->roulette();
		else            // node required
			if (maxargs > 5)
				while (funclist[i]->argnum == 0) i=funcbag->roulette();
			else                    // not enough room.  Take what you can get.
				while (funclist[i]->argnum>maxargs) i=funcbag->roulette();

	SETNODE(expr[ip],i,rnd(funclist[i]->varnum));
	ip++;
	thisargs=funclist[i]->argnum;
	argstogo += funclist[i]->argnum-1;

	for (i=0;i<thisargs;i++)
	{
		SubInit(argstogo,depth-1,isfull);
		argstogo--;
	}
}

void Chrome::Traverse() {             // skip the next expression
	int args = 1;
	while (args > 0)
	{
				 args=args+ARGNUM()-1;
				 ip++;
	}
}


void Chrome::TraverseGlobal() {             // skip the next expression
	int args = 1;
	while (args > 0)
	{
				 args=args+PARGNUM(IP)-1;
				 IP++;
	}
}

				// Write the next subexpression to a stream
				// pretty flag controls parentheses, indenting
				// PRETTY_NONE = no indenting, just a stream, with parens
				// PRETTY_NOPARENS = indented, one function per line, no parens
				// PRETTY_PARENS = indented, one function per line, parens
void Chrome::SubWrite(ostream& ostr,int pretty) {
	int args,i;
	ip++;
	args = ARGNUM();
	if (pretty)                                     // indent?
	{
		ostr<< '\r' << '\n';
		for (i=0;i<depth;i++){
				ostr << " : ";
		}
	}
	else
		ostr << ' ';
	if (args>0)
	{                               // write (FUNC args)
		if (pretty != PRETTY_NOPARENS)
			ostr << '(';
		ostr << funclist[FUNCNUM(expr[ip])]->getprint(this);
		depth++;
		while (args-- > 0) SubWrite(ostr,pretty);
		depth--;
		if (pretty != PRETTY_NOPARENS)
			ostr << ")";
	}
	else {                          // write an atom
		ostr << funclist[FUNCNUM(expr[ip])]->getprint(this);
	}
}

Chrome* Chrome::CrossTree(Chrome* mate)
// Return a new Chrome that is a cross with mate

{
		Chrome* newexpr = Copy();
	int thislen;        // total length of expression
	int thissubptr;     // pointer to subexpression
	int thissublen;
	int matelen;
	int matesubptr;
	int matesublen;
	thislen=SubLen(0);
	matelen=mate->SubLen(0);
	if (rnd(101)>params->params[pUnRestrictWt])
	{
		thissubptr=GetIntNode();
		matesubptr=mate->GetIntNode();
	}
	else
    {
		thissubptr=GetAnyNode();
		matesubptr=mate->GetAnyNode();
	}
	thissublen = SubLen(thissubptr);
	matesublen=mate->SubLen(matesubptr);

	if (thislen+matesublen-thissublen > MaxExpr){      // take smaller side of swap
		memcpy(newexpr->expr,mate->expr,matesubptr*sizeof(node));
		memcpy(&(newexpr->expr[matesubptr]),&(expr[thissubptr]),thissublen*sizeof(node));
		memcpy(&(newexpr->expr[matesubptr+thissublen]),&(mate->expr[matesubptr+matesublen]),(matelen-(matesubptr+matesublen))*sizeof(node));
	}
	else {
		memcpy(newexpr->expr,expr,thissubptr*sizeof(node));
		memcpy(&(newexpr->expr[thissubptr]),&(mate->expr[matesubptr]),matesublen*sizeof(node));
		memcpy(&(newexpr->expr[thissubptr+matesublen]),&(expr[thissubptr+thissublen]),(thislen-(thissubptr+thissublen))*sizeof(node));
	}
	return newexpr;
}

int Chrome::GetAnyNode()                // get a node for crossover
{
	return rnd(SubLen(0));
}

int Chrome::GetIntNode()                // get an internal node for crossover
{
	int rval;
	int l=SubLen(0);
	if (l>2)
	{
		rval=rnd(l);
		while (funclist[FUNCNUM(expr[rval])]->argnum <1)
			rval=rnd(l);
	}
	else
		rval=0;
	return rval;
}

				// Mutate the current Chrome
void Chrome::Mutate()                   // mutate nodes with rate r
{
		int end,i,args;
	int rate=params->params[pMuteRate];
	ip=0;
	Traverse();
	end=ip;
	ip=0;
	while (ip<end)
	{
		if (rnd(1024) < rate)
		{
			args=ARGNUM();
			i=funcbag->roulette();
			while (funclist[i]->argnum!=args) i=funcbag->roulette();
			SETNODE(expr[ip],i,rnd(funclist[i]->varnum));
		}
		ip++;
	}
}


void Chrome::MutateC()                  // jiggle constants with a random walk
{
	int i,end,newconst;
	int oldconst;
	int radius=8;
	ip= 0;
	Traverse();
	end=ip;
	ip=0;
	while (ip<end)
	{
				if (FUNCNUM(expr[ip])==0)
		{
			oldconst=VARNUM(expr[ip]);
			newconst=oldconst;
			for (i=0;i<radius;i++)
				newconst+=rnd(2);
			newconst-=radius/2;
			newconst=min(CONSTARRAYSIZE-1,max(newconst,0));
			if ((constlist[oldconst]>100 && constlist[newconst]<0) || (constlist[oldconst]<-100 && constlist[newconst]>0))  // overrun?
		newconst=oldconst;
			SETNODE(expr[ip],0,newconst);
		}
	ip++;
    }
}


void Chrome::MutateShrink()             // shrink by promoting a subtree
{
	int tree,treelen,subtree,subtreelen,l;
	node *temp;
	int argnum;
	argnum=0;
	l=SubLen(0);
	if (l>argnum+1)
    {
		// node required
		temp = new node[MaxExpr];
		if (temp==NULL) NoMoreMem();		// Out of memory?
		tree=rnd(l);
		while (funclist[FUNCNUM(expr[tree])]->argnum == 0) tree=rnd(l);

		// now pick a subtree
		treelen=SubLen(tree);
		subtree=tree+rnd(treelen-1)+1;
	subtreelen=SubLen(subtree);

		memcpy(temp,expr,l*sizeof(node));               // save the old expr
		memcpy(expr+tree,temp+subtree,subtreelen*sizeof(node)); // add the subtree
		memcpy(expr+tree+subtreelen,temp+tree+treelen,sizeof(node)*(l-(tree+treelen)));         // add the rest
		delete[] temp;
    }
}

int Chrome::Load(istream& ifile,int issource)
// Load an expression from a stream
// issource should always be TRUE

{
	int x,tempop, tempidx;
	int rval=LOAD_OK;
	node* buf;

	if (!issource)
	{
		for(x=0;x<MaxExpr;x++)
		{
			ifile >> tempop;
			expr[x].op =(unsigned) tempop;
			ifile >> tempidx;
			expr[x].idx=(unsigned) tempidx;
		}
	}
	else            // load a Source file
	{
		ip=0;
		buf=new node[MaxExpr];
		if (buf==NULL) NoMoreMem();		// Out of memory?
		if ((rval=SubLoad(ifile,buf)) == LOAD_OK)
		{
			delete[] expr;
			expr=buf;
		} else
			delete[] buf;
	}
	return rval;
}

#define EATSPACE c=istr.peek();while(isspace(c)||c==':') {istr.get();c=istr.peek();}
#define GETTOK tp=0;c=istr.peek();while(c!=EOF && !isspace(c) && c!=':' && c!='(' && c!=')' &&tp<79) {scratch[tp++]=istr.get(); c=istr.peek();} scratch[tp]='\0'

int Chrome::SubLoad(istream& istr,node* buf)
// load a sub-expression from source format
// Return LOAD_OK or an error value
// text expression is in istr
// nodes go into buf

{
	int rval = LOAD_OK;
	char c;
//      char token[80];
	int tp;
	int func;
	int args;
	int vnum;
	char* s;
	scratch[0]='\0';
	EATSPACE;
	if (istr.peek()==')' || istr.peek()==EOF)
	{
		rval=LOAD_TOOFEW;                       // missing expression body
	}
	else if (ip>=MaxExpr)
	{
		rval=LOAD_TOOLONG;
	}
	else if (istr.peek()=='(')              // Get an expression and the close paren
	{
		istr >> c;
		rval = SubLoad(istr,buf);
		if (rval==LOAD_OK)
	{
			EATSPACE;
			istr >> c;
			if (c!=')')
				rval=LOAD_TOOMANY;
	}
	}
	else                            // Get an expression.  Return if you hit a close paren.
	{
		GETTOK;
		if (strlen(scratch)==0)
			rval=LOAD_TOOFEW;
		else
		{
			if (isdigit(scratch[0]) || scratch[0]=='-')     // it's a number.  Function 0
			{
				func=atoi(scratch);
				if (func>=0-CONSTARRAYSIZE/2 && func<CONSTARRAYSIZE/2)
				{
					SETNODE(buf[ip],0,func+CONSTARRAYSIZE/2);
					ip++;
					rval=LOAD_OK;
				}
				else rval=LOAD_BADCONST;
			}
			else       // look up a function name
			{
				vnum=0;
				if (strchr(scratch,'_')!=NULL)
				{
					// parse it to take off the variable number?
					// This is fancy footwork for functions with operands
					// Except for NUMBER (function 0) these functions are
					// written as <name>_operand
					s=strrchr(scratch,'_');
					if (strchr("0123456789",s[1]))  // it is an underscore followed by numbers
					{
						vnum=atoi(s+1);
						if (vnum>=0 && vnum <= CONSTARRAYSIZE)
						{
							s[0]='\0';              // found a valid function with variable number appended.  Keep function name.
						}
						else
				vnum=0;
					}
				}
				func=FindFunc(scratch);
				if (func<0)
					rval=LOAD_BADFUNC;
				else
				{
					SETNODE(buf[ip],func,vnum);
					ip++;
		    rval=LOAD_OK;
					// get the arguments
		    args=funclist[func]->argnum;
					while(args>0 && rval==LOAD_OK)
		    {
						rval=SubLoad(istr,buf);
						args--;
					}
//                                      if (rval == LOAD_TOOFEW)
//                      ;               // restore the token for the error message
				}
			}
		}
	}
	return rval;
}


//**************************************************************************

int Chrome::FindFunc(char* funcname)            // find a function index by name, or return -1
{
	int rval=-1;
	int i;
	for (i=0;i<funccount && rval<0;i++)
		if (strcmpi(funcname,funclist[i]->name)==0)
			rval=i;
	return rval;
}

Chrome::~Chrome()
{
		delete[] expr;
	delete nfitness;
}

//**************************************************************************
//// Generic Problem Functions /////////////////////////////////////

Problem::Problem()
// Set up the tables
// You add the primitive functions in your subclass constructor

{
	int i;

	funclist = new Function*[FUNCARRAYSIZE];
		funcbag = new CSelector(EStraight,256);
	varlist = new retval[CONSTARRAYSIZE];
	constlist = new retval[CONSTARRAYSIZE];
	funccount = 0;
	// set up constant table (not implemented)
	for (i=0;i<CONSTARRAYSIZE;i++) constlist[i]=i-(CONSTARRAYSIZE/2);
}

Problem::~Problem()
{
	int i;
	delete[] constlist;
	delete[] varlist;

	for (i=0;i<funccount;i++) delete funclist[i];
    delete funcbag;
	delete[] funclist;
}

FitnessValue* Problem::GetFitnessObj()
{
	FitnessValue* fv = new FitnessValue;
	if (fv==NULL) NoMoreMem();		// Out of memory?
	fv->fvalue=0;
    // fv has to be deleted by the Chrome Object
    return fv;
}

CSelector* Problem::getfuncbag()         // update the function selector and return its pointer
{
	int i;
	funcbag->reset();
	for (i=0;i<funccount;i++)
		funcbag->add(funclist[i]->weight/(2+funclist[i]->argnum),i);
	return funcbag;
}

float Problem::fitness(Chrome* chrome)
// This is just a stub.  You must add your own virtual fitness function

{
	float f=0;
	// clear variables if required
	// call the installed fitness function
	chrome->nfitness->fvalue = f;
	return f;
}



//**************************************************************************
//// Pop Functions /////////////////////////////////////////////////

Pop::Pop(Problem* prob,ChromeParams* par,UINT size)
// set up a population for a particular problem
// Creates <size> new Chromes
// evaluates the first Chrome.  The rest will be evaluated in the first
// Size-1 calls to generate

{
	UINT i;
		CSelector* fbag = prob->getfuncbag();
	isAborted=FALSE;
	problem = prob;
	gencount = 0;
	start=time(NULL);
		BestMember=0;
	BestFitness=NULL;

	params=par;
	par->funccount = prob->funccount;
	if (par->params[pMaxExpr] > EXPRLEN)
		par->params[pMaxExpr]=EXPRLEN;

	popsize=size;
	pop = new Chrome*[size];
	if (pop==NULL) NoMoreMem();		// Out of memory?
	for (i=0;i<size;i++)
	{
		pop[i]=new Chrome(params,fbag,prob,prob->getfuncs(),prob->getconsts());
		if (pop[i]==NULL) NoMoreMem();		// Out of memory?
	}
	// now eval 1 guy
	initeval=TRUE;
	nexteval=1;
	Fitness(pop[0]);
	InsertMember(0,pop[0],TRUE);
}

float Pop::Fitness(Chrome* chrome)
// uses the problem to evalue the fitness of member X
// performs setup on the Chrome
// updates the BestMember and returns the fitness value

{
	chrome->SetupEval();
	return problem->fitness(chrome);
}


void Pop::InsertMember(int slot,Chrome* NewChrome,int nodelete)
// uses the problem to evalue the fitness of member X
// performs setup on the Chrome
// updates the BestMember and returns the fitness value

{
	UINT endpop;


	if (!nodelete)
		 {
	// replace the chrome in this slot
	    delete pop[slot];
		pop[slot]=NewChrome;
	 }
	 // Update Best Member
		if (BestFitness==NULL)
		{
			BestMember=slot;
			BestFitness=NewChrome->nfitness;
	}
	else if(NewChrome->nfitness->IsBetter(BestFitness))
	{
		BestMember=slot;
		BestFitness=NewChrome->nfitness;
	}
	else if(slot==BestMember)
	{
		endpop = (initeval? nexteval : popsize);
		BestMember=0;
		BestFitness=pop[0]->nfitness;
		UINT i;
		for (i=1;i<endpop;i++)
		{
			if (pop[i]->nfitness->IsBetter(BestFitness))
			{
				BestMember = i;
				BestFitness=pop[i]->nfitness;
			}
		}

	}
	NewChrome->birth=gencount;
}


Pop::Pop(Problem* prob,ChromeParams* par)
// set up just the core of a population;; let a subclass allocate the members
{
	isAborted=FALSE;
	problem = prob;
	gencount = 0;
	params=par;
	popsize=0;
	pop=NULL;
		BestMember=-1;
		BestFitness=prob->GetFitnessObj();    // allocates FitnessValue object on heap

	BestFitness->fvalue=1-FLT_MAX;
}


Pop::~Pop()
{
		int i;
		for (i=0;i<popsize;i++) delete pop[i];
	delete[] pop;
}

Chrome* Pop::best()
{
	return pop[BestMember];
}

enum {docross,domutate,doanneal,docrossanneal,docopy};

Chrome* Pop::selectParent(int target)
{                                                                               // target identifies selection region in pop
// select one parent. Return pointer to selected parent
	int region, ts, offset, i;
	Chrome* best;
	Chrome* trial;

	region = params->params[pMateRadius];
	ts = params->params[pTournSize];
	// Only tournament selection is implemented in GPQUICK.
	// Add your own methods to taste
	if (params->params[pSelectMethod] == ETournament)
	{
		if (region == 0 || region > popsize)
			region = popsize;
		offset=popsize+target-region/2;
		best=pop[(rnd(region)+offset)%popsize];
	for (i=1;i<ts;i++)
		{
			trial=pop[(rnd(region)+offset)%popsize];
			if (trial->nfitness->IsBetter(best->nfitness))
				best=trial;
		}
	}
    return best;
}

Chrome* Pop::generate()
// generate a single new chrome.  Return fitness.
// This implements a one processor, steady stage GA
// Its reproductive operators are Copy, Subtree Crossover, and Node Mutation
// It uses tournament selection to select parents
// It selects in a one dimensional local region
// It uses global tournament anti-selection to select a member for replacement

// Virtual - add your own GA here if desired

{
	int target;
	int i,region,ts,offset;
	int newchrome = FALSE;
	int prob,wheel;
	int dowhat;

	Chrome* best;
	Chrome* secondbest;
    Chrome* trial;



	gencount++;

  if (initeval)                 // still on initial evaluations?
							// don't generate. Finish evaluating initial pop.
	{
	  Fitness(pop[nexteval]);
      InsertMember(nexteval,pop[nexteval],TRUE);
	  target=nexteval;
	  nexteval++;
	  if (nexteval>=popsize)
		initeval=FALSE;
  }
  else
  {
	// decide what to do - Cross, Mutate, Copy
		prob=rnd(params->params[pCrossWt]+params->params[pMuteWt]+params->params[pCopyWt]+params->params[pAnnealMuteWt]+params->params[pAnnealCrossWt]);
	wheel=params->params[pCrossWt];
	if (prob<wheel)
		dowhat=docross;
	else if (prob<(wheel += params->params[pMuteWt]))
		dowhat=domutate;
	else if (prob<(wheel += params->params[pCopyWt]))
		dowhat=docopy;
	else if (prob<(wheel += params->params[pAnnealMuteWt]))
	dowhat=doanneal;
	else
		dowhat=docrossanneal;

	// Perform reproduction
	switch (dowhat)
	{
	case docross:
	target=GetTarget();                     // Find a member to replace
		best=selectParent(target);
		secondbest=selectParent(target);
		trial=best->CrossTree(secondbest);
		newchrome = TRUE;
		break;
	case domutate:
		target=GetTarget();                     // Find a member to replace
	    best=selectParent(target);
		trial = best->Copy();
				prob=rnd(params->params[pMuteNodeWt]+params->params[pMuteConstWt]+params->params[pMuteShrinkWt]);
				wheel=params->params[pMuteNodeWt];
		if (prob<wheel)
	    trial->Mutate();
				else if (prob<(wheel += params->params[pMuteConstWt]))
			trial->MutateC();
		else
			trial->MutateShrink();
		newchrome = TRUE;
		break;
	case doanneal:
		target=GetAnnealTarget();
		trial = pop[target]->Copy();
				prob=rnd(params->params[pMuteNodeWt]+params->params[pMuteConstWt]+params->params[pMuteShrinkWt]);
		wheel=params->params[pMuteNodeWt];
		if (prob<wheel)
	    trial->Mutate();
		else if (prob<(wheel += params->params[pMuteConstWt]))
			trial->MutateC();
		else
			trial->MutateShrink();
	Fitness(trial);
		// test whether mutated chome is fitter
				if (trial->nfitness->IsBetter(pop[target]->nfitness))
		{
		InsertMember(target,trial);
			newchrome = TRUE;
		}
		else
			delete trial;
		break;
    case docrossanneal:
		target=GetAnnealTarget();
		best=selectParent(target);
		trial=pop[target]->CrossTree(best);
		Fitness(trial);
		// test whether chome is fitter
		if (trial->nfitness->IsBetter(pop[target]->nfitness))
		{
		InsertMember(target,trial);
			newchrome = TRUE;
		}
		else
			delete trial;
		break;

	case docopy:
		target=GetTarget();                     // Find a member to replace
	    best=selectParent(target);
		trial=best->Copy();
		newchrome = (params->params[pRepeatEval]? TRUE: FALSE);
	}

    // Update the pop array
	if ((dowhat!=doanneal) && (dowhat!=docrossanneal))
    {

		// fitness?
		if (newchrome)
		{
			Fitness(trial);
	}
	InsertMember(target,trial);
	}
  }     // if not initeval

	return pop[target];
}

Chrome* Pop::go_until(time_t max_time, int maxevals, float maxfitness)
// generate until time max_time, evals maxevals, or fitness maxfitness
// Do this to run the GA in a bigger event loop

{
	//int done = FALSE;
	int didevals = 0;
	Chrome* lastchrome;

	lastchrome=generate();
	didevals++;
		//while(maxevals == 0 || didevals<maxevals)
		while ((max_time == 0 || time(NULL) < max_time) &&
			(maxevals == 0 || didevals<maxevals) &&
			( lastchrome->nfitness->fvalue < maxfitness)
			&&!Aborted())
		{
				lastchrome = generate();
				didevals++;
	}
	return lastchrome;
}

int Pop::GetTarget()
// Tournament anti-selection for replacement.  Usually size 1 (random selection) or 2
// Will select a target that is too old, even if more fit.
{
	int target = rnd(popsize);
	int i,winner;
		if (params->params[pKillTourn]>1 && gencount- pop[target]->birth < params->params[pMaxAge])
	// pick a target to replace
	for (i=1;i<params->params[pKillTourn];i++)
	{
		winner=rnd(popsize);
		if (gencount - pop[winner]->birth > params->params[pMaxAge])
		{
			i=1000;
			target = winner;
		} else if (pop[target]->nfitness->IsBetter(pop[winner]->nfitness))
			target = winner;
	}
	return target;
}


int Pop::GetAnnealTarget()
{                                                                               // target identifies selection region in pop
// select one parent. Return pointer to selected parent
	int ts, i;
    int best,trial;

	ts = params->params[pTournSize];
	// Only tournament selection is implemented in GPQUICK.
	// Add your own methods to taste
	if (params->params[pSelectMethod] == ETournament)
	{
		best=rnd(popsize);
	for (i=1;i<ts;i++)
		{
			trial=rnd(popsize);
			if (pop[trial]->nfitness->IsBetter(pop[best]->nfitness))
				best=trial;
		}
	}
    return best;
}

//************************************************************************
//////////////////////ChromeParams methods

ChromeParams::ChromeParams()
//Set default parameters here in the constructor
{
	varcount = 0;
	funccount = 0;

	int x;
	for (x=0;x<PARAM_COUNT;x++)
		params[x]=0;

		params[pMaxExpr] = 50;         // Maximum expression size in nodes
		params[pInitExpr] = 6;          // Maximum initial expression depth
		params[pMuteRate] = 100;        // Node Mutation rate per 1000
		params[pCrossSelf] = 1;         // enable cross with self
		params[pUnRestrictWt] = 70;        // any point crossover out of 100
		params[pCrossWt] = 100;       // Crossover weight
		params[pMuteWt] = 30;        // overall Mutation weight
		params[pAnnealCrossWt] = 0;   // Crossover Annealing Weight
		params[pAnnealMuteWt] = 0;   // Mutate Annealing weight
		params[pCopyWt] = 10;        // Copy weight
		params[pMuteNodeWt] = 100;   // Node Mutate weight
		params[pMuteConstWt] = 100;    // MutateC weight
		params[pMuteShrinkWt] = 100;   // MutateShrink weight
		params[pSelectMethod] = ETournament;  // Selection method = Tournament
		params[pTournSize] = 7;         // Tournament size
		params[pMateRadius] = 500;       // Mating radius
		params[pGaussRegion] = 0;         // Guassian mate selection 0 disable 1 enable
		params[pRepeatEval] = 0;         // Repeat eval on copy? 0 no 1 yes
		params[pKillTourn] = 2;         // Size of "Kill Tournament" for replacement
		params[pMaxAge] = 2000;      // Maximum age before replaced even if better
		params[pParsimony] = 0;         // Parsimony factor
		params[pFitnessCases] = 20;        // # fitness cases
}


ChromeParams::~ChromeParams()
{
}

//************************************************************************


