prolog './gq.ijs'
NB. rational numbers ----------------------------------------------------

NB. test failed on small ct
ct   =: 9!:18''
9!:19[2^_38

rat =: 128&=@type
xint=:  64&=@type
fl  =:   8&=@type
cmpx=:  16&=@type


NB. = -------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$5
(a=/b) -: a (   [ =/ x:@]) b
(a=/b) -: a (x:@[ =/    ]) b
(a=/b) -: a (x:@[ =/ x:@]) b

a=: _50+?13$100
b=: b,-b=:%/1+?2 20$5
(a=/b) -: a (   [ =/ x:@]) b
(a=/b) -: a (x:@[ =/    ]) b
(a=/b) -: a (x:@[ =/ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a= b) -: a (   [ =  x:@]) b
(a= b) -: a (x:@[ =     ]) b
(a= b) -: a (x:@[ =  x:@]) b
(a=/b) -: a (   [ =/ x:@]) b
(a=/b) -: a (x:@[ =/    ]) b
(a=/b) -: a (x:@[ =/ x:@]) b

1 0 1 -: 1r2 3r4 2r5 = 0.5 3j4 0.4
0 0 0 -: 1r2 = 'abc'
0 0 0 -: 1r2 = ;:'foo upon thee'


NB. < -------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a</b) -: a (   [ </ x:@]) b
(a</b) -: a (x:@[ </    ]) b
(a</b) -: a (x:@[ </ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a</b) -: a (   [ </ x:@]) b
(a</b) -: a (x:@[ </    ]) b
(a</b) -: a (x:@[ </ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a< b) -: a (   [ <  x:@]) b
(a< b) -: a (x:@[ <     ]) b
(a< b) -: a (x:@[ <  x:@]) b
(a</b) -: a (   [ </ x:@]) b
(a</b) -: a (x:@[ </    ]) b
(a</b) -: a (x:@[ </ x:@]) b

'domain error' -: 1r2  < etx 3j4
'domain error' -: 1r2  < etx 'a'
'domain error' -: 1r2  < etx <12
'domain error' -: 3j4  < etx 1r3
'domain error' -: 'a'  < etx 1r3
'domain error' -: (<12)< etx 1r3


NB. <. ------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a<./b) -: a (   [ <./ x:@]) b
(a<./b) -: a (x:@[ <./    ]) b
(a<./b) -: a (x:@[ <./ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a<./b) -: a (   [ <./ x:@]) b
(a<./b) -: a (x:@[ <./    ]) b
(a<./b) -: a (x:@[ <./ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a<. b) -: a (   [ <.  x:@]) b
(a<. b) -: a (x:@[ <.     ]) b
(a<. b) -: a (x:@[ <.  x:@]) b
(a<./b) -: a (   [ <./ x:@]) b
(a<./b) -: a (x:@[ <./    ]) b
(a<./b) -: a (x:@[ <./ x:@]) b

'domain error' -: 1r2  <. etx 3j4
'domain error' -: 1r2  <. etx 'a'
'domain error' -: 1r2  <. etx <12
'domain error' -: 3j4  <. etx 1r3
'domain error' -: 'a'  <. etx 1r3
'domain error' -: (<12)<. etx 1r3


NB. <: ------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a<:/b) -: a (   [ <:/ x:@]) b
(a<:/b) -: a (x:@[ <:/    ]) b
(a<:/b) -: a (x:@[ <:/ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a<:/b) -: a (   [ <:/ x:@]) b
(a<:/b) -: a (x:@[ <:/    ]) b
(a<:/b) -: a (x:@[ <:/ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a<: b) -: a (   [ <:  x:@]) b
(a<: b) -: a (x:@[ <:     ]) b
(a<: b) -: a (x:@[ <:  x:@]) b
(a<:/b) -: a (   [ <:/ x:@]) b
(a<:/b) -: a (x:@[ <:/    ]) b
(a<:/b) -: a (x:@[ <:/ x:@]) b

'domain error' -: 1r2  <: etx 3j4
'domain error' -: 1r2  <: etx 'a'
'domain error' -: 1r2  <: etx <12
'domain error' -: 3j4  <: etx 1r3
'domain error' -: 'a'  <: etx 1r3
'domain error' -: (<12)<: etx 1r3


NB. > -------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a>/b) -: a (   [ >/ x:@]) b
(a>/b) -: a (x:@[ >/    ]) b
(a>/b) -: a (x:@[ >/ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a>/b) -: a (   [ >/ x:@]) b
(a>/b) -: a (x:@[ >/    ]) b
(a>/b) -: a (x:@[ >/ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a>/b) -: a (   [ >/ x:@]) b
(a>/b) -: a (x:@[ >/    ]) b
(a>/b) -: a (x:@[ >/ x:@]) b

'domain error' -: 1r2  > etx 3j4
'domain error' -: 1r2  > etx 'a'
'domain error' -: 1r2  > etx <12
'domain error' -: 3j4  > etx 1r3
'domain error' -: 'a'  > etx 1r3
'domain error' -: (<12)> etx 1r3


NB. >. ------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a>./b) -: a (   [ >./ x:@]) b
(a>./b) -: a (x:@[ >./    ]) b
(a>./b) -: a (x:@[ >./ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a>./b) -: a (   [ >./ x:@]) b
(a>./b) -: a (x:@[ >./    ]) b
(a>./b) -: a (x:@[ >./ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a>./b) -: a (   [ >./ x:@]) b
(a>./b) -: a (x:@[ >./    ]) b
(a>./b) -: a (x:@[ >./ x:@]) b

'domain error' -: 1r2  >. etx 3j4
'domain error' -: 1r2  >. etx 'a'
'domain error' -: 1r2  >. etx <12
'domain error' -: 3j4  >. etx 1r3
'domain error' -: 'a'  >. etx 1r3
'domain error' -: (<12)>. etx 1r3


NB. >: ------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$20
(a>:/b) -: a (   [ >:/ x:@]) b
(a>:/b) -: a (x:@[ >:/    ]) b
(a>:/b) -: a (x:@[ >:/ x:@]) b

a=: _5000+?13$10000
b=: b,-b=:%/1+?2 20$20
(a>:/b) -: a (   [ >:/ x:@]) b
(a>:/b) -: a (x:@[ >:/    ]) b
(a>:/b) -: a (x:@[ >:/ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a>: b) -: a (   [ >:  x:@]) b
(a>: b) -: a (x:@[ >:     ]) b
(a>: b) -: a (x:@[ >:  x:@]) b
(a>:/b) -: a (   [ >:/ x:@]) b
(a>:/b) -: a (x:@[ >:/    ]) b
(a>:/b) -: a (x:@[ >:/ x:@]) b

'domain error' -: 1r2  >: etx 3j4
'domain error' -: 1r2  >: etx 'a'
'domain error' -: 1r2  >: etx <12
'domain error' -: 3j4  >: etx 1r3
'domain error' -: 'a'  >: etx 1r3
'domain error' -: (<12)>: etx 1r3


NB. + -------------------------------------------------------------------

 11r6 -:  1r2 +  4r3
 _5r6 -:  1r2 + _4r3
  5r6 -: _1r2 +  4r3
_11r6 -: _1r2 + _4r3

(fl  y) *. 6 -: y=: 5r2 + 3.5
(fl  y) *. 6 -: y=: 2.5 + 7r2
(rat y) *. 6 -: y=: 5r2 + 7r2

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20

*./ 1e_14 > , (a+/b) - a (x:@[ +/ x:@]) b
*./ 1e_14 > , (a+/b) - a (   [ +/ x:@]) b
*./ 1e_14 > , (a+/b) - a (x:@[ +/    ]) b

(+/%x) -: +/ % x: x=:1+i.12

'domain error' -: 1r2  + etx 'a'
'domain error' -: 1r2  + etx <12
'domain error' -: 'a'  + etx 1r3
'domain error' -: (<12)+ etx 1r3


NB. +. ------------------------------------------------------------------
NB. +: ------------------------------------------------------------------

NB. * -------------------------------------------------------------------

 2r3 -:  1r2 *  4r3
_2r3 -:  1r2 * _4r3
_2r3 -: _1r2 *  4r3
 2r3 -: _1r2 * _4r3

(fl  y) *. 8.75 -: y=: 5r2 * 3.5
(fl  y) *. 8.75 -: y=: 2.5 * 7r2
(rat y) *. 35r4 -: y=: 5r2 * 7r2

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20

(a*/b) -: a (x:@[ */ x:@]) b
(a*/b) -: a (   [ */ x:@]) b
(a*/b) -: a (x:@[ */    ]) b

1e_16 > | (*/%x) - */ % x: x=:1+i.9

'domain error' -: 1r2  * etx 'a'
'domain error' -: 1r2  * etx <12
'domain error' -: 'a'  * etx 1r3
'domain error' -: (<12)* etx 1r3


NB. *. ------------------------------------------------------------------
NB. *: ------------------------------------------------------------------

NB. - -------------------------------------------------------------------

 _5r6 -:  1r2 -  4r3
 11r6 -:  1r2 - _4r3
_11r6 -: _1r2 -  4r3
  5r6 -: _1r2 - _4r3

(fl  y) *. _1 -: y=: 5r2 - 3.5
(fl  y) *. _1 -: y=: 2.5 - 7r2
(rat y) *. _1 -: y=: 5r2 - 7r2

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20

*./ 1e_14 > , (a-/b) - a (x:@[ -/ x:@]) b
*./ 1e_14 > , (a-/b) - a (   [ -/ x:@]) b
*./ 1e_14 > , (a-/b) - a (x:@[ -/    ]) b

(-/%x) -: -/ % x: x=:1+i.12

'domain error' -: 1r2  - etx 'a'
'domain error' -: 1r2  - etx <12
'domain error' -: 'a'  - etx 1r3
'domain error' -: (<12)- etx 1r3


NB. % -------------------------------------------------------------------

 3r8 -:  1r2 %  4r3
_3r8 -:  1r2 % _4r3
_3r8 -: _1r2 %  4r3
 3r8 -: _1r2 % _4r3

(fl  y) *. (5%7) -: y=: 5r2 % 3.5
(fl  y) *. (5%7) -: y=: 2.5 % 7r2
(rat y) *.  5r7  -: y=: 5r2 % 7r2

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20

(a%/b) -:!.1e_13 a (x:@[ %/ x:@]) b
(a%/b) -:!.1e_13 a (   [ %/ x:@]) b
(a%/b) -:!.1e_13 a (x:@[ %/    ]) b

(%/%x) -: %/ % x: x=:1+i.12

0  =  0 % 0r1
_  =  4 % 0r1
__ = _4 % 0r1

128 = type 0r1
128 = type 0 % 0r1
(4%x+2.5-2.5) -: 4 % x=:1 2 3 4 0r1
128 = type 4 % x

'domain error' -: 1r2  % etx 'a'
'domain error' -: 1r2  % etx <12
'domain error' -: 'a'  % etx 1r3
'domain error' -: (<12)% etx 1r3


NB. %. ------------------------------------------------------------------

Hilbert=: x: @: % @: >: @: (+/~) @: i.

(=i.#h) -: h +/ .* %. h=: Hilbert 5
(=i.#h) -: h +/ .* %. h=: Hilbert 6
(=i.#h) -: h +/ .* %. h=: Hilbert 7

(%. -: %.@:x:) i.8
(%. -: %.@:x:) ,8
(%. -: %.@:x:) 8
(%. -: %.@:x:) i.0
1e_7 > >./| , (%. - %.@:x:) x=:_50+?7 7$100
(-: %.@%.) x: x
1e_7 > >./| , (%. - %.@:x:) x=:_50+?7 5$100
   
'domain error' -: %. etx 3 3$1r2


NB. %: ------------------------------------------------------------------

(%: 2.5) -: %: 5r2
(%:_2.5) -: %:_5r2
(%: 25 ) -: %: 25r1

(%:_1  ) -: %: _1r1
(%:_0.5) -: %: _1r2

(3 %: 8) -: 3 %: 8r1

rat %: *: 7r2
7r2 -: %: *: 7r2
(%: 3.5) -: %: 7r2
(%:_3.5) -: %:_7r2


NB. ^ -------------------------------------------------------------------

(^  2.5) -: ^  5r2
(^ _2.5) -: ^ _5r2

(rat x) *. 0r1 = x=: 0r1 ^ 1
(rat x) *. 0r1 = x=: 0r1 ^ 5
(rat x) *. 0r1 = x=: 0r1 ^ 5x
(xint x) *. 0r1 = x=: 0x ^ 1
(xint x) *. 0r1 = x=: 0x ^ 5
(xint x) *. 0r1 = x=: 0x ^ 5x
(rat  x) *. 0r1 = x=: 0r1 ^ 5r2
(rat  x) *. 0r1 = x=: 0r1 ^ 1 2 3 5r2

(fl  x) *. 0   = x=: 0r1 ^ 1p1

(rat x) *. 1r1 = x=: 0r1 ^ 0
(rat x) *. 1r1 = x=: 0r1 ^ 0x
(rat x) *. 1r1 = x=: 0r1 ^ 0r1
(xint x) *. 1r1 = x=: 0x ^ 0
(xint x) *. 1r1 = x=: 0x ^ 0x
(rat x) *. 1r1 = x=: 0x ^ 0r1

(xint x) *. 1r1 = x=: 1x ^ 5
(xint x) *. 1r1 = x=: 1x ^ 5x
(rat  x) *. 1r1 = x=: 1x ^ 5r2
(xint x) *. 1r1 = x=: 1x ^ _5
(rat  x) *. 1r1 = x=: 1x ^ _5r2
(xint x) *. 1r1 = x=: 1x ^ 0
(xint x) *. 1r1 = x=: 1x ^ _5+i.11
(rat x) *. 1r1 = x=: 1r1 ^ 5
(rat x) *. 1r1 = x=: 1r1 ^ 5x
(rat  x) *. 1r1 = x=: 1r1 ^ 5r2
(rat x) *. 1r1 = x=: 1r1 ^ _5
(rat  x) *. 1r1 = x=: 1r1 ^ _5r2
(rat x) *. 1r1 = x=: 1r1 ^ 0
(rat x) *. 1r1 = x=: 1r1 ^ _5+i.11

(fl   x) *. 1   = x=: 1r1 ^ 1p1

_ -: 0r1 ^ _5r1
_ -: 0r1 ^ _5r2 
0 0 0 0 _ -: 0r1 ^ 1 2 3 4 _5r1
0 0 0 0 _ -: 0r1 ^ 1 2 3 4 _5r2

stope=: 1 : 0
 :
 */x+m*i.y
)

(2   ^!.1r2 i.10) -: 2   (1r2 stope)"0 i.10 
(2   ^!.1r2 i.10) -: 2   ^!.0.5        i.10 
(3r4 ^!.0.5 i.10) -: 3r4 (0.5 stope)"0 i.10
(3r4 ^!.0.5 i.10) -: 0.75 ^!.0.5       i.10


NB. $ -------------------------------------------------------------------

(12  $'abcd') -:  12r1   $ 'abcd'
(12 3$'abcd') -: (12 3r1)$ 'abcd'

'domain error' -: 12r7 $ etx 'abcd'
'domain error' -: 3 12r7 $ etx 'abcd'


NB. ~. ------------------------------------------------------------------

NB. ~: ------------------------------------------------------------------

a=: ?13$2
b=: b,-b=:%/1+?2 20$5
(a~:/b) -: a (   [ ~:/ x:@]) b
(a~:/b) -: a (x:@[ ~:/    ]) b
(a~:/b) -: a (x:@[ ~:/ x:@]) b

a=: _50+?13$100
b=: b,-b=:%/1+?2 20$5
(a~:/b) -: a (   [ ~:/ x:@]) b
(a~:/b) -: a (x:@[ ~:/    ]) b
(a~:/b) -: a (x:@[ ~:/ x:@]) b

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20
(a~: b) -: a (   [ ~:  x:@]) b
(a~: b) -: a (x:@[ ~:     ]) b
(a~: b) -: a (x:@[ ~:  x:@]) b
(a~:/b) -: a (   [ ~:/ x:@]) b
(a~:/b) -: a (x:@[ ~:/    ]) b
(a~:/b) -: a (x:@[ ~:/ x:@]) b

0 1 0 -: 1r2 3r4 2r5 ~: 0.5 3j4 0.4
1 1 1 -: 1r2 ~: 'abc'
1 1 1 -: 1r2 ~: ;:'foo upon thee'


NB. | -------------------------------------------------------------------

a=: a,-a=:%/1+?2 20$20
b=: b,-b=:%/1+?2 20$20

(| x: a) -: x: | a
(|    a) -: | &. x: a

1e_13> |, (x: a|/b) - a |/&:x: b
1e_13> |, (   a|/b) - a |/&.x: b

x -: 0r1 | x=:0r1 _5r2 5r2 1234567890123456789r7777 _1234567890123456789r7777


NB. . -------------------------------------------------------------------

(-/ .* -: -/ .*@:x:) x=:_500+?  3 3$1000
(-/ .* -: -/ .*@:x:) x=: %/1+?2 3 3$1000

(+/ .* -: +/ .*@:x:) x=:_500+?  3 3$1000
(+/ .* -: +/ .*@:x:) x=: %/1+?2 3 3$1000

Hilbert=: x: @: % @: >: @: (+/~) @: i.
f      =: i.&.(p:^:_1)@+: 
g      =: ~.@q:@%@(-/ .*)@Hilbert

(f -: g)"0 i.4 5


NB. : -------------------------------------------------------------------

f=: 3 : 0
 if. y do. 'ja' else. 'nein' end.
)

'ja'   -: f 3r1
'ja'   -: f 1r777777777 0 0 0
'nein' -: f 0r1 5r1


NB. , -------------------------------------------------------------------

(rat  x) *. 512 3r2 -: x=: 512,3r2  
(rat  x) *. 5r2 3r1 -: x=: 5r2,3    
(rat  x) *. 5r1 3r2 -: x=: 5x ,3r2  
(rat  x) *. 5r2 3r1 -: x=: 5r2,3x  
(rat  x) *. 5r2 3r1 -: x=: 5r2,3  
(rat  x) *. 5r2 3r4 -: x=: 5r2,3r4

(fl   x) *. 2.5 3.4          -: x=: 5r2,3.4
(fl   x) *. 2.5 3.4          -: x=: 2.5,17r5
(fl   x) *. 1 2 3.4 2.5 _0.2 -: x=: 1 2 3.4, 5r2 _1r5
(fl   x) *. 2.5 _0.2 1 2 3.4 -: x=: 1 2 3.4,~5r2 _1r5

(cmpx x) *. 2.5 3j4          -: x=: 5r2,3j4
(cmpx x) *. 2j5 3.4          -: x=: 2j5,17r5
(cmpx x) *. 1 2 3j4 2.5 _0.2 -: x=: 1 2 3j4, 5r2 _1r5
(cmpx x) *. 2.5 _0.2 1 2 3j4 -: x=: 1 2 3j4,~5r2 _1r5

'domain error' -: 5r2   , etx 'abc'
'domain error' -: 'abc' , etx 5r2
'domain error' -: 5r2   , etx <'x'
'domain error' -: (<'x'), etx 5r2


NB. # -------------------------------------------------------------------

(2#y) -: 2r1#y=:?10$10000
(x#1r2 _3r4) -: x: x#0.5 _0.75 [ x=:?2$1000

'domain error' -: 1r2 # etx 3 4 5


NB. #. ------------------------------------------------------------------
NB. #: ------------------------------------------------------------------
NB. ! -------------------------------------------------------------------

120x    -: ! 5r1
(!i.10) -: ! 0r1 1r1 2r1 3r1 4r1 5r1 6r1 7r1 8r1 9r1

(!2.5)  -: ! 5r2


NB. 3!:x ----------------------------------------------------------------

128 = type 1r2 3r4

ir =: 3!:1
ri =: 3!:2
hex=: 3!:3

x -: ri ir  x=: %/*: x:1+?2 4 5$1000000
x -: ri hex x


NB. /: ------------------------------------------------------------------

(0.66;2r3;0.67) -: /:~ 2r3; 0.66 ; 0.67


NB. \: ------------------------------------------------------------------

(0.67;2r3;0.66) -: \:~ 2r3; 0.66 ; 0.67


NB. { -------------------------------------------------------------------

({x;y) -: x ,&.>/ y [ x=:0 1 0 1 [ y=: 5r2 _1r5
({x;y) -: x ,&.>/ y [ x=:1 2 314 [ y=: 5r2 _1r5
({x;y) -: x ,&.>/ y [ x=:1 2 31x [ y=: 5r2 _1r5
({x;y) -: x ,&.>/ y [ x=:1 2 3.4 [ y=: 5r2 _1r5
({x;y) -: x ,&.>/ y [ x=:1 2 3j4 [ y=: 5r2 _1r5

'domain error' -: { etx 1r2 3r4 ; 'abc'
'domain error' -: { etx 1r2 3r4 ; <1;2;3


NB. ". ------------------------------------------------------------------
NB. extended integer comparisons ----------------------------------------
NB. A. ------------------------------------------------------------------
NB. e. ------------------------------------------------------------------


NB. i. ------------------------------------------------------------------

(i.5) -: i. 5r1

'domain error' -: i. etx 5r2
'domain error' -: i. etx 12345678901234567890r1


x=:?1000$1000
y=: (1000?1000){x

(x i. x) -: x i.&:x: x
(x i. y) -: x i.&:x: y

x=:?1000 3$1000
y=: (1000?1000){x

(x i. x) -: x i.&:x: x
(x i. y) -: x i.&:x: y


NB. o. ------------------------------------------------------------------

0 = (  o. 2%3) -   o. 2r3

0 = (1 o. 2%3) - 1 o. 2r3
0 = (2 o. 2%3) - 2 o. 2r3
0 = (3 o. 2%3) - 3 o. 2r3
0 = (4 o. 2%3) - 4 o. 2r3
0 = (5 o. 2%3) - 5 o. 2r3
0 = (6 o. 2%3) - 6 o. 2r3
0 = (7 o. 2%3) - 7 o. 2r3


NB. p. ------------------------------------------------------------------

r=: 1r2 2 4
c=: _4 11 _13r2 1r1

(rat  y) *.      c  -: y=:p.    <r
(rat  y) *. (314*c) -: y=:p. 314;r
(fl   y) *. (3.4*c) -: y=:p. 3.4;r
(cmpx y) *. (3j4*c) -: y=:p. 3j4;r

0r1 = c    p. r
0r1 = (<r) p. r

'domain error' -: p. etx 'a';r
'domain error' -: p. etx 1r2;'abc'
'domain error' -: p. etx (u:'a');r
'domain error' -: p. etx 1r2;u:'abc'
'domain error' -: p. etx (10&u:'a');r
'domain error' -: p. etx 1r2;10&u:'abc'

1 2 3 4 (p.!.1r2  -: p.!.0.5) 5
1 2 3 4 (p.!._2r1 -: p.!._2 ) 5


NB. q: ------------------------------------------------------------------

(q: x) -: q: x: x=:?1e9

'domain error' -: q: etx 5r2
'domain error' -: q: etx _9r1


9!:19 ct

4!:55 ;:'a b c cmpx ct f g fl h hex Hilbert ir'
4!:55 ;:'r rat ri stope x xint y'



epilog''

