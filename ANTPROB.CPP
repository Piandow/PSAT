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
// ANTPROB Artificial Ant traversing a trail of food
// Implementation by Georg Fuellen
// Includes main function
// This problem requires the file SANTAFE.TRL or equivalent 32x32 ascii trail file

// The ARTIFICIAL ANT problem was originally created by Jefferson and Collins et al
// as part of their experiments in evolution using a 64K processor CM2.
// They created the "Santa Fe Trail" and used a binary string GA to evolve
// a finite automata that could traverse the trail.
// John Koza adapted this problem to genetic programming and reported his
// results at the A-LIFE II conference.  This problem attempts to be a
// faithful reproduction of Koza's experiment.  For more information, see
// the Koza article in A-LIFE II or the Koza "GENETIC PROGRAMMING" book.
// The artificial ant problem is a popular benchmark of the
// GA learning mechanism.
// The artificial ant problem is a fairly easy problem because the trail
// never changes.  This is another reason it makes for a convenient experiment.
// You can make it harder, however, by designing trails with less food.
// For real extra credit, try moving the trail around between evals.

#include "pch.h"
#include "chrome.h"
#include "primitiv.h"

extern char scratch[100];
#ifdef FASTEVAL
	extern evalnode* IpGlobal;
	extern Chrome* ChromeGlobal;
#endif


//**************************************************************************
// Define your Problem class

class AntProb : public Problem {
public:
	AntProb(ChromeParams* p);               // initialize functions, parameters, and data
	float fitness(Chrome* chrome);  // Driving ingredient - GP fitness function
};


//**************************************************************************
// static variables that make up the evaluation environment
// Build your environment here as statics
// where the evaluation functions can get to the data
#define Grid_Horizontal 32
#define Grid_Vertical  32


unsigned char Grid[Grid_Horizontal][Grid_Vertical];
unsigned char LoadedGrid[Grid_Horizontal][Grid_Vertical];
#define START_ENERGY 600;
int energy=START_ENERGY;
int direction=0;
int xx=0; //position of the ant
int yy=0;

//**************************************************************************
// Data handling functions

retval FoodAhead()
{
	retval rval=0;
	switch(direction)
	{

		case 0:           /* is food on the right ?? */
			rval=( ( xx < Grid_Horizontal-1 ) && ( Grid[xx+1][yy] == '8' ) );
	    break;
		case 1:        /* is food up  ?? */
			rval=( ( yy > 0 ) && ( Grid[xx][yy-1] == '8' ) );
		break;
		case 2:    /* is food on the left ?? */
			rval=( ( xx > 0 ) && ( Grid[xx-1][yy] == '8' ) );
		break;
		case 3:  /* is food down ?? */
			rval=( ( yy < Grid_Vertical-1 ) && ( Grid[xx][yy+1] == '8' ) );
		break;
	}
    return rval;
}

int Reset()
{
	energy=START_ENERGY;
	direction=0;
    xx=0;
	yy=0; //position of the ant
    return 0;
}

int CopyGrid()
{
	int i,j;

	for (i=0; i<Grid_Horizontal; i++)
	{
		for (j=0; j<Grid_Vertical; j++)
		{
			Grid[i][j]=LoadedGrid[i][j];
		}
	}

	return 0;
}

// Load a GRID_HORIZONTAL column by GRID_VERTICAL row trail (usually 32x32)
// '8' marks the food pieces
BOOL LoadFromFile(char* filename = "santafe.trl")
{
	int i,j;
	char ch;
    int rval=TRUE;
	ifstream inf(filename);
	if (!inf)
	{
		 cerr<<"cannot open"<<filename;
		 rval=FALSE;
	}
    else {
		for (i=0; i<Grid_Vertical; i++)
		{
			for (j=0; j<Grid_Horizontal; j++)
			{
				inf.get(ch); while (ch =='\n' || ch == '\r') inf.get(ch);
				LoadedGrid[j][i]=ch;
			}
		}
    }
	CopyGrid();
	return rval;

}

//**************************************************************************
//////////////////////////  Problem specific functions

// Use the OPDEF,EVAL,IP and CURCHROME defines
// and the code will recompile for different eval methods


OPDEF(IfFoodEval)
{
		  retval rval;
		  if (FoodAhead())
		  {
					 rval=EVAL;
					 IP++;           // Jump the second expression
					 TRAVERSE();
					 IP--;           // And back up one (since EVAL increments)
		  }
		  else {
					 IP++;
					 TRAVERSE();    // Jump the first expression
					 IP--;                                   // Back up for EVAL
					 rval=EVAL;
		  }
		  return rval;
}

class IfFoodAhead : public Function {
public:
	IfFoodAhead() {strcpy(name,"IFFOODAHEAD");argnum=2;varnum=0;weight=100;
	evalfunc=IfFoodEval;};

};
				//
OPDEF(Progn3) {return (EVAL + EVAL + EVAL);}

				//
OPDEF(Left)
{
	retval rval;
	rval=0;
	if ( energy != 0 )
	{
		direction = ( direction + 1 ) % 4;
		energy--;
    }
	return rval;

}

				//
OPDEF(Right)
{
	retval rval;
	rval=0;
	if ( energy != 0 )
	{
		direction = ( direction + 3 ) % 4;
		energy--;
    }
	return rval;

}
				//
