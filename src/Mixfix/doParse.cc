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
//	Functions that initiate parsing and analyse result.
//

void
MixfixModule::getParserStats(int& nrNonterminals, int& nrTerminals, int& nrProductions)
{
  makeGrammar();
  int nrKinds = getConnectedComponents().length();
  nrNonterminals = - nextNonTerminal;
  nrTerminals = parser->getTokenSet().cardinality();
  nrProductions = parser->getNrProductions();
}

Term*
MixfixModule::parseTerm(const Vector<Token>& bubble,
			ConnectedComponent* component,
			int begin,
			int end)
{
  makeGrammar();
  int r = parseSentence(bubble,
			((component == 0) ? TERM :
			nonTerminal(component->getIndexWithinModule(), TERM_TYPE)),
			begin,
			end);
  if (r <= 0)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) << ": no parse for term.");
      return 0;
    }

  Term* t1;
  Term* t2;
  parser->makeTerms(t1, t2);
  if (r > 1)
    {
      t1->symbol()->fillInSortInfo(t1);
      t2->symbol()->fillInSortInfo(t2);
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": ambiguous term, two parses are:\n" << t1 <<
		   "\n-versus-\n" << t2 <<
		   "\n\nArbitrarily taking the first as correct.");
      t2->deepSelfDestruct();
    }
  return t1;
}

int
MixfixModule::parseTerm2(const Vector<Token>& bubble,
			 ConnectedComponent* component,
			 Term*& parse1,
			 Term*& parse2,
			 int& firstBad)
{
  makeGrammar();
  int r = parser->parseSentence(bubble,
				((component == 0) ? TERM :
				 nonTerminal(component->getIndexWithinModule(), TERM_TYPE)),
				firstBad,
				0,
				bubble.length());
  if (r > 0)
    parser->makeTerms(parse1, parse2);
  return r;
}

void
MixfixModule::parseStatement(const Vector<Token>& bubble)
{
  makeGrammar();
  int r = parseSentence(bubble, STATEMENT);
  if (r <= 0)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": no parse for statement\n" << bubble << " .");
      return;
    }
  if (r > 1)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": multiple distinct parses for statement\n" << bubble << " .");
    }
  parser->insertStatement();
}

bool
MixfixModule::parseMatchCommand(const Vector<Token>& bubble,
				Term*& pattern,
				Term*& subject,
				Vector<ConditionFragment*>& condition)
{
  makeGrammar(true);
  int r = parseSentence(bubble, MATCH_COMMAND);
  if (r <= 0)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": no parse for command.");
      return false;
    }
  if (r > 1)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": multiple distinct parses for command.");
    }
  parser->makeMatchCommand(pattern, subject, condition);
  return true;
}

bool
MixfixModule::parseSearchCommand(const Vector<Token>& bubble,
				 Term*& initial,
				 int& searchType,
				 Term*& target,
				 Vector<ConditionFragment*>& condition)
{
  makeGrammar(true);
  int r = parseSentence(bubble, SEARCH_COMMAND);
  if (r <= 0)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": no parse for command.");
      return false;
    }
  if (r > 1)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": multiple distinct parses for command.");
    }
  parser->makeSearchCommand(initial, searchType, target, condition);
  return true;
}

bool
MixfixModule::parseStrategyCommand(const Vector<Token>& bubble,
				   Term*& subject,
				   StrategyExpression*& strategy)
{
  makeGrammar(true);
  int r = parseSentence(bubble, STRATEGY_COMMAND);
  if (r <= 0)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": no parse for command.");
      return false;
    }
  if (r > 1)
    {
      IssueWarning(LineNumber(bubble[0].lineNumber()) <<
		   ": multiple distinct parses for command.");
    }
  parser->makeStrategyCommand(subject, strategy);
  return true;
}

int
MixfixModule::parseSentence(const Vector<Token>& bubble, int root, int begin, int end)
{
  if (end == DEFAULT)
    end = bubble.length() - 1;
  int nrTokens = end - begin + 1;
  int firstBad;
  int r = parser->parseSentence(bubble, root, firstBad, begin, nrTokens);
  if (r <= 0)
    {
      if (r == -1)
	{
	  IssueWarning(LineNumber(bubble[firstBad].lineNumber()) <<
		       ": bad token " << QUOTE(bubble[firstBad].name()) << '.');
	}
      else if (firstBad == begin + nrTokens)
	{
	  IssueWarning(LineNumber(bubble[end].lineNumber()) <<
		       " : unexpected end of tokens.");
	}
      else
	{
	  IssueWarning(LineNumber(bubble[firstBad].lineNumber()) <<
		       ": didn't expect token " << QUOTE(bubble[firstBad]) << ':');
	  Token::printTokenVector(cerr, bubble, begin, firstBad, false);  // HACK
	  cerr << " <---*HERE*\n";
	}
    }
  return r;
}
