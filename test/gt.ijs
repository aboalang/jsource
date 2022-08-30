prolog './gt.ijs'

1: 0 : 0  NB. removed from 901
NB. t. ------------------------------------------------------------------

(%!i.5)        = ^       t. i.5
1 1 0 0 0      = >:      t. i.5
_1 1 0 0 0     = <:      t. i.5
0 _1 0 0 0     = -       t. i.5
1 _1 0 0 0     = -.      t. i.5
0 2 0 0 0      = +:      t. i.5
0 0.5 0 0 0    = -:      t. i.5
0 0 1 0 0      = *:      t. i.5
0 0j1 0 0 0    = j.      t. i.5
0 1p1 0 0 0    = o.      t. i.5     
_9 0 0 0 0     = _9:     t. i.5
_8 0 0 0 0     = _8:     t. i.5
_7 0 0 0 0     = _7:     t. i.5
_6 0 0 0 0     = _6:     t. i.5
_5 0 0 0 0     = _5:     t. i.5
_4 0 0 0 0     = _4:     t. i.5
_3 0 0 0 0     = _3:     t. i.5
_2 0 0 0 0     = _2:     t. i.5
_1 0 0 0 0     = _1:     t. i.5
0 0 0 0 0      = 0:      t. i.5
1 0 0 0 0      = 1:      t. i.5
2 0 0 0 0      = 2:      t. i.5
3 0 0 0 0      = 3:      t. i.5
4 0 0 0 0      = 4:      t. i.5
5 0 0 0 0      = 5:      t. i.5
6 0 0 0 0      = 6:      t. i.5
7 0 0 0 0      = 7:      t. i.5
8 0 0 0 0      = 8:      t. i.5
9 0 0 0 0      = 9:      t. i.5

17.5 0 0 0 0   = 17.5"0  t. i.5
  
3 1 0 0 0      = 3&+     t. i.5
_7.1 1 0 0 0   = +&_7.1  t. i.5
9 1 0 0 0      = -&_9    t. i.5
9 1 0 0 0      = -&_9    t. i.5
0 _1.3 0 0 0   = _1.3&*  t. i.5
0 3.21 0 0 0   = *&3.21  t. i.5
0 4 0 0 0      = %&0.25  t. i.5
3 0j1 0 0 0    = 3&j.    t. i.5
0j4 1 0 0 0    = j.&4    t. i.5
1 0 0 0 0      = ^&0     t. i.5
0 1 0 0 0      = ^&1     t. i.5
0 0 1 0 0      = ^&2     t. i.5
0 0 0 1 0      = ^&3     t. i.5
0 0 0 0 1      = ^&4     t. i.5
(y,0)          = y&p.    t. i.5 [  y=:_50+?4$100
(p. r)         = r&p.    t. i.5 [  r=:(>:?10);_5+?4$20

f=: ^&3 + (_3:**:) + (3:*]) + _1:
_1 3 _3 1 0    = f       t. i.5
f=: ^&3 + (_3 **:) + (3 *]) + _1:
_1 3 _3 1 0    = f       t. i.5

f =: ! %~ p.@<@i.
g =: 3 : 'y&! t. i.1+y'
(f -: g)"0 x: i.3 5

f =: i.@>: ! ]
g =: 3 : '(<y$_1)&p. t. i.@>:y'
(f -: g)"0 i.3 5

(3.2&^ = (3.2&^ t.i.20)&p.) x=:(2^_10)*?2 10$1000

