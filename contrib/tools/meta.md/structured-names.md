Structured Names
================

For substutions:

```maude
sload cterms.maude
```

First, we'll need a way to generate variables at the object level because writing all our syntax at the meta-level can be a pain.
Additionally, we'll need a way to have "structured" sort names and operator names.
For this, a few constructors are provided.

`var<X>`, where `X` is some sort, will create an object-level variable of the given sort by first creating a meta-level variable and then downing it.

```maude
fmod STRUCTURED-NAME is
   protecting META-LEVEL .
   protecting SUBSTITUTION-SET .

    vars I I' : Import . vars IL IL' : ImportList . vars S S' S'' : Sort . var SS : SortSet . var SSDS : SubsortDeclSet .
    vars H H' : Header . vars Q Q' : Qid . vars A A' : Attr . var AS : AttrSet . var SU : Substitution .
    vars OP OP' : OpDecl . var OPDS : OpDeclSet . vars MB MB' : MembAx . var MAS : MembAxSet .
    vars EQ EQ' : Equation . var EQS : EquationSet . vars RL RL' : Rule . var RLS : RuleSet .
    var TYPE : Type . vars NeTL NeTL' : NeTypeList . var TL : TypeList . vars COND COND' : Condition . vars EQC EQC' : EqCondition .
    vars TERM TERM' : Term . var TERML : TermList . vars NeTERML NeTERML' : NeTermList . var VAR : Variable . var CONST : Constant . var N : Nat .

    op _<_>  : Qid  TypeList -> Qid      [ctor prec 23] .
    op _{_}  : Sort TypeList -> Sort     [ctor prec 23] .
    op _@_   : Sort Sort     -> Sort     [ctor prec 23] .
    op _?    : Sort          -> Sort     [ctor prec 23] .
    op _==>_ : Sort Sort     -> Sort     [ctor prec 25 gather(e E)] .
    op const : Qid  Sort     -> Constant [ctor] .
    op var   : Qid  Sort     -> Variable [ctor] .
    ---------------------------------------------

    op downQidError?          : -> [Qid] .
    op downSubstitutionError? : -> [Substitution] .
    op downImportListError?   : -> [ImportList] .
    op downSortsError?        : -> [SortSet] .
    op downSubsortsError?     : -> [SubsortDeclSet] .
    op downAttrSetError?      : -> [AttrSet] .
    op downOpSetError?        : -> [OpDeclSet] .
    op downMembAxSetError?    : -> [MembAxSet] .
    op downEqSetError?        : -> [EquationSet] .
    op downRuleSetError?      : -> [RuleSet] .
    ------------------------------------------

    op var<Qid>            : Qid -> Qid .
    op var<Substitution>   : Qid -> Substitution .
    op var<ImportList>     : Qid -> ImportList .
    op var<Sort>           : Qid -> Sort .
    op var<SortSet>        : Qid -> SortSet .
    op var<SubsortDeclSet> : Qid -> SubsortDeclSet .
    op var<AttrSet>        : Qid -> AttrSet .
    op var<OpDeclSet>      : Qid -> OpDeclSet .
    op var<MembAxSet>      : Qid -> MembAxSet .
    op var<EquationSet>    : Qid -> EquationSet .
    op var<RuleSet>        : Qid -> RuleSet .
    -----------------------------------------
   ceq var<Qid>            (Q) = Q'   if Q'   := downTerm(qid(string(Q) + ":Qid"),            downQidError?) .
   ceq var<Substitution>   (Q) = SU   if SU   := downTerm(qid(string(Q) + ":Substitution"),   downSubstitutionError?) .
   ceq var<ImportList>     (Q) = IL   if IL   := downTerm(qid(string(Q) + ":ImportList"),     downImportListError?) .
   ceq var<Sort>           (Q) = S    if S    := downTerm(qid(string(Q) + ":Sort"),           downSortsError?) .
   ceq var<SortSet>        (Q) = SS   if SS   := downTerm(qid(string(Q) + ":SortSet"),        downSortsError?) .
   ceq var<SubsortDeclSet> (Q) = SSDS if SSDS := downTerm(qid(string(Q) + ":SubsortDeclSet"), downSubsortsError?) .
   ceq var<AttrSet>        (Q) = AS   if AS   := downTerm(qid(string(Q) + ":AttrSet"),        downAttrSetError?) .
   ceq var<OpDeclSet>      (Q) = OPDS if OPDS := downTerm(qid(string(Q) + ":OpDeclSet"),      downOpSetError?) .
   ceq var<MembAxSet>      (Q) = MAS  if MAS  := downTerm(qid(string(Q) + ":MembAxSet"),      downMembAxSetError?) .
   ceq var<EquationSet>    (Q) = EQS  if EQS  := downTerm(qid(string(Q) + ":EquationSet"),    downEqSetError?) .
   ceq var<RuleSet>        (Q) = RLS  if RLS  := downTerm(qid(string(Q) + ":RuleSet"),        downRuleSetError?) .
```