OPDEF(Move)
{
	retval rval=0;
	if ( energy == 0 ) {
		rval=0;
	}
    else {
	switch ( direction )
	{
		case 0:  //right
			{
				if ( xx < Grid_Horizontal-1 ) xx++;
			break;
			}
		case 1:  //up
			{
				if ( yy > 0) yy--;
			break;
			}
		case 2: //left
			{
				if ( xx > 0) xx--;
			break;
			}
		case 3: //down
		{
				if ( yy < Grid_Vertical-1) yy++;
			break;
		}
	}
    }
	energy--;
	if ( Grid[xx][yy] == '8' )      /* Note spaces will not be seen as they == 2 */
	{
		Grid[xx][yy] = '+';          /* Eats Food */
		rval = 1;
	}
	else {
		{if (((Grid[xx][yy])=='+')||(Grid[xx][yy]=='*'))
		{Grid[xx][yy] = '*';}
		else {Grid[xx][yy]= '-';}}
		rval= 0;
	}
    return rval;
}


//**************************************************************************
///////////////////////// PROBLEM definition


AntProb::AntProb(ChromeParams* p) : Problem()
{
	AddF(new ConstFunc(0));                         // Required for GPQUICK, but zero weight

	// Problem specific functions
	AddF(new Function(2,0,100,IfFoodEval,"IfFoodAhead"));
	AddF(new Function(3,0,150,Progn3,"Prog3"));
	AddF(new Function(0,0,100,Left,"Left"));
	AddF(new Function(0,0,100,Right,"Right"));
	AddF(new Function(0,0,100,Move,"Move"));

	// Set some parameters
    p->params[pRepeatEval]=0;                   // no need to repeat, same case each time
	p->params[pMuteConstWt]=0;                      // no constants
    // Get to Koza type parameters
    p->params[pMuteWt]=15;                              // less mutation
	p->params[pAnnealMuteWt]=15;                    // disable annealing
	p->params[pAnnealCrossWt] = 0;

	// If you need to load problem specific data for evaluation
	// Or load an environment (a map, a scenario)
	// Do it here
	LoadFromFile("santafe.trl");
}

float AntProb::fitness(Chrome* chrome)
{

	int x,y,stop;
	float f = 0;
	CopyGrid();
	Reset();
	retval answer=0;
	while ( energy > 0 )
	{
		answer+= chrome->evalAll();
	}
	f=answer;
	chrome->nfitness->fvalue=f;
	return f;
}



//**************************************************************************
///////////////////////////////  MAIN

//int main(int, char **)
int EvolveAnt()
// Run with a population of size 2000
// Run for 80,000 evals, or until it finds the answer
// DOS version will exit on keyboard hit
// Every 5 seconds or 1000 evals, print the best answer so far

{
	ChromeParams* params = new ChromeParams();
	Problem* prob=new AntProb(params);
	Pop* pop;

    cout << "\nGPQUICK Artificial Ant Problem\n";

	rndomize();
	pop = new Pop(prob,params,2000);                // population of size 2000

	int done = 0;

	// Do for 80,000 iterations, or until we find the right answer
	//while (pop->gencount<20000l && !done)  // Portable version - not interruptible
	while (pop->gencount<25000l && !done && !kbhit())  // kbhit IS NOT PORTABLE (Borland specific)
	{
		// go for 5 seconds, 1000 evals, or until we get a right answer
		pop->go_until(time(NULL)+5,1000,89);

		// Display the best so far
		cout << "\n\nAfter " << pop->gencount << " generates, #food found = " << pop->BestFitness->fvalue << "\n";
		pop->pop[pop->BestMember]->write(PRETTY_NONE,cout);
		cout.flush();

		done = (pop->BestFitness->fvalue >=89);
	}


	// Display the best
	ofstream of("RESULT.TXT",ios::out | ios::app);
	cout << "\n\nFINAL RESULTS...";
	cout << "\nAfter " << pop->gencount << " generates, #food found = " << pop->BestFitness->fvalue << "\n";
	pop->pop[pop->BestMember]->write(PRETTY_NONE,cout);
	cout.flush();
			cout<<"****";pop->pop[pop->BestMember]->SetupEval();

			int x,y,stop;
			float f = 0;

	    // Get the track of the best ant
			CopyGrid();
			Reset();
			retval answer=0;
			while ( energy > 0 )
			{
				answer+= pop->pop[pop->BestMember]->evalAll();
			}

	    // Show the track
			f=answer;
			cout << '\n';
			of << '\n';
				for (y=0; y<Grid_Vertical; y++)
				{
					for (x=0; x<Grid_Horizontal; x++)
					{
						cout<<Grid[x][y];
			of<<Grid[x][y];
					}
					cout<<"\n";
		    of<<"\n";
				}

		of<< "\nLegend: (8)=Uneaten Food (+)=Eaten food (-)=Tracks (*)=Multiple Tracks";

			cout<<"\nFitness="<<f << " After " << pop->gencount << " generates";
			of<<"\nFitness="<<f << " After " << pop->gencount << " generates";

			cout<<"\n";pop->pop[pop->BestMember]->write(PRETTY_NONE,cout);
			of<<"\n";pop->pop[pop->BestMember]->write(PRETTY_NONE,of);

			cout << "\nResult in RESULT.TXT";

			of.close();
	delete pop;
	delete prob;
	delete params;
	return done? 1 : 0;
}

/*int main(int, char **)
{
	int i;
	BOOL done=FALSE;
	// remove comments to collect statistics
//      for (i=0;i<100 && !done;i++)
//    {
		EvolveAnt();
//              done=kbhit();
//      }
	return done? 0 : 1;
}
	*/
