Unsortify
=========

This file implements two functions:

1.  `unsortify(Module)`: returns a Module with a single sort
2.  `sortify(Term,TermSet)`: takes constants in a term that are unsorted and sorts them

`unsortify()` requires that the module to transformed has no:

a. imports (can be achieved by module transformation)
b. conditional equations
c. memberships
d. associative/id axioms

`sortify()` requires `TermSet` is actually a `ConstantSet`.

```maude
sload ../base/prelude-aux.maude
sload ../base/full-maude.maude

fmod UNSORTIFY is
  pr META-LEVEL .
  pr UNIT .
  pr ATTR-EXTRA .

  op unsortify       : FModule           -> FModule .
  op unsortify       : OpDeclSet         -> OpDeclSet .
  op unsortify       : EquationSet       -> EquationSet .
  op unsortify       : TypeList          -> TypeList .
  op unsortify       : Term              -> Term .
  op unsortify       : TermList TermList -> TermList .
  op unsortify-codom : Substitution      -> Substitution .
  op unsortify-qids  : QidSet            -> QidSet .
  op unsortify-safe  : AttrSet           -> Bool .

  var H  : Header    . var SS : SortSet . var SDS : SubsortDeclSet .
  var OS : OpDeclSet . var ES : EquationSet . var M : FModule . var AS : AttrSet .
  var TY : Type . var TYL : TypeList . var V : Variable . var C : Constant .
  var Q : Qid . var T T' : Term . var NL : NeTermList . var TL TL' : TermList .
  var TQ : TermQid . var QS : QidSet . var S : Substitution .

  eq unsortify(fmod H is nil sorts SS . SDS OS none ES endfm) =
    fmod H is nil sorts 'U . none unsortify(OS) none unsortify(ES) endfm .
  eq unsortify(M) = noModule [owise] .

 ceq unsortify(op Q : TYL -> TY [AS]. OS) = op Q : unsortify(TYL) -> 'U [AS]. unsortify(OS) if unsortify-safe(AS) .
  eq unsortify((none).OpDeclSet)          = (none).OpDeclSet .
  eq unsortify(eq T = T' [AS]. ES)        = eq unsortify(T) = unsortify(T') [AS]. unsortify(ES) .
  eq unsortify((none).EquationSet)        = (none).EquationSet .

  eq unsortify(TY TYL)                    = 'U unsortify(TYL) .
  eq unsortify(nil)                       = nil .
  eq unsortify(V)                         = qid(string(getName(V)) + ":U") .
  eq unsortify(C)                         = qid(string(getName(C)) + ".U") .
  eq unsortify(Q[NL])                     = Q[unsortify(NL,empty)] .
  eq unsortify((T,TL),TL')                = unsortify(TL,(TL',unsortify(T))) .
  eq unsortify(empty,NL)                  = NL .

  eq unsortify-codom(V <- T ; S)          = V <- unsortify(T) ; unsortify-codom(S) .
  eq unsortify-codom(none)                = none .

  eq unsortify-qids(TQ ; QS)              = unsortify(TQ) ; unsortify-qids(QS) .
  eq unsortify-qids(none)                 = none .

  eq unsortify-safe(id(T) AS)             = false .
  eq unsortify-safe(right-id(T) AS)       = false .
  eq unsortify-safe(left-id(T) AS)        = false .
  eq unsortify-safe(AS)                   = true  . --- in(assoc,AS) implies in(comm,AS) .
endfm

fmod SORTIFY is
  pr META-LEVEL .
  pr OPDECLSET-EXTRA .
  pr QIDTUPLESET .

  op sortify      : Term              QidTripleSet -> Term .
  op sortify'     : Term              QidTripleSet -> Term .
  op sortify'     : TermList TermList QidTripleSet -> Term .
  op sortify''    : Qid               QidTripleSet -> Term .
  --- create qidpairset from module for use with sortify
  op constsToQTS  : Module                       -> QidTripleSet .
  op constsToQTS  : OpDeclSet                    -> QidTripleSet .
  op varsToQTS    : QidSet                       -> QidTripleSet .
  op varsToQTS    : QidSet                       -> QidTripleSet .

  var T  : Term . var NL   : NeTermList . var TL TL' : TermList . var TQ : TermQid .
  var Q K : Qid  . var C C' : Constant   . var V      : Variable .
  var QS : QidTripleSet . var S : Sort . var OD : OpDecl . var OS : OpDeclSet .
  var VS : QidSet .

  eq sortify (T,QS)          = sortify'(T,QS) .

  eq sortify'(TQ,QS)         = sortify''(getName(TQ),QS) .
  eq sortify'(Q[NL],QS)      = Q[sortify'(NL,empty,QS)] .
  eq sortify'((T,TL),TL',QS) = sortify'(TL,(TL',sortify'(T,QS)),QS) .
  eq sortify'(empty,NL,QS)   = NL .

  eq sortify''(Q,qt(Q,K,S) | QS) = qid(string(Q) + string(K) + string(S)) .

  eq constsToQTS(M:Module)   = constsToQTS(constants(getOps(M:Module))) .
 ceq constsToQTS(OD OS)      = qt(qid(OD),'.,resultType(OD)) | constsToQTS(OS) if argTypes(OD) == nil .
  eq constsToQTS(none)       = none .
  eq varsToQTS(Q ; VS)       = qt(getName(Q),':,getType(Q)) | varsToQTS(VS) .
  eq varsToQTS(none)         = none .
endfm
```
