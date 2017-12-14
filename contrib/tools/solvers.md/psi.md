PSI - Exact Probabalistic Inference
===================================

The [PSI Solver](http://psisolver.org) does exact symbolic inference/simplifications over Probability Distributions.
Here we build the syntax and some semantics of the PSI solver, allowing us to use probability distributions directly in Maude.

### Utilities

-   `IDS` gives us a handy back of variables.
-   `RENAMED-RATS` allows us to re-use syntax from the Prelude.

```maude
set include BOOL off .

fmod IDS is
    sort Id .

    ops a b c d f g h i j k l m n o p q r s t u v w x y z : -> Id .
endfm

fmod RENAMED-RAT is
   protecting RAT * ( op true  to tt
                    , op false to ff
                    , op _<_   to _P<_
                    , op _>_   to _P>_
                    ) .
endfm
```

PSI Languages
-------------

PSI has two languages, an internal representation and a user language.
Probabalistic simplifications and reasoning happen over the internal language; the user language is compiled to the internal language by PSI.

### Internal Language

The internal language is that of arithmetic expressions over discrete and continuous probability distributions.
Sort `DExp` is the grammar of this language:

-   Subsort `Id` is for variables.
-   Subsort `DConst` is for rational numbers and other special constants.

```maude
fmod PSI-INTERNAL is
   protecting IDS .
   protecting RENAMED-RAT .

    sorts DConst DExp .
    -------------------
    subsort Rat < DConst .
    subsorts Id DConst < DExp .

    var DC : DConst . vars DE DE' : DExp .
```

Arithmetic is supported, with some special constants added.

```maude
   ops pi e : -> DConst .
   ----------------------

    op -_ : DExp -> DExp [ditto] .
    ------------------------------

    op _+_ : DExp DExp -> DExp [ditto] .
    op _-_ : DExp DExp -> DExp [ditto] .
    op _*_ : DExp DExp -> DExp [ditto] .
    op _/_ : DExp DExp -> DExp [ditto] .
    op _^_ : DExp DExp -> DExp [ditto] .
    ------------------------------------
```

[Iverson Brackets](https://en.wikipedia.org/wiki/Iverson_bracket) are used to represent various conditionals.

TODO: Stop using non-ascii syntax variants.
TODO: Add subsort `BDExp` and operator `[_]` to turn any boolean into a condition.

```maude
    op [_<_] : DExp DExp -> DExp .
    op [_>_] : DExp DExp -> DExp .
    op [_≤_] : DExp DExp -> DExp .
    op [_≥_] : DExp DExp -> DExp .
    op [_=_] : DExp DExp -> DExp [comm] .
    op [_≠_] : DExp DExp -> DExp [comm] .
    -------------------------------------
```

Some primitive functions are included, including:

-   Logarithms, exponentials, and trigonometric functions,
-   Floor and ceiling functions, and
-   Dirac delta (point distribution) and Error function (anti-derivative of normal distribution).

TODO: Rename `gaussAnti` to `err`.

```maude
    op ln        : DExp -> DExp .
    op exp       : DExp -> DExp .
    op sin       : DExp -> DExp .
    op cos       : DExp -> DExp .
    op floor     : DExp -> DExp .
    op ceil      : DExp -> DExp .
    op dirac     : DExp -> DExp .
    op δ(0)[_]   : DExp -> DExp .
    op gaussAnti : Id DExp -> DExp .
    --------------------------------
    eq dirac(DE) = δ(0)[DE] .
    eq exp(DE)   = e ^ DE .
```

Operators in sort `Binder` bind a variable in context, which must be taken into account when doing substitution.

```maude
    sort Binder .
    -------------
    vars X Y : Id . var S : Subst . var BIND : Binder .

    op __._ : Binder Id DExp -> DExp [prec 20] .
   ops sum int lim : -> Binder .
   -----------------------------
```

Finally, substitutions over `DExp` are provided.

```maude
    sort Subst .
    ------------

    op [_/_] : DExp Id    -> Subst .
    op __    : DExp Subst -> DExp [prec 55] .
    -----------------------------------------
    eq X [ DE / Y ] = if X == Y then DE else X fi .

    eq DC         S = DC .
    eq (DE + DE') S = (DE S) + (DE' S) .
    eq (DE - DE') S = (DE S) + (DE' S) .
    eq (DE * DE') S = (DE S) * (DE' S) .
    eq (DE / DE') S = (DE S) * (DE' S) .
    eq (DE ^ DE') S = (DE S) ^ (DE' S) .

    eq [ DE < DE' ] S = [ (DE S) < (DE' S) ] .
    eq [ DE > DE' ] S = [ (DE S) > (DE' S) ] .
    eq [ DE ≤ DE' ] S = [ (DE S) ≤ (DE' S) ] .
    eq [ DE ≥ DE' ] S = [ (DE S) ≥ (DE' S) ] .
    eq [ DE = DE' ] S = [ (DE S) = (DE' S) ] .
    eq [ DE ≠ DE' ] S = [ (DE S) ≠ (DE' S) ] .

    eq ln(DE)           S = ln(DE S) .
    eq exp(DE)          S = exp(DE S) .
    eq sin(DE)          S = sin(DE S) .
    eq cos(DE)          S = cos(DE S) .
    eq floor(DE)        S = floor(DE S) .
    eq ceil(DE)         S = ceil(DE S) .
    eq δ(0)[DE]         S = δ(0)[DE S] .
    eq gaussAnti(X, DE) S = gaussAnti(X S, DE S) .

    eq BIND X . DE [ DE' / Y ] = if X == Y then BIND X . DE else BIND X . (DE [ DE' / Y ]) fi .
endfm
```

### PSI Simplification

Many simple algebraic simplifications can be expressed directly in Maude and greatly reduce the size of the generated terms.

```maude
fmod PSI-INTERNAL-SIMPLIFICATION is
    including PSI-INTERNAL .

    vars R R' : Rat . var NzR : NzRat .
    vars DE DE' : DExp . var DC : DConst . vars X Y : Id .

    --- Arithmetic
    eq    - (- DE)  = DE .
    eq DE - (- DE') = DE + DE' .
    eq 0  + DE'     = DE' .
    eq 0  * DE'     = 0 .
    eq 1  * DE'     = DE' .

    --- Iverson Brackets
    eq [ R < R' ] = if R P<  R' then 1 else 0 fi .
    eq [ R > R' ] = if R P>  R' then 1 else 0 fi .
    eq [ R ≤ R' ] = if R <=  R' then 1 else 0 fi .
    eq [ R ≥ R' ] = if R >=  R' then 1 else 0 fi .
    eq [ R = R' ] = if R ==  R' then 1 else 0 fi .
    eq [ R ≠ R' ] = if R =/= R' then 1 else 0 fi .

    --- Dirac Delta Arithmetic
    eq δ(0)[NzR] = 0 .
endfm
```

### PSI User Language

```maude
fmod PSI-USER is
   protecting PSI-INTERNAL .
   protecting RENAMED-RAT .

    sort Exp .
    ----------
    subsort DExp < Exp .

    op _+_ : Exp Exp -> Exp [ditto] .
    op _-_ : Exp Exp -> Exp [ditto] .
    op _*_ : Exp Exp -> Exp [ditto] .
    op _/_ : Exp Exp -> Exp [ditto] .
    op _^_ : Exp Exp -> Exp [ditto] .
    ---------------------------------

   ops true false : -> Exp .
    op _&&_ : Exp Exp -> Exp [assoc comm] .
    op _||_ : Exp Exp -> Exp [assoc comm] .
    ---------------------------------------

    op _<_   : Exp Exp -> Exp .
    op _>_   : Exp Exp -> Exp .
    op _≤_   : Exp Exp -> Exp .
    op _≥_   : Exp Exp -> Exp .
    op _===_ : Exp Exp -> Exp [comm] .
    op _≠_   : Exp Exp -> Exp [comm] .
    ----------------------------------

    op _?_:_ : Exp Exp Exp -> Exp [prec 50] .
    -----------------------------------------

    sorts NeIdList IdList NeExpList ExpList .
    -----------------------------------------
    subsorts Id  < NeIdList  < IdList .
    subsorts Exp < NeExpList < ExpList .
    subsort NeIdList < NeExpList .
    subsort IdList   < ExpList .

    op .IdList : -> IdList .
    op _,_     : IdList     IdList ->    IdList [assoc id: .IdList] .
    op _,_     : IdList   NeIdList ->  NeIdList [ditto] .
    op _,_     : ExpList   ExpList ->   ExpList [ditto] .
    op _,_     : ExpList NeExpList -> NeExpList [ditto] .
    -----------------------------------------------------

    op _(_) : Id ExpList -> Exp .
    -----------------------------

    op bernoulli  : Exp     -> Exp .
    op poisson    : Exp     -> Exp .
    op uniformInt : Exp Exp -> Exp .
    op uniform    : Exp Exp -> Exp .
    op gauss      : Exp Exp -> Exp .
    op pareto     : Exp Exp -> Exp .
    op beta       : Exp Exp -> Exp .
    op gamma      : Exp Exp -> Exp .
    --------------------------------

    sort Function .
    ---------------

    op fun_(_) =_ : Id IdList Exp  -> Function [prec 50] .
    ------------------------------------------------------

    sort Program .
    --------------
    subsort Function < Program .

    op __ : Program Program -> Program [assoc prec 70] .
    ----------------------------------------------------

    --- No imperative support yet
--- ;     sort Stmt .
--- ;     -----------
--- ;
--- ;     op skip :           -> Stmt .
--- ;     op _;_  : Stmt Stmt -> Stmt [assoc id: skip prec 55] .
--- ;     ------------------------------------------------------
--- ;
--- ;     op _:=_     : Id  Exp -> Stmt [prec 50] .
--- ;     op _=_      : Id  Exp -> Stmt [prec 50] .
--- ;     op observe_ : Exp     -> Stmt [prec 50] .
--- ;     op assert_  : Exp     -> Stmt [prec 50] .
--- ;     op return_  : Exp     -> Stmt [prec 50] .
--- ;     -----------------------------------------
--- ;
--- ;     op if_{_} else {_} : Exp Stmt Stmt -> Stmt .
--- ;     op for_in [_.._] {_} : Id Exp Exp Stmt -> Stmt .
--- ;     ------------------------------------------------
--- ;
--- ;     sort Procedure .
--- ;     ----------------
--- ;     subsort Procedure < Program .
--- ;
--- ;     op def _(_) {_} : Id IdList Stmt -> Procedure .
--- ;     -----------------------------------------------

    --- No array support yet
--- ;     op _[_] : Id  Exp -> Exp [prec 35] .
--- ;     op _:= array(_,_) : Id  Exp Exp -> Stmt [prec 50] .
--- ;     op _[_] =_        : Id  Exp Exp -> Stmt [prec 50] .

    --- No mariginalization yet
--- ;    ops Expectation FromMarginal SampleFrom : -> Sample .
--- ;     op _(_) : Sample ExpList -> Exp .   --- TODO: should be Exp or Stmt?
endfm
```

### PSI User to Internal Translation

```maude
fmod PSI-TRANSLATION is
   protecting PSI-USER .

    sorts Query Program? .
    ----------------------
    subsorts Query Program < Program? .

    op __ : Program? Program? -> Program? [ditto] .
    -----------------------------------------------

    op dist: _ : Exp -> Query [prec 50] .
    -------------------------------------

    op fv : DExp -> Id .
    --------------------

    sort TranslateEnv .
    -------------------

    var X : Id . vars DE DE' DE'' : DExp . var DC : DConst .
    vars E E' E'' : Exp . vars NeEL NeEL' : NeExpList . var EL : ExpList .
    vars TE TE' TE'' : TranslateEnv .

    op .TranslateEnv :                           -> TranslateEnv .
    op _;_           : TranslateEnv TranslateEnv -> TranslateEnv [assoc id: .TranslateEnv prec 55] .
    ------------------------------------------------------------------------------------------------

    op Assert : DExp -> TranslateEnv .
    ----------------------------------

    op {_} :         TranslateEnv -> TranslateEnv .
    op _|_ : ExpList TranslateEnv -> TranslateEnv [prec 60] .
    ---------------------------------------------------------
    eq { { TE } }        = { TE } .
    eq { .IdList  | TE } = .IdList     | TE .
   ceq { E , NeEL | TE } = DE' , NeEL' | TE''
                        if DE'   | TE'  := { E    | TE  }
                        /\ NeEL' | TE'' := { NeEL | TE' } .

    eq { X  | TE } = X  | TE .
    eq { DC | TE } = DC | TE .

   ceq {   -   E' | TE } =    - DE'                   | TE'                     if      DE' | TE' := {     E' | TE } .
   ceq { E +   E' | TE } = DE + DE'                   | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E -   E' | TE } = DE - DE'                   | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E *   E' | TE } = DE * DE'                   | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E /   E' | TE } = DE / DE'                   | TE' ; Assert([DE' ≠ 0]) if DE , DE' | TE' := { E , E' | TE } .
   ceq { E ^   E' | TE } = DE ^ DE'                   | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E &&  E' | TE } = [DE ≠ 0] * [DE' ≠ 0]       | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E ||  E' | TE } = [[DE ≠ 0] + [DE' ≠ 0] ≠ 0] | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E <   E' | TE } = [ DE < DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E >   E' | TE } = [ DE > DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E ≤   E' | TE } = [ DE ≤ DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E ≥   E' | TE } = [ DE ≥ DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E === E' | TE } = [ DE = DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .
   ceq { E ≠   E' | TE } = [ DE ≠ DE' ]               | TE'                     if DE , DE' | TE' := { E , E' | TE } .

   ceq { E ? E' : E'' | TE } = [ DE ≠ 0 ] * DE' + [ DE = 0 ] * DE'' | TE' if DE , DE' , DE'' | TE' := { E , E', E'' | TE } .

   ceq { [ E < E' ] | TE } = [ DE < DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .
   ceq { [ E > E' ] | TE } = [ DE > DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .
   ceq { [ E ≤ E' ] | TE } = [ DE ≤ DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .
   ceq { [ E ≥ E' ] | TE } = [ DE ≥ DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .
   ceq { [ E = E' ] | TE } = [ DE = DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .
   ceq { [ E ≠ E' ] | TE } = [ DE ≠ DE' ] | TE' if DE , DE' | TE' := { E , E' | TE } .

   ceq { ln(E)           | TE } = ln(DE)           | TE' ; Assert([DE > 0]) if DE | TE' := { E | TE } .
   ceq { sin(E)          | TE } = sin(DE)          | TE'                    if DE | TE' := { E | TE } .
   ceq { cos(E)          | TE } = cos(DE)          | TE'                    if DE | TE' := { E | TE } .
   ceq { floor(E)        | TE } = floor(DE)        | TE'                    if DE | TE' := { E | TE } .
   ceq { ceil(E)         | TE } = ceil(DE)         | TE'                    if DE | TE' := { E | TE } .
   ceq { δ(0)[E]         | TE } = δ(0)[DE]         | TE'                    if DE | TE' := { E | TE } .
   ceq { gaussAnti(X, E) | TE } = gaussAnti(X, DE) | TE'                    if DE | TE' := { E | TE } .

   ceq { bernoulli(E) | TE } = DE * dirac(1 - fv(DE)) + (1 - DE) * dirac(fv(DE)) | TE' ; Assert([0 ≤ DE] * [DE ≤ 1]) if DE | TE' := { E | TE } .
---   ceq { gauss(E, E')      | TE } = gauss(DE, DE')      | TE' if DE , DE' | TE' := { E , E' | TE } .
---   ceq { uniformInt(E, E') | TE } = uniformInt(DE, DE') | TE' if DE , DE' | TE' := { E , E' | TE } .

    op translate : ExpList -> TranslateEnv .
    ----------------------------------------
    eq translate(EL) = { EL | .TranslateEnv } .

    op translateExp : Exp -> [Exp] .
    --------------------------------
   ceq translateExp(E) = DE if DE | .TranslateEnv := translate(E) .
endfm
```