It's useful when building universal constructions to generate the same structure, but with incremented variable names.

-   `prime` will add a prime `'` to every variable in a given term, the variant which takes an extra sort only primes that extra sort.
-   `#prime` is the meta-level version, which does the same over a `Term`.

```maude
    op prime :     Sort -> Sort .
    op prime : Nat Sort -> Sort .
    -----------------------------
    eq prime(S)    = prime(1, S) .
   ceq prime(N, S) = S' if S' := downTerm(#prime(N, upTerm(S)), downSortsError?) .

    op prime :     Sort Sort -> Sort .
    op prime : Nat Sort Sort -> Sort .
    ----------------------------------
    eq prime(S, S')    = prime(1, S, S') .
   ceq prime(N, S, S') = S'' if S'' := downTerm(upTerm(S') << (upTerm(S) <- upTerm(prime(N, S))), downSortsError?) .

    op #prime : Nat TermList -> TermList .
    --------------------------------------
    eq #prime(N, VAR)                  = qid(string(getName(VAR)) + #primes(N) + ":" + string(getType(VAR))) .
    eq #prime(N, CONST)                = CONST .
    eq #prime(N, Q[NeTERML])           = Q[#prime(N, NeTERML)] .
    eq #prime(N, (NeTERML , NeTERML')) = #prime(N, NeTERML) , #prime(N, NeTERML') .

    op #primes : Nat -> String .
    ----------------------------
    eq #primes(0)    = "" .
    eq #primes(s(N)) = "'" + #primes(N) .
```

We'll need the ability to get the free variables of some specific sorts.

-   `#fv<Sort>` is a meta-level operation which gets all the variables of sort `Sort` from a `Term`.

```maude
    op #fv<Sort> : TermList -> SortSet .
    ------------------------------------
    eq #fv<Sort>(VAR)                  = if getType(VAR) == 'Sort then var<Sort>(getName(VAR)) else none fi .
    eq #fv<Sort>(CONST)                = none .
    eq #fv<Sort>(Q[NeTERML])           = #fv<Sort>(Q) ; #fv<Sort>(NeTERML) .
    eq #fv<Sort>((NeTERML , NeTERML')) = #fv<Sort>(NeTERML) ; #fv<Sort>(NeTERML') .
    eq #fv<Sort>(Q)                    = none [owise] .
```

We'll also need ways to "resolve" these names into proper Maude names, so that we can do execution in Maude with the results.
Here we provide the "base" resolution, as well as lifting it over the various meta-level data.

-   `resolveNames` provides the reduction of structured names to unstructured
    Core Maude names (a single quoted identifier).

TODO: `resolveNames` should be defined over just a few primitives (the ones that we have structured names for), and `#resolveNames` should be a META-LEVEL version which descends to those points in the term and then calls the object level version.
This should fix the preregularity issue with this as well.

