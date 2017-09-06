/*

    This file is part of the Maude 2 interpreter.

    Copyright 1997-2003 SRI International, Menlo Park, CA 94025, USA.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

*/

//
//	Implementation for class RewriteSequenceSearch.
//

//	utility stuff
#include "macros.hh"
#include "vector.hh"

//	forward declarations
#include "interface.hh"
#include "core.hh"
#include "higher.hh"

//	interface class definitions
#include "symbol.hh"
#include "dagNode.hh"

//	variable class definitions
#include "variableDagNode.hh"

//	core class definitions
#include "rewritingContext.hh"

//	higher class definitions
#include "pattern.hh"
#include "narrowingSearchState.hh"
#include "narrowingSequenceSearch.hh"
#include "freshVariableGenerator.hh"

NarrowingSequenceSearch::NarrowingSequenceSearch(RewritingContext* initial,
						 SearchType searchType,
						 Pattern* goal,
						 int maxDepth,
						 FreshVariableGenerator* freshVariableGenerator)
  : initial(initial),
    goal(goal),
    maxDepth((searchType == ONE_STEP) ? 1 : maxDepth),
    freshVariableGenerator(freshVariableGenerator)
{
  initial->reduce();
  seenSet.insert(initial->root());

  matchState = 0;

  NarrowingSearchState* initialState = new NarrowingSearchState(initial, freshVariableGenerator);
  stateStack.append(initialState);
  contextStack.append(initial);

  needToTryInitialState = (searchType == ANY_STEPS);
  //reachingInitialStateOK = (searchType == AT_LEAST_ONE_STEP || searchType == ONE_STEP);
  normalFormNeeded = (searchType == NORMAL_FORM);
  topOfStackFresh = true;
}

NarrowingSequenceSearch::~NarrowingSequenceSearch()
{
  delete matchState;
  delete goal;
  delete freshVariableGenerator;
  //
  //	initial get deleted as the 0th element in contextStack.
  //
  int nrPendingStates = stateStack.size();
  for (int i = 0; i < nrPendingStates; ++i)
    {
      delete stateStack[i];
      delete contextStack[i];
    }
}

bool
NarrowingSequenceSearch::findNextMatch()
{
  if (matchState != 0)
    goto tryMatch;  // non-startup case

  for(;;)
    {
      if (!(normalFormNeeded ? findNextNormalForm() : findNextInterestingState()))
	break;
      {
	matchState = new MatchSearchState(initial->makeSubcontext(getStateDag()),
					  goal,
					  MatchSearchState::GC_CONTEXT);
      }
    tryMatch:
      bool foundMatch = matchState->findNextMatch();
      //matchState->transferCount(*(getContext()));
      if (foundMatch)
	return true;
      delete matchState;
    }

  matchState = 0;
  return false;
}

bool
NarrowingSequenceSearch::findNextNormalForm()
{
  int currentIndex = stateStack.size() - 1;
  NarrowingSearchState* currentState;

  if (!topOfStackFresh)
    {
    backtrack:
      //
      //	Backtrack.
      //
      delete stateStack[currentIndex];
      delete contextStack[currentIndex];
      stateStack.resize(currentIndex);
      contextStack.resize(currentIndex);
      --currentIndex;
      if (currentIndex < 0)
	return false;
      topOfStackFresh = false;
    }
  //
  //	Forward.
  //
  currentState = stateStack[currentIndex];
  while (currentState->findNextNarrowing())
    {
      if (currentIndex == maxDepth)
	goto backtrack;
      DagNode* narrowedDag = currentState->getNarrowedDag();
      RewritingContext* newContext = initial->makeSubcontext(narrowedDag);
      newContext->reduce();
      if (seenSet.dagNode2Index(newContext->root()) != NONE)
	{
	  delete newContext;
	  topOfStackFresh = false;
	  continue;
	}
      seenSet.insert(newContext->root());
      currentState = new NarrowingSearchState(newContext, freshVariableGenerator);
      stateStack.append(currentState);
      contextStack.append(newContext);
      ++currentIndex;
      topOfStackFresh = true;
    }
  if (!topOfStackFresh)
    goto backtrack;

  topOfStackFresh = false;  // top of stack will be stale next time we're called
  return true;
}

bool
NarrowingSequenceSearch::findNextInterestingState()
{
  if (needToTryInitialState)
    {
      //
      //	Special case: return the initial state.
      //
      needToTryInitialState = false;  // don't do this again
      return true;
    }

  int currentIndex = stateStack.size() - 1;
  while (currentIndex >= 0)
    {
      NarrowingSearchState* currentState = stateStack[currentIndex];
      if ((maxDepth == NONE || currentIndex < maxDepth) && currentState->findNextNarrowing())
	{
	  DagNode* narrowedDag = currentState->getNarrowedDag();
	  ////cout << currentState->getDagNode(0) << " narrowed to " << narrowedDag << endl;

	  RewritingContext* newContext = initial->makeSubcontext(narrowedDag);
	  newContext->reduce();
	  ////cout << "which reduced to " << newContext->root() << endl;
	  if (seenSet.dagNode2Index(newContext->root()) != NONE)
	    {
	      delete newContext;
	      continue;
	    }
	  seenSet.insert(newContext->root());
	  stateStack.append(new NarrowingSearchState(newContext, freshVariableGenerator));
	  contextStack.append(newContext);
	  return true;
	}
      stateStack.resize(currentIndex);
      contextStack.resize(currentIndex);
      --currentIndex;
    }
  return false;
}
