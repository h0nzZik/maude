/*

    This file is part of the Maude 2 interpreter.

    Copyright 1997-2007 SRI International, Menlo Park, CA 94025, USA.

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
//	Code for unification descent functions.
//

local_inline bool
MetaLevelOpSymbol::getCachedUnificationProblem(MetaModule* m,
					       FreeDagNode* subject,
					       Int64 solutionNr,
					       UnificationProblem*& unification,
					       Int64& lastSolutionNr)
{
  if (solutionNr > 0)
    {
      if (m->remove(subject, unification, lastSolutionNr))
	{
	  if (lastSolutionNr < solutionNr)
	    return true;
	  delete unification;
	}
    }
  return false;
}

bool
MetaLevelOpSymbol::metaUnify2(FreeDagNode* subject, RewritingContext& context, bool disjoint)
{
  //
  //	We handle both metaUnify() and metaDisjointUnify().
  //
  if (MetaModule* m = metaLevel->downModule(subject->getArgument(0)))
    {
      Int64 solutionNr;
      DagNode* metaVarIndex = subject->getArgument(2);
      if (metaLevel->isNat(metaVarIndex) &&
	  metaLevel->downSaturate64(subject->getArgument(3), solutionNr))
	{
	  const mpz_class& varIndex = metaLevel->getNat(metaVarIndex);
	  UnificationProblem* unification;
	  Int64 lastSolutionNr;
	  if (!getCachedUnificationProblem(m, subject, solutionNr, unification, lastSolutionNr))
	    {
	      Vector<Term*> lhs;
	      Vector<Term*> rhs;
	      if (!metaLevel->downUnificationProblem(subject->getArgument(1), lhs, rhs, m, disjoint))
		return false;
	      unification = new UnificationProblem(lhs, rhs, new FreshVariableSource(m, varIndex));
	      if (!(unification->problemOK()))
		{
		  delete unification;
		  return false;
		}
	      lastSolutionNr = -1;
	    }

	  DagNode* result;
	  m->protect();
	  for (; lastSolutionNr < solutionNr; ++lastSolutionNr)
	    {
	      if (!(unification->findNextUnifier()))
		{
		  delete unification;
		  result = disjoint ? metaLevel->upNoUnifierTriple() : metaLevel->upNoUnifierPair();
		  goto fail;
		}
	    }
	  m->insert(subject, unification, solutionNr);
	  {
	    const Substitution& solution = unification->getSolution();
	    const VariableInfo& variableInfo = unification->getVariableInfo();
	    mpz_class lastVarIndex = varIndex + unification->getNrFreeVariables();
	    result = disjoint ?
	      metaLevel->upUnificationTriple(solution, variableInfo, lastVarIndex, m) :
	      metaLevel->upUnificationPair(solution, variableInfo, lastVarIndex, m);
	  }
	fail:
	  (void) m->unprotect();
	  return context.builtInReplace(subject, result);
	}
    }
  return false;
}

bool
MetaLevelOpSymbol::metaUnify(FreeDagNode* subject, RewritingContext& context)
{
  //
  //	op metaUnify : Module UnificationProblem Nat Nat ~> UnificationPair? .
  //
  return metaUnify2(subject, context, false);
}

bool
MetaLevelOpSymbol::metaDisjointUnify(FreeDagNode* subject, RewritingContext& context)
{
  //
  //	op metaDisjointUnify : Module UnificationProblem Nat Nat ~> UnificationTriple? .
  //
  return metaUnify2(subject, context, true);
}

bool
MetaLevelOpSymbol::metaXunify2(FreeDagNode* subject, RewritingContext& context, bool disjoint)
{
  //
  //	We handle both metaXunify() and metaDisjointXunify().
  //
  if (MetaModule* m = metaLevel->downModule(subject->getArgument(0)))
    {
      Int64 solutionNr;
      DagNode* metaVarIndex = subject->getArgument(3);
      if (metaLevel->isNat(metaVarIndex) &&
	  metaLevel->downSaturate64(subject->getArgument(4), solutionNr))
	{
	  const mpz_class& varIndex = metaLevel->getNat(metaVarIndex);
	  UnificationProblem* unification;
	  Int64 lastSolutionNr;
	  if (!getCachedUnificationProblem(m, subject, solutionNr, unification, lastSolutionNr))
	    {
	      Vector<Term*> lhs(1);
	      Vector<Term*> rhs(1);
              if (!metaLevel->downTermPair(subject->getArgument(1), subject->getArgument(2), lhs[0], rhs[0], m, disjoint))
                return false;
	      unification = new UnificationProblem(lhs, rhs, new FreshVariableSource(m, varIndex), true);
	      if (!(unification->problemOK()))
		{
		  delete unification;
		  return false;
		}
	      lastSolutionNr = -1;
	    }

	  DagNode* result;
	  m->protect();
	  for (; lastSolutionNr < solutionNr; ++lastSolutionNr)
	    {
	      if (!(unification->findNextUnifier()))
		{
		  delete unification;
		  result = disjoint ? metaLevel->upNoUnifierContext4Tuple() : metaLevel->upNoUnifierContextTriple();
		  goto fail;
		}
	    }
	  m->insert(subject, unification, solutionNr);
	  {
	    //
	    //	Build a context with a hole.
	    //
	    Sort* sort = unification->getLeftHandSides()[0]->symbol()->rangeComponent()->sort(Sort::KIND);  // HACK
	    VariableSymbol* vs = safeCast(VariableSymbol*, m->instantiateVariable(sort));
	    DagNode* hole = new VariableDagNode(vs, 0, UNDEFINED);
	    DagNode* context = unification->makeContext(hole);
	    //
	    //	Up the solution.
	    //
	    const Substitution& solution = unification->getSolution();
	    const VariableInfo& variableInfo = unification->getVariableInfo();
	    mpz_class lastVarIndex = varIndex + unification->getNrFreeVariables();
	    result = disjoint ?
	      metaLevel->upUnificationContext4Tuple(solution, variableInfo, context, hole, lastVarIndex, m) :
	      metaLevel->upUnificationContextTriple(solution, variableInfo, context, hole, lastVarIndex, m);
	  }
	fail:
	  (void) m->unprotect();
	  return context.builtInReplace(subject, result);
	}
    }
  return false;
}

bool
MetaLevelOpSymbol::metaXunify(FreeDagNode* subject, RewritingContext& context)
{
  //
  //	op metaXunify : Module Term Term Nat Nat ~> UnificationContextTriple? .
  //
  return metaXunify2(subject, context, false);
}

bool
MetaLevelOpSymbol::metaDisjointXunify(FreeDagNode* subject, RewritingContext& context)
{
  //
  //	op metaDisjointXunify : Module Term Term Nat Nat ~> UnificationContext4Tuple? .
  //
  return metaXunify2(subject, context, true);
}