```maude
    op resolveNames : Header -> Header .
    ------------------------------------
    eq resolveNames(H) = H [owise] .

    op #tl-string : TypeList -> String .
    ------------------------------------
    eq #tl-string(nil)        = "" .
    eq #tl-string(S)          = string(S) .
    eq #tl-string(NeTL NeTL') = #tl-string(NeTL) + ";" + #tl-string(NeTL') .

    op resolveNames : TypeList -> TypeList .
    ----------------------------------------
    eq resolveNames((nil).TypeList) = nil .
    eq resolveNames(NeTL NeTL')     = resolveNames(NeTL) resolveNames(NeTL') .

    op resolveNames : Qid -> Qid .
    ------------------------------
    eq resolveNames(Q < TL >) = qid(string(resolveNames(Q)) + "<" + #tl-string(resolveNames(TL)) + ">") .
    eq resolveNames(Q)        = Q [owise] .

    op resolveNames : TermList -> TermList .
    ----------------------------------------
    eq resolveNames(const(Q, S))          = qid(string(resolveNames(Q)) + "." + string(resolveNames(S))) .
    eq resolveNames(var(Q, S))            = qid(string(resolveNames(Q)) + ":" + string(resolveNames(S))) .
    eq resolveNames(Q[TERML])             = resolveNames(Q)[resolveNames(TERML)] .
    eq resolveNames((NeTERML , NeTERML')) = resolveNames(NeTERML) , resolveNames(NeTERML') .
    eq resolveNames(TERM)                 = TERM [owise] .

    op resolveNames : SortSet -> SortSet .
    --------------------------------------
    eq resolveNames((none).SortSet) = none .
    eq resolveNames(S { TL })       = qid(string(resolveNames(S)) + "{" + #tl-string(resolveNames(TL)) + "}") .
    eq resolveNames(Q @ S)          = qid(string(resolveNames(Q)) + "@" + string(resolveNames(S))) .
    eq resolveNames(S ==> S')       = qid("(" + string(resolveNames(S)) + "==>" + string(resolveNames(S')) + ")") .
    eq resolveNames(S ?)            = qid(string(resolveNames(S)) + "?") .
    eq resolveNames(S ; S' ; SS)    = resolveNames(S) ; resolveNames(S') ; resolveNames(SS) .
    eq resolveNames(S)              = S [owise] .

    op resolveNames : AttrSet -> AttrSet .
    --------------------------------------
    eq resolveNames((none).AttrSet) = none .
    eq resolveNames(id(TERM))       = id(resolveNames(TERM)) .
    eq resolveNames(A A' AS)        = resolveNames(A) resolveNames(A') resolveNames(AS) .
    eq resolveNames(A)              = A [owise] .

    op resolveNames : SubsortDecl -> SubsortDecl .
    ----------------------------------------------
    eq resolveNames(subsort S < S' .) = ( subsort resolveNames(S) < resolveNames(S') . ) .

    op resolveNames : Condition -> Condition .
    ------------------------------------------
    eq resolveNames((nil).Condition) = nil .
    eq resolveNames(TERM : S)        = resolveNames(TERM) :  resolveNames(S) .
    eq resolveNames(TERM = TERM')    = resolveNames(TERM) =  resolveNames(TERM') .
    eq resolveNames(TERM := TERM')   = resolveNames(TERM) := resolveNames(TERM') .
    eq resolveNames(TERM => TERM')   = resolveNames(TERM) => resolveNames(TERM') .
   ceq resolveNames(COND /\ COND')   = resolveNames(COND) /\ resolveNames(COND') if not (COND == nil or COND' == nil) .

    op resolveNames : OpDecl -> OpDecl .
    ------------------------------------
    eq resolveNames(op Q : TL -> TYPE [AS] .) = (op resolveNames(Q) : resolveNames(TL) -> resolveNames(TYPE) [resolveNames(AS)] .) .

    op resolveNames : MembAxSet -> MembAxSet .
    ------------------------------------------
    eq resolveNames( mb TERM : S        [AS] .) = ( mb resolveNames(TERM) : resolveNames(S)                      [resolveNames(AS)] .) .
    eq resolveNames(cmb TERM : S if EQC [AS] .) = (cmb resolveNames(TERM) : resolveNames(S) if resolveNames(EQC) [resolveNames(AS)] .) .

    op resolveNames : EquationSet -> EquationSet .
    ----------------------------------------------
    eq resolveNames( eq TERM = TERM'        [AS] .) = ( eq resolveNames(TERM) = resolveNames(TERM')                      [resolveNames(AS)] .) .
    eq resolveNames(ceq TERM = TERM' if EQC [AS] .) = (ceq resolveNames(TERM) = resolveNames(TERM') if resolveNames(EQC) [resolveNames(AS)] .) .

    op resolveNames : RuleSet -> RuleSet .
    --------------------------------------
    eq resolveNames( rl TERM => TERM'         [AS] .) = ( rl resolveNames(TERM) => resolveNames(TERM')                       [resolveNames(AS)] .) .
    eq resolveNames(crl TERM => TERM' if COND [AS] .) = (crl resolveNames(TERM) => resolveNames(TERM') if resolveNames(COND) [resolveNames(AS)] .) .
endfm
```
