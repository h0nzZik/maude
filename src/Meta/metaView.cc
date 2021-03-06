/*

    This file is part of the Maude 2 interpreter.

    Copyright 2018 SRI International, Menlo Park, CA 94025, USA.

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
//      Implementation for class SyntacticView.
//

//      utility stuff
#include "macros.hh"
#include "vector.hh"

//      forward declarations
#include "core.hh"
#include "interface.hh"
#include "term.hh"
#include "higher.hh"
#include "strategyLanguage.hh"
#include "mixfix.hh"
#include "meta.hh"

//      interface class definitions
#include "symbol.hh"
#include "dagNode.hh"

//	front end class definitions
#include "interpreter.hh"
#include "metaLevel.hh"
#include "metaView.hh"

MetaView::MetaView(Token viewName, DagNode* opMappings, MetaLevel* metaLevel, Interpreter* owner)
  : View(viewName, owner),
    opMappings(opMappings),
    metaLevel(metaLevel)
{
}

MetaView::~MetaView()
{
}

bool
MetaView::handleOpTermMappings()
{
  //
  //	First we pull down the op->term mappings; this can only be done once
  //	the module expressions for the from theory and to module have be evaluated.
  //
  ImportModule* fromTheory = getFromTheory();
  ImportModule* toModule = getToModule();
  Vector<Term*> fromTerms;
  Vector<Term*> toTerms;
  if (!(metaLevel->downOpTermMappings(opMappings.getNode(), fromTheory, toModule, fromTerms, toTerms)))
    return false;
  //
  //	Then we go though them, inserting them into the base view.
  //
  int nrMappings = fromTerms.size();
  for (int i = 0; i < nrMappings; ++i)
    {
      if (!(insertOpToTermMapping(FileTable::META_LEVEL_CREATED,
				  fromTerms[i],
				  FileTable::META_LEVEL_CREATED,
				  toTerms[i],
				  toModule)))
	{
	  for (++i; i < nrMappings; ++i)
	    {
	      fromTerms[i]->deepSelfDestruct();
	      toTerms[i]->deepSelfDestruct();
	    }
	  return false;
	}
    }
  return true;
}
