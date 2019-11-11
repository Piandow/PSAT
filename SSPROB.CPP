// Copyright Andy Singleton, 1993,1994
// This code is released for non-commercial use only
// For questions or upgrades contact:
// Andy Singleton, Creation Mechanics Inc.
// PO Box 248, Peterborough, NH 03458
// Internet: p00396@psilink.com
// Compuserve 73313,757
// Phone: (603) 563-7757


// GPQUICK
// sample GP problem file
// Compatible with problem DLL's for the GP-GIM professional GP system
// SSRProb = Simple Symbolic regression problem.
// Includes main function

#include "pch.h"
#include "chrome.h"
#include "primitiv.h"
#include <iostream>
#include<fstream>

using namespace std;

extern char scratch[100];
#ifdef FASTEVAL
	extern evalnode* IpGlobal;
	extern Chrome* ChromeGlobal;
#endif


//**************************************************************************
// Define your Problem class

class SSRProb : public Problem {
public:
	SSRProb(ChromeParams* p=NULL);  // Initialize primitives, parameters, and data
	float fitness(Chrome* chrome);  // Driving ingredient - GP fitness function

	//virtual FitnessValue* GetFitnessObj(); // Optional new behavior
};

//**************************************************************************
// static variables that make up the evaluation environment
// Build your environment here as statics
// where the evaluation functions can get to the data
retval john, paul, george, ringo, elvis;


//**************************************************************************
/////////////////// Problem specific functions and terminals
// Use the OPDEF,EVAL,IP and CURCHROME defines
// and the code will recompile for different eval methods

				// A terminal named "John" that returns a data value
OPDEF(JohnEval) {return john;}

				// A terminal named "Paul" that returns a data value
OPDEF(PaulEval) {return paul;}

				// A terminal named "George" that returns a data value
OPDEF(GeorgeEval) {return george;}

				// A terminal named "Ringo" that returns a data value
OPDEF(RingoEval) {return ringo;}

				// A terminal named "Elvis" that returns a data value
OPDEF(ElvisEval) {return elvis;}



//**************************************************************************
///////////////////////// PROBLEM definition


SSRProb::SSRProb(ChromeParams* p) : Problem()
{
	AddF(new ConstFunc(300));    // REQUIRED as function 0

	// Add some standard primitives
	AddF(new AddFunc(100));
	AddF(new SubFunc(100));
	AddF(new MulFunc(100));
	AddF(new DivFunc(100));

	// These are not used for SSPROB, but are available
	//AddF(new Add4Func(100));
	//AddF(new Prog4Func(100));
    //AddF(new SineFunc(100));
	//AddF(new AbsFunc(100));
	//AddF(new BoolFunc(100));
	//AddF(new NotFunc(100));
	//AddF(new AndFunc(100));
	//AddF(new OrFunc(100));
	//AddF(new IfFunc(100));
	//AddF(new IflteFunc(100));

	// Add the problem specific primitives
	AddF(new Function(0,0,100,JohnEval,"John"));
	AddF(new Function(0,0,100,PaulEval,"Paul"));
	AddF(new Function(0,0,100,GeorgeEval,"George"));
	AddF(new Function(0,0,100,RingoEval,"Ringo"));
	AddF(new Function(0,0,100,ElvisEval,"Elvis"));

	// Set some parameters
	p->params[pRepeatEval]=1;               // repeat evaluation with new case on copy
	p->params[pMuteShrinkWt]=30;    // population tends to collapse toward 0 average value.  Reduce shrinkage
	p->params[pMuteWt]=20;                  // standard settings
	p->params[pCrossWt]=70;
	p->params[pMuteRate]=60;
	// compensate for small population (typically 1000)
	p->params[pAnnealCrossWt]=50;           // add some annealing
	p->params[pAnnealMuteWt]=100;
	p->params[pTournSize]=6;                        //reduce greediness from 7

	// If you need to load problem specific data for evaluation
	// Or load an environment (a map, a scenario)
	// Do it here
}


// This is the value we want to discover
// Change the formula to experiment with difficulty
// Put a data field here for regression against actual data
#define REGRESSIONVALUE john*(george-paul)+2.5*ringo



float SSRProb::fitness(Chrome* chrome)
// Regression type fitness function.
// Calculate a sum squared error over some number of cases

{
	int i;
	float f = 0;
	retval answer,diff;

	// Check some number of cases (20 here)
	for (i=0;i<20;i++)
	{
		// Set up the evaluation environment
		// for a specific case
		john=rnd(1000);
		paul=rnd(1000);
		george=rnd(1000);
		ringo=rnd(1000);
		elvis=rnd(1000);

		// Evaluate the expression
		answer=chrome->evalAll();

		// Sum up the squared error
		diff = (REGRESSIONVALUE) - answer;
		f-=diff*diff;
	}

	chrome->nfitness->fvalue=f;
	return f;
}



//**************************************************************************
///////////////////////////////  MAIN

int main(int, char **)
// Run SSRProb (the Simple Symbolic Regression Problem)
// With a population of size 2000
// Run for 80,000 evals, or until it finds the answer
// DOS version will exit on keyboard hit
// Every 5 seconds or 1000 evals, print the best answer so far

{
	cout <<"I DO RUN"<<endl;
	ChromeParams* params = new ChromeParams();
	Problem* prob=new SSRProb(params);
	Pop* pop;

	cout << "\nGPQUICK Sample Problem - Simple Symbolic Regression\n";

	rndomize();
	pop = new Pop(prob,params,1000);             // population of size 2000

	int done = 0;


	// Do for 80,000 iterations, or until we find the right answer
	//while (pop->gencount<80000l && !done)  // Portable version - not interruptible
	while (pop->gencount<80000l && !done && !kbhit())  // kbhit IS NOT PORTABLE (Borland specific)
	{
		// go for 5 seconds, 1000 evals, or until we get to less than 4 error
		pop->go_until(time(NULL)+5,1000,-4);

		// Display the best so far
		cout << "\n\nAfter " << pop->gencount << " generates, error = " << pop->BestFitness->fvalue << "\n";
		//pop->pop[pop->BestMember]->write(PRETTY_NONE,cout);
		cout.flush();

		done = (pop->BestFitness->fvalue >=-4);
	}


	// Display the best
	cout << "\n\nFINAL RESULTS...";
	cout << "\nAfter " << pop->gencount << " generates, error = " << pop->BestFitness->fvalue << "\n";
	//pop->pop[pop->BestMember]->write(PRETTY_NONE,cout);
	cout.flush();

	delete pop;
	delete prob;
	delete params;
	return done? 1 : 0;
}