s=: (2^_10)*_1000+?2000
c=: (2^ _4)*_10+?5$20
f=: c&(p.!.s)
x=: j./(2^_7)*_50+?2 2 10$100
1e_8 > (f x) %&|~ (f - f T. (#c)) x

'domain error' -: ex '1.2&! t.'
'domain error' -: ex 'p.&1  t.'

'rank error'   -: ex '(1 2 3"_ * *:) t.'
'rank error'   -: ex '(1 2 3   * *:) t.'

'length error' -: ex '1 2 3 t.'
'length error' -: ex '+`-`* t.'


NB.  f@g t. -------------------------------------------------------------

p=: (8 %~ _10+?4$20)&p.
q=: (8 %~ _10+?3$20)&p.
sin =: 1&o.

(p@q   =!.5e_11 p@q   T. 20) x=: (2^ _8)*_200+?30$400
(q@p   =!.5e_11 q@p   T. 20) x

(p@sin =!.5e_11 p@sin T. 40) x=: (2^_10)*_600+?30$1200
(q@sin =!.5e_11 q@sin T. 40) x
(p@:^  =!.5e_11 p@:^  T. 40) x
(q@:^  =!.5e_11 q@:^  T. 40) x


NB.  %@f t. -------------------------------------------------------------

pp =: [: +//. */
1e_11 > 1 -&(m&{.) p pp q=:%@(p&p.) t. i.m [ p=:>:?n$10      [ m=: +:n=: 5
1e_11 > 1 -&(m&{.) p pp q=:%@(p&p.) t. i.m [ p=:1,}._9+?n$19 [ m=: +:n=: >:?10

1 1 2 3 5 8 13 21 34 55 89   = %@(1 _1 _1&p.) t. i.11x

0 1 1 2 3 5 8 13 21 34 55 89 = (0 1&p. % 1 _1 _1&p.) t. i.12x
0 1 1 2 3 5 8 13 21 34 55 89 = (%-.-*:) t. i.12x

1 3 3 1 0 0 0 0 0 0 -: (1 5 10 10 5 1&p. % 1 2 1&p.) t. i.10

((n$1 _1)*^ t. i.n) = %@^ t. i.n=: 8

rp =: 1 : '%@(x&p.) t.'

(1   ^i.n) = 1 _1    rp i.n=:20
(2   ^i.n) = 1 _2    rp i.n
(_2  ^i.n) = 1  2    rp i.n
(2.71^i.n) = 1 _2.71 rp i.n
(0j1 ^i.n) = 1 0j_1  rp i.n
(c   ^i.n) = (1,-c)  rp i.n=:?20 [ c=:(_1^?2)*1+(2^_8)*?1e3

(n$1    ) = 1 _1   rp i.n=:20
(n$1 0  ) = 1 0 _1 rp i.n
(0=k|i.n) = (1,(-k){._1) rp i.n=:?40 [ k=:?20

(n{.1 ) = 1      rp i.n=:20
(n$1  ) = 1 _1   rp i.n
(>:i.n) = 1 _2 1 rp i.n
(+/\^:0 n{.1) = %@((1 _1&pp^:0 [ 1)&p.) t.i.n
(+/\^:1 n{.1) = %@((1 _1&pp^:1 [ 1)&p.) t.i.n
(+/\^:2 n{.1) = %@((1 _1&pp^:2 [ 1)&p.) t.i.n
(+/\^:k n{.1) = %@((1 _1&pp^:k [ 1)&p.) t.i.n=:?40 [ k=:?12

tangent=: 3 : 0  NB. tangent numbers from 0 to y  Tn+1(x)=(1+x^2)Tn'(x)
 f=. [: +//. 1 0 1"_ */ [: }. [ * i.@#
 {."1 f^:(i.>:y) 0 1x
()

B=: 3 : 0   NB. Bernoulli numbers from 0 to y
 t=. 1,}.}:tangent y
 (* $&_1 _1 1 1@#) _1,t*n*%(* <:)2x^n=. >:i.#t
()

(B@<:@# -: ! * (% <:@^) t.) i.13x
(B@<:@# -:     (% <:@^) t:) i.13x


NB.  %:@f t. ------------------------------------------------------------

taysqrt=: 4 : 0
 n=. x
 a=. (1+n){.y
 c=. n{.%:{.a
 d=. %2*{.c
 i=. 0
 while. n>i=.>:i do. c=. c i}~ d * (i{a) - (+/ .* |.) }.i{.c end.
 c
()

pp=: [: +//. */

NB. *** c=: _5+?7$20
NB. *** d=: %:@(c&p.) t. i.20
NB. *** d -: 20 taysqrt c
NB. *** c -:      (#c){. +//.@(*/)~ d
NB. *** 1e_12 > | (#c)}. 20{.+//.@(*/)~ d


NB.  f^:n t. ------------------------------------------------------------

(>:^: 3 t. -: >:@>:@>: t.) i.2 10
(>:^:_3 t. -: <:@<:@<: t.) i.2 10

f=: (1+?3$5)&p.
(f^:4 t. -: f@f@f@f t.) i.4 10


NB.  t. circle functions  -----------------------------------------------

sin   =: 1&o.
cos   =: 2&o.
sinh  =: 5&o.
cosh  =: 6&o.
asin  =: _1&o.
atan  =: _3&o.
asinh =: _5&o.
atanh =: _7&o.

0  1  0 _1  0  1  0 _1  0  1 -: sin                      t: i.10
0  1  0 _1  0  1  0 _1  0  1 -: -: (^ - ^&-) &.j.              t: i.10
0  1  0 _1  0  1  0 _1  0  1 -: (0j2"0 %~ ^@j. - ^@-@j.) t: i.10
0  1  0 _1  0  1  0 _1  0  1 -: (* '' H. 3r2@(_1r4&*)@*:)t: i.10

1  0 _1  0  1  0 _1  0  1  0 -: cos                      t: i.10
1  0 _1  0  1  0 _1  0  1  0 -: -: (^@j. + ^@j.&-)                t: i.10
1  0 _1  0  1  0 _1  0  1  0 -: (   2: %~ ^@j. + ^@-@j.) t: i.10
1  0 _1  0  1  0 _1  0  1  0 -: '' H. 1r2@(_1r4&*)@*:    t: i.10

0  1  0  1  0  1  0  1  0  1 -: sinh                     t: i.10
0  1  0  1  0  1  0  1  0  1 -: -: (^ - ^&-)                   t: i.10
0  1  0  1  0  1  0  1  0  1 -: (2: %~ ^ - ^@-)          t: i.10
0  1  0  1  0  1  0  1  0  1 -: (* '' H. 3r2@(1r4&*)@*:) t: i.10

1  0  1  0  1  0  1  0  1  0 -: cosh                     t: i.10
1  0  1  0  1  0  1  0  1  0 -: -: (^ + ^&-)                   t: i.10
1  0  1  0  1  0  1  0  1  0 -: (2: %~ ^ + ^@-)          t: i.10
1  0  1  0  1  0  1  0  1  0 -: '' H. 1r2@(1r4&*)@*:     t: i.10

1e_16 > | (20{.1) - (*:@cos  + *:@sin ) t. i.20
1e_16 > | (20{.1) - (*:@cosh - *:@sinh) t. i.20
1e_16 > | (20{.1) - ((cos ^2:) + (sin ^2:)) t. i.20
1e_16 > | (20{.1) - ((cosh^2:) - (sinh^2:)) t. i.20

(sin  = sin  T. 20) x=: (2^_6)*_50+?2 10$100
(cos  = cos  T. 20) x=: (2^_6)*_50+?2 10$100
(sinh = sinh T. 20) x=: (2^_6)*_50+?2 10$100
(cosh = cosh T. 20) x=: (2^_6)*_50+?2 10$100

(asin  =!.1e_12 asin  T. 20) x=: (2^_10)*?2 10$250
(atan  =!.1e_12 atan  T. 20) x=: (2^_10)*?2 10$250
(asinh =!.1e_12 asinh T. 20) x=: (2^_10)*?2 10$250
(atanh =!.1e_12 atanh T. 20) x=: (2^_10)*?2 10$250

d=: [: 1e_13&>@| - % (+0&=)@] 
x=: (2^_7)*_1e2+?20$2e2
(f T. 20 d f=:sin + cos) x
(f T. 20 d f=:sin - cos) x
(f T. 20 d f=:sin * cos) x
(f T. 20 d f=:cos * ^  ) x
(f T. 80 d f=:sin % cos) x

phi=: -:%:5
0 1 1 2 3 5 8 13 21 34 55 89 -: (^@-: * sinh&.(phi&*)) t: i.12


4!:55 ;:'asin asinh atan atanh B c cos cosh d f g k '
4!:55 ;:'m n p phi pp q r rp s sin sinh tangent taysqrt x y '

)

epilog''

