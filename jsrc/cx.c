/* Copyright (c) 1990-2024, Jsoftware Inc.  All rights reserved.           */
/* Licensed use only. Any other use is in violation of copyright.          */
/*                                                                         */
/* Conjunctions: Explicit Definition : and Associates                      */

/* Usage of the f,g,h fields of : defined verbs:                           */
/*  f  character matrix of  left argument to :                             */
/*  g  character matrix of right argument to :                             */
/*  h  4-element vector of boxes                                           */
/*       0  vector of boxed tokens for f                                   */
/*       1  vector of triples of control information                       */
/*       2  vector of boxed tokens for g                                   */
/*       3  vector of triples of control information                       */

#include "j.h"
#include "d.h"
#include "p.h"
#include "w.h"

// DD definitions
#define DDBGN (US)('{'+256*'{')  // digraph for start DD
#define DDEND (US)('}'+256*'}')  // digraph for end DD
#define DDSEP 0xa  // ASCII value used to mark line separator inside 9 : string.  Must have class CU so that it ends a final comment

#define BASSERT(b,e)   {if(unlikely(!(b))){jsignal(e); z=0; goto bodyend;}}
#define BZ(e)          {if(unlikely(!(e))){z=0; goto bodyend;}}

#if SY_64
#define CWSENTX (cwgroup>>32)  // .sentx
#else
#define CWSENTX (cw[i].ig.indiv.sentx)
#endif

// sv->h is the A block for the [2][4] array of saved info for the definition; hv->[4] boxes of info for the current valence;
// line-> box 0 - tokens; x->box 1 - A block for control words; n (in flag word)=#control words; cw->array of control-word data, a CW struct for each
#define LINE(sv)       {A x; \
                        hv=AAV(sv->fgh[2])+4*((nGpysfctdl>>6)&1);  \
                        line=AAV(hv[0]); x=hv[1]; nGpysfctdl&=65535; nGpysfctdl|=(AN(x)<<16); cw=(CW*)AV(x);}

// Parse/execute a line, result in z.  If locked, reveal nothing.  Save current line number in case we reexecute
// If the sentence passes a u/v into an operator, the current symbol table will become the prev and will have the u/v environment info
// If the sentence fails, we go into debug mode and don't return until the user releases us
#if NAMETRACK
#define SETTRACK mvc(sizeof(trackinfo),trackinfo,1,iotavec-IOTAVECBEGIN+' '); wx=0; \
 wlen=sprintf(trackinfo,"%d: ",cw[i].source); wx+=wlen; trackinfo[wx++]=' '; \
 AK(trackbox)=(C*)queue-(C*)trackbox; AN(trackbox)=AS(trackbox)[0]=m; trackstg=unparse(trackbox); \
 wlen=AN(trackstg); wlen=wlen+wx>sizeof(trackinfo)-1?sizeof(trackinfo)-1-wx:wlen; MC(trackinfo+wx,CAV(trackstg),wlen); wx+=wlen; \
 trackinfo[wx]=0;  // null-terminate the info
#else
#define SETTRACK
#endif

#define PUSHTRYSTK ((jt->uflags.trace&TRACEDB)|((jt->emsgstate&EMSGSTATETRAPPING)>>(EMSGSTATETRAPPINGX-TRACEPMX)))  // value to save before a push
#define POPTRYSTK(v) {jt->uflags.trace=(jt->uflags.trace&~TRACEDB)|((v)&TRACEDB); jt->emsgstate=(jt->emsgstate&~EMSGSTATETRAPPING)|(((v)<<(EMSGSTATETRAPPINGX-TRACEPMX))&EMSGSTATETRAPPING);}   // restore value saved before push
// obsolete #define POPORIGTRYSTK {jt->uflags.trace=(jt->uflags.trace&~TRACEDB)|((UC)(nGpysfctdl>>8)&TRACEDB); jt->emsgstate=(jt->emsgstate&~EMSGSTATETRAPPING)|((UC)(nGpysfctdl>>(8+TRACEPMX-EMSGSTATETRAPPINGX))&EMSGSTATETRAPPING); nGpysfctdl^=4;}
#define POPIFTRYSTK if(unlikely(nGpysfctdl&4)){--tdv; POPTRYSTK(tdv->trap) if(likely(tdv->ndx==0))nGpysfctdl&=~4;}  // pop within try. stack; if pop to empty, clear flag
#define NOUNERR(t,ti) \
    /* Signal post-exec error*/ \
    {t=pee(line,&cw[ti],EVNONNOUN,nGpysfctdl<<(BW-2),callframe); \
    /* go to error loc; if we are in a try., send this error to the catch.  z may be unprotected, so clear it, to 0 if error shows, mtm otherwise */ \
    i=cw[ti].go; if(unlikely(i<SMAX)){ RESETERR; z=mtm; cv=forpopgoto(jt,cv,i,1); POPIFTRYSTK}else z=0; \
    goto nextline;}

#define CHECKNOUN if (unlikely(!(NOUN&AT(t))))NOUNERR(t,ti)   /* error, T block not creating noun */ \

// run one line.  If we see break request, accept it as ATTN but promote it to BREAK in other cores if debug off
// if debug mode/perfmon, call parsex which will check for stops and go into suspension on error
#define parseline(z,lbl) {S attnval=__atomic_load_n((S*)JT(jt,adbreakr),__ATOMIC_ACQUIRE); A *queue=line+CWSENTX; I m=(cwgroup>>16)&0xffff; \
 SETTRACK \
 if(likely(!(attnval+(nGpysfctdl&128+16))))z=parsea(queue,m);else {if(thisframe)thisframe->dclnk->dcix=i; z=parsex(queue,m,cw+i,(nGpysfctdl&128+16)?callframe:0);} \
 if(likely(z!=0)){I zasgn=PARSERASGN(z); z=PARSERVALUE(z); if(unlikely(!((AT(z)|zasgn)&NOUN))){if(!(AT(self)&ADV+CONJ)||((UI)(i+1)<(UI)(nGpysfctdl>>16)&&cw[i+1].ig.group[0]&0x200)) \
   if(jtdeprecmsg(jt,~7,"(007) noun result was required\n")==0)NOUNERR(z,i); \
 }} /* puns that ASGN flag is a NOUN type.  Err if can't be result, or if this is not a modifier */ \
 }

// for./select. control structure
typedef struct CDATA {
 A fchn;  // pointer to next allocated block in stack, 0 if end of stack
 struct CDATA *bchn;  // pointer to previous allocated block, possibly the local block.  0 in the local block itself
 A t;  // iteration array for for_xyz., select. value, or nullptr for for.
 A item;  // if for_xyz, the virtual block we are using to hold the value
 I j;  // iteration index
 I niter;  // for for. and for_xyz., number of iterations (number of items in T block)
 I itemsiz;  // size of an item of xyz, in bytes
 I4 w; // cw code for the structure
 LX itemsym;  // symbol number of xyz, 0 for for. or select.
 LX indexsym;  // symbol number of xyz_index, 0 for for. or select.
 US i;  // cw index of the start of the structure
 US go;  // cw index of the end. of the structure
} CDATA;

typedef struct{I4 b,e;C ndx; C trap;} TD;  // cw#s of try., end.; index of entry; emsgstate flag for trapping
// obsolete #define WTD            (sizeof(TD)/sizeof(I))
#define NTD            17     /* maximum nesting for try/catch */

// called from for. or select. to start filling in the entry
static B forinitnames(J jt,CDATA*cv,I cwtype,A line,I i, I go){
 cv->j=-1;                               /* iteration index     */
 cv->t=0;  // init no selector value/iterator list
 cv->w=cwtype;  // remember type of control struct
 cv->i=i, cv->go=go;  // remember start/end line#s of control struct
 if(cwtype==CFOR){
  // for for_xyz., get the symbol indexes for xyz & xyz_index
  I k=AN(line)-5;  /* length of item name; -1 if omitted (for.; for_. not allowed) */
  if(k>0){A x;  // if it is a for_xyz.
   // We need a string buffer for "xyz_index".  Use the stack if the name is short
   C ss[20], *s; if(unlikely(k>(I)(sizeof(ss)-6))){GATV0(x,LIT,k+6,1); s=CAV1(x);}else s=ss;  // s point to buffer
   MC(s,CAV(line)+4,k);  MC(s+k,"_index",6L);  // move "xyz_index" into *s
   cv->itemsym=(probeislocal(nfs(k,s)))-SYMORIGIN;  // get index of symbol in table, which must have been preallocated
   L *indexl; cv->indexsym=(indexl=probeislocal(nfs(k+6,s)))-SYMORIGIN;
   if(unlikely(k>(I)(sizeof(ss)-6))){ACINITZAP(x); fr(x);}  // remove tpop and free, now that we're done.  We may be in a loop 
   // Make initial assignment to xyz_index, and mark it readonly
   // Since we remove the readonly at the end of the loop, the user might have changed our value; so if there is
   // an incumbent value, we remove it.  We also zap the value we install, just as in any normal assignment
   ASSERT(!(indexl->flag&LREADONLY),EVRO)  // it had better not be readonly now
   fa(indexl->val);  // if there is an incumbent value, discard it
   A xx; GAT0(xx,INT,1,0); IAV0(xx)[0]=-1;  // -1 is the iteration number if there are no iterations
   ACINITZAP(xx); indexl->val=xx; indexl->valtype=ATYPETOVALTYPE(INT); // raise usecount, install as value of xyz_index
   indexl->flag|=LREADONLY;  // in the loop, the user may not modify xyz_index
  }else{cv->itemsym=cv->indexsym=0;}  // if not for_xyz., indicate with 0 indexes
 }
 R 1;  // normal return
}

// called to init the iterator for for.
static B jtforinit(J jt,CDATA*cv,A t){A x;C*s,*v;I k;
 ASSERT(t!=0,EVEMPTYDD);
 SETIC(t,cv->niter);                            /* # of items in t     */
 if(likely(cv->indexsym!=0)){
  // for_xyz.   protect iterator value and save it; create virtual item name
  ASSERT(!ISSPARSE(AT(t)),EVNONCE)
  rifv(t);  // it would be work to handle virtual t, because you can't just ra() a virtual, as virtuals are freed only from the tpop stack.  So we wimp out & realize.  note we can free from a boxed array now
  ra(t) cv->t=t;  // if we need to save iteration array, do so, and protect from free
  // create virtual block for the iteration.  We will store this in xyz.  We have to do usecount by hand because
  // true virtual blocks are freed only by tpop or from free of a boxed array, and we will be freeing this in unstackcv, either normally or at end-of-definition
  // We must keep ABACK in case we create a virtual block from xyz.
  // We store the block in 2 places: cv and symp.val.  We ra() once for each place
  // If there is an incumbent value, discard it
  L *asym=&SYMORIGIN[cv->itemsym]; A val=asym->val;  // stored reference address; incumbent value there
  fa(val); asym->val=0; asym->valtype=0;   // free the incumbent if any, clear val in symbol in case of error
  // Calculate the item size and save it
  I isz; I r=AR(t)-((UI)AR(t)>0); PROD(isz,r,AS(t)+1); I tt=AT(t); cv->itemsiz=isz<<bplg(tt); // rank of item; number of bytes in an item
  // Allocate a virtual block.  Zap it, fill it in, make noninplaceable.  Point it to the item before the data, since we preincrement in the loop
  A *pushxsave = jt->tnextpushp; jt->tnextpushp=&asym->val; A svb=virtual(t,0,r); jt->tnextpushp=pushxsave;  // since we can't ZAP a virtual, allocate this offstack to take ownership
  RZ(svb) AK(svb)=(CAV(t)-(C*)svb)-cv->itemsiz; ACINIT(svb,2); AN(svb)=isz; MCISH(AS(svb),AS(t)+1,r)  // AC=2 since we store in symbol and cv
  // Install the virtual block as xyz, and remember its address
  cv->item=svb; asym->valtype=ATYPETOVALTYPE(tt);  // save in 2 places (already in asym->val), commensurate with AC of 2
 }
 R 1;
}    /* for. do. end. initializations */

// A for/select. block is ending.   Free the iteration array.  Don't delete any names.  Mark the index as no longer readonly (in case we start the loop again).
// If assignvirt is set (normal), the xyz value is realized and reassigned if it is still the svb.  Otherwise it is freed and the value expunged.
// result is address of cv after stack popped
static CDATA* jtunstackcv(J jt,CDATA*cv,I assignvirt){
 if(cv->w==CFOR){
  if(cv->t){  // if for_xyz. that has processed forinit ...
   SYMORIGIN[cv->indexsym].flag&=~LREADONLY;  // set xyz_index is no longer readonly.  It is still available for inspection
   // If xyz still points to the virtual block, we must be exiting the loop early: the value must remain, so realize it
   A svb=cv->item;  // the virtual block for the item
   if(unlikely(SYMORIGIN[cv->itemsym].val==svb)){A newb;   // loop did not complete, and xyz has not been reassigned
    fa(svb);   // remove svb from itemsym.val.  Safe, because it can't be the last free
    if(likely(assignvirt!=0)){RZ(newb=realize(svb)); ACINITZAP(newb); ra00(newb,AT(newb)); SYMORIGIN[cv->itemsym].val=newb; SYMORIGIN[cv->itemsym].valtype=ATYPETOVALTYPE(AT(newb)); // realize stored value, raise, make recursive, store in symbol table
    }else{SYMORIGIN[cv->itemsym].val=0; SYMORIGIN[cv->itemsym].valtype=0;}  // after error, we needn't bother with a value
   }
   // Decrement the usecount to account for being removed from cv - this is the final free of the svb, unless it is a result.  Since this is a virtual block, free the backer also
   if(AC(svb)<=1)fa(ABACK(svb)); fr(svb);  // MUST NOT USE fa() for svb so that we don't recur and free svb's current contents in cv->t - svb is virtual
  }
 }
 fa(cv->t);  // decr the for/select value, protected at beginning.  NOP if it is 0
 cv=cv->bchn;  // go back to previous stack level
 R cv;
}

// call here when we find that xyz_index has been aliased.  We remove it, free it, and replace it with a new block.  Return 0 if error
// We do not have to change the valtype field, since it never changes
static A swapitervbl(J jt,A old,A *valloc){
 fa(old);  // discard the old value
 GAT0(old,INT,1,0);
 ACINITZAP(old); *valloc=old;  // raise usecount, install as value of xyz_index
 R old;
}

#if 0  // obsolete
// When try. in encountered, scan forward to find the other components
static void jttryinit(J jt,TD*v,I i,CW*cw){I j=i,t=0;
 v->b=(I4)i; v->d=v->t=0;
 while(t!=CEND){
  j=(j+cw)->go;  // skip through just the control words for this try. structure
  switch(t=(j+cw)->ig.indiv.type){
   case CCATCHD: v->d=(I4)j; break;
   case CCATCHT: v->t=(I4)j; break;
   case CEND:    v->e=(I4)j; break;
  }
 }
}  /* processing on hitting try. */
#endif

// tdv points to the try stack or 0 if none; tdv is NEXT try. slot to fill; i is goto line number
// if there is a try stack, pop its stack frames if they don't include the line number of the goto
// result is new value for tdv
// This is called only if we have a stack
static TD *trypopgoto(J jt, TD* tdv, I dest){
 NOUNROLL while(tdv->ndx&&!BETWEENC(dest,tdv[-1].b,tdv[-1].e)){--tdv; POPTRYSTK(tdv[0].trap)}  // discard stack frame if structure does not include dest
 R tdv;
}

// pop for/select frames that don't include i.  assignvirt is passed on to unstackcv
static CDATA* forpopgoto(J jt, CDATA *cv, I i, I assignvirt){
 while(cv){if(BETWEENC(i,cv->i,cv->go))break; cv=unstackcv(cv,assignvirt);}  // process the for/select stack.  If we branch out of a structure, pop it
 R cv;
}


// Return next line to execute, in case debug changed it
// If debug is running we have to check for a new line to run, after any execution with error or on any line in case the debugger interrupted something
// result is line to continue on
static I debugnewi(I i, DC thisframe, A self){
 if(thisframe){DC siparent;  // if we can take debug input
  // debug mode was on when this execution started.  See if the execution pointer was changed by debug.
  // The newline info is in the parent of this frame, which may be up the stack if there are parse lines
  for(siparent=thisframe;siparent&&siparent->dctype!=DCCALL;siparent=siparent->dclnk);  // search up to find call to here
  if(siparent&&self==siparent->dcf){   // if prev stack frame is a call to here
   if(siparent->dcnewlineno){  // the debugger has asked for a jump
    i=siparent->dcix;  // get the jump-to line
    siparent->dcnewlineno=0;  // reset semaphore
   }
   siparent->dcix=i;  // notify the debugger of the line we are on, in case we stop  
  }
 }
 R i;  // return the current line, possibly modified
}

// Processing of explicit definitions, line by line.  Bivalent, called as y vb vb or x y vb.  If JTXDEFMODIFIER is set,
// it's a modifier, called as u adv adv or u v conj
DF2(jtxdefn){
 V *sv=FAV(self); I sflg=sv->flag;   // pointer to definition, and flags therefrom
 F2PREFIP;PROLOG(0048);
 ARGCHK2(a,w);
 A *line;   // pointer to the words of the definition.  Filled in by LINE
 CW *cw;  // pointer to control-word info for the definition.  Filled in by LINE
 UI nGpysfctdl;  // flags: 1=locked 2=debug(& not locked) 4=tdv->ndx!=0 8 unused 16=thisframe!=0 (i. e. debug allowed) 32=symtable was the original (i. e. !AR(symtab)&ARLSYMINUSE)
             // 64=call is dyadic 128=pm is running    0xff00=(flags byte: mostly debug, but bit 1 from trapping)   0xffff0000=#cws in the definition
 DC callframe=0;  // pointer to the debug frame of the caller to this function (only if it's named), but 0 if we are not debugging/pm'ing
#if NAMETRACK
 // bring out the name, locale, and script into easy-to-display name
 C trackinfo[256];  // will hold name followed by locale
 fauxblock(trackunp); A trackbox; fauxBOXNR(trackbox,trackunp,0,1)  // faux block for line to unparse.  Will be filled in
 UI wx=0, wlen; A trackstg;   // index/len we will write to; unparsed line
 forcetomemory(&trackinfo);
#endif
 TD*tdv=0;  // pointer to base of try. stack.  We could keep the try. stack on the C stack but it's a waste of 5 cachelines
// obsolete I tdi=0;  // index of the next open slot in the try. stack
 CDATA cdata,*cv=0;  // pointer to the current entry in the for./select. stack
 cdata.fchn=0;  // init no blocks allocated

 A locsym;  // local symbol table to use

 DC thisframe=0;   // if we allocate a parser-stack frame, this is it
 A z=mtm;  // last B-block result; will become the result of the execution. z=0 is treated as an error condition inside the loop, so we have to init the result to i. 0 0
 A *hv;  // will hold pointer to the precompiled parts
 {  A u,v;  // pointers to args
 nGpysfctdl=w!=self?64:0;  // set if dyad
 if(likely(((I)jtinplace&JTXDEFMODIFIER)==0)){
  // we are executing a verb.  It may be an operator
   w=nGpysfctdl&64?w:a; a=nGpysfctdl&64?a:0;  // a w self = [x] y verb
  if(unlikely((sflg&VXOP)!=0)){u=sv->fgh[0]; v=sv->fgh[2]; sv=FAV(sv->fgh[1]);}else u=v=0;  // flags don't change
 }else{
  // modifier. it must be (1/2 : n) executed with no x or y.  Set uv then, and undefine x/y
  v=nGpysfctdl&64?w:0; u=a; a=w=0;  // a w self = u [v] mod
 }
  nGpysfctdl|=SGNTO0(-(jt->glock|(sflg&VLOCK)));  // init flags: 1=lock bit, whether from locked script or locked verb
  // Read the info for the parsed definition, including control table and number of lines
  LINE(sv);
  // Create symbol table for this execution.  If the original symbol table is not in use (rank unflagged), use it;
  // otherwise clone a copy of it.  We have to do this before we create the debug frame
  locsym=hv[3];  // fetch pointer to preallocated symbol table
  ASSERT(locsym!=0,EVVALENCE);  // if the valence is not defined, give valence error
  if(likely(!(__atomic_fetch_or(&AR(locsym),ARLSYMINUSE,__ATOMIC_ACQ_REL)&ARLSYMINUSE))){nGpysfctdl|=32;}  // remember if we are using the original symtab
  else{RZ(locsym=clonelocalsyms(locsym));}
  // Symbols may have been allocated.  DO NOT TAKE ERROR RETURNS AFTER THIS POINT: use BASSERT, GAE, BZ
  if(unlikely((jt->uflags.trace|(sflg&(VTRY1|VTRY2))))){  // debug/pm, or try.
   // special processing required
   // if we are in debug/pm mode, the call to this defn should be on the stack (unless debug was entered under program control).  If it is, point to its
   // stack frame, which functions as a flag to indicate that we are debugging.  If the function is locked we ignore debug mode
   if(!(nGpysfctdl&1)&&jt->uflags.trace){
    if(jt->sitop&&jt->sitop->dctype==DCCALL){   // if current stack frame is a call
     callframe=jt->sitop;  // remember the caller's frame
     if(jt->uflags.trace&TRACEDB){  // if debug
      nGpysfctdl|=2;   // set debug flag if debug requested and not locked (no longer used)
      // If we are in debug mode, and the current stack frame has the DCCALL type, pass the debugger
      // information about this execution: the local symbols and the control-word table
      if(self==jt->sitop->dcf){  // if the stack frame is for this exec
       jt->sitop->dcloc=locsym; jt->sitop->dcc=hv[1];  // install info about the exec
       // Use this out-of-the-way place to ensure that the compiler will not try to put for. and try. stuff into registers
// obsolete       forcetomemory(&tdi);
       forcetomemory(&tdv); forcetomemory(&cv); 
      }
     }
    }

    // With debug on, we will save pointers to the sentence being executed in the stack frame we just allocated
   }

   // If the verb contains try., allocate a try-stack area for it.  Remember debug/trapping state coming in so we can restore on exit
   if(sflg&VTRY1+VTRY2){A td; GAT0E(td,LIT,(NTD+1)*sizeof(TD),1,{z=0; goto bodyend;}); tdv=(TD*)AV(td); tdv[0].ndx=0; nGpysfctdl|=PUSHTRYSTK<<8;}
  }
  // End of unusual processing
  SYMPUSHLOCAL(locsym);   // Chain the calling symbol table to this one

  // zombieval should never be set here; if it is, there must have been a pun-in-ASGSAFE that caused us to mark a
  // derived verb as ASGSAFE and it was later overwritten with an unsafe verb.  That would be a major mess; we'll invest
  // in preventing it - still not a full fix, since invalid inplacing may have been done already
  CLEARZOMBIE
  // Assign the special names x y m n u v.  Do this late in initialization because it would be bad to fail after assigning to yx (memory leak would result)
  // For low-rank short verbs, this takes a significant amount of time using IS, because the name doesn't have bucket info and is
  // not an assignment-in-place
  // So, we short-circuit the process by assigning directly to the name.  We take advantage of the fact that we know the
  // order in which the symbols were defined: y then x; and we know that insertions are made at the end; so we know
  // the bucketx for xy are 0 or maybe 1.  We have precalculated the buckets for each table size, so we can install the values
  // directly.
  // Assignments here are special.  At this point we know that the value is coming in from a different namespace, and that it will be
  // freed in that namespace.  Thus, (1) there is no need to realize a virtual block - we can just assign it to x/y, knowing that it is
  // not tying up a backer that could have been freed otherwise (we do have to raise the usecount); (2) if the input
  // has been abandoned, we do not need to raise the usecount here: we can just mark the arg non-inplaceable, usecount 1 and take advantage
  // of assignment in place here - in this case we must flag the name to suppress decrementing the usecount on reassignment or final free.  In
  // both cases we know the block will be freed by the caller.  Since we may revert the value to abandoned inplace to allow it to be free,
  // we must not modify AM.
  // Virtual abandoned blocks are both cases at once.  That's OK.
  // To simplify value stacking, we ensure that any recursible block is recursive.  This is guaranteed by ra() for non-abandoned blocks
  UI4 yxbucks = *(UI4*)LXAV0(locsym);  // get the yx bucket indexes, stored in first hashchain by crelocalsyms
  L *sympv=SYMORIGIN;  // bring into local
  L *ybuckptr = &sympv[LXAV0(locsym)[(US)yxbucks]];  // pointer to sym block for y, known to exist
  if(likely(w!=0)){  // If y given, install it & incr usecount as in assignment.  Include the script index of the modification
   ybuckptr->val=w; ybuckptr->valtype=ATYPETOVALTYPE(AT(w)); ybuckptr->sn=jt->currslistx;  // finish the assignment, with QCGLOBAL semantics
   // If input is abandoned inplace and not the same as x, DO NOT increment usecount, but mark as abandoned and make not-inplace.  Otherwise ra
   // We can handle an abandoned argument only if it is direct or recursive, since only those values can be assigned to a name
   if((a!=w)&SGNTO0(AC(w)&(((AT(w)^AFLAG(w))&RECURSIBLE)-1))&((I)jtinplace>>JTINPLACEWX)){
    AFLAGORLOCAL(w,AFKNOWNNAMED);   // indicate the value is in a name.  We do this to allow virtual extension.
    ybuckptr->flag=LPERMANENT|LWASABANDONED; ACIPNOABAND(w);  // remember, blocks from every may be 0x8..2, and we must preserve the usecount then as if we ra()d it
    ramkrecursv(w);  // make the block recursive
   }else{
    // not abandoned; but it could be VIRTUAL and even UNINCORPABLE!  We know that those blocks have valid usecounts inited to 1, so if we
    // keep the usecount right the block will never be freed except when it goes out of scope in the originator
    ra(w);  // not abandoned: raise the block.  No need for AFKNOWNNAMED since usecount will preclude virtual extension
   }
  }
    // for x (if given), slot is from the beginning of hashchain EXCEPT when that collides with y; then follow y's chain
    // We have verified that hardware CRC32 never results in collision, but the software hashes do (needs to be confirmed on ARM CPU hardware CRC32C)
  if(a!=0){
   L *xbuckptr = &sympv[LXAV0(locsym)[yxbucks>>16]];  // pointer to sym block for x
   if(!C_CRC32C&&xbuckptr==ybuckptr)xbuckptr=xbuckptr->next+sympv;
   xbuckptr->val=a; xbuckptr->valtype=ATYPETOVALTYPE(AT(a)); xbuckptr->sn=jt->currslistx;
   if((a!=w)&SGNTO0(AC(a)&(((AT(a)^AFLAG(a))&RECURSIBLE)-1))&((I)jtinplace>>JTINPLACEAX)){
    AFLAGORLOCAL(a,AFKNOWNNAMED); xbuckptr->flag=LPERMANENT|LWASABANDONED; ACIPNOABAND(a); ramkrecursv(a);
   }else{ra(a);}
  }
  // Do the other assignments, which occur less frequently, with symbis
  if(unlikely(((I)u|(I)v)!=0)){
   if(u){(symbis(mnuvxynam[2],u,locsym)); if(NOUN&AT(u))symbis(mnuvxynam[0],u,locsym); }  // assign u, and m if u is a noun
   if(v){(symbis(mnuvxynam[3],v,locsym)); if(NOUN&AT(v))symbis(mnuvxynam[1],v,locsym); }  // bug errors here must be detected
  }
 }
 FDEPINC(1);
 // remember tnextpushx.  We will tpop after every sentence to free blocks.  Do this AFTER any memory
 // allocation that has to remain throughout this routine.
 // If the user turns on debugging in the middle of a definition, we will raise old when he does
 A *old=jt->tnextpushp;

 // loop over each sentence
 I i=0;  // control-word number of the executing line
 A t=0;  // last T-block result
 I4 bi;   // cw number of last B-block result.  Needed only if it gets a NONNOUN error - can force to memory
 I4 ti;   // cw number of last T-block result.  Needed only if it gets a NONNOUN error
 while(1){
  // **************** top of main dispatch loop ********************
  // Inside the loop we must use BZ and BASSERT or continue for errors; these will break out of the loop and run the ending code
  // i holds the control-word number of the current control word
  // Check for debug and other modes
  if(unlikely(jt->uflags.trace)){  // fast check to see if we have overhead functions to perform
   // here to handle debug jump, perf monitor, or any other unusual cases
   if(!(nGpysfctdl&(1))){  // only if not locked
    if(!(nGpysfctdl&(16))){
     // If we haven't done so already, allocate an area to use for the SI entries for sentences executed here, if needed.  We need a new area only if we are debugging/pm'ing.  Don't do it if locked.
     // We have to have 1 debug frame to hold parse-error information in, but it is allocated earlier if debug is off
     // We check before every sentence in case the user turns on debug in the middle of this definition
     // NOTE: this stack frame could be put on the C stack, but that would reduce the recursion limit because the frame is pretty big
     // If there is no calling stack frame we can't turn on debug mode because we can't suspend
     // If we are executing a recursive call to JDo we can't go into debug because we can't prompt
     DC d; for(d=jt->sitop;d&&DCCALL!=d->dctype;d=d->dclnk);  /* find bottommost call                 */
     if(d&&jt->recurstate<RECSTATEPROMPT){  // if there is a call and thus we can suspend; and not prompting already
      BZ(thisframe=deba(DCPARSE,0L,0L,0L));  // if deba fails it will be before it modifies sitop.  Remember our stack frame
      old=jt->tnextpushp;  // protect the stack frame against free
      nGpysfctdl|=16+2;  // indicate we have a debug frame and are in debug mode
      forcetomemory(&bi); forcetomemory(&ti);  // urge the compiler to leave these values in memory
     }
    }

    if(jt->uflags.trace&TRACEDB){   // if debug is on, or coming on
     nGpysfctdl|=2;  // if this is coming from debug, indicate debug mode
     i=debugnewi(i,thisframe,self);  // get possibly-changed execution line
    }
    if((UI)i>=(UI)(nGpysfctdl>>16))break;  // if defn is exiting, avoid a PM record for an invalid line

    // if performance monitor is on, collect data for it
    if((jt->uflags.trace&TRACEPM)&&C1==((PM0*)CAV1(JT(jt,pma)))->rec){
     // PM is enabled & we have requested line timing.  We have to record the name and locale that is running.
     // If the most recent stack entry before this entity started is a CALL, that is the name to use (it may have been extracted from an anonymous operator).
     // Otherwise, the call is truly anonymous and we use a name of ''
     nGpysfctdl|=128;  // indicate pm running so we flag calls to parser
     pmrecord(callframe&&callframe->dctype==DCCALL?jt->curname:mtv,jt->global?LOCNAME(jt->global):0,i,nGpysfctdl&64?VAL2:VAL1);
    }

    // If the executing verb was reloaded during debug, switch over to the modified definition
    DC siparent;
    if(nGpysfctdl&16){
      if(thisframe->dcredef&&(siparent=thisframe->dclnk)&&siparent->dcn&&DCCALL==siparent->dctype&&self!=siparent->dcf){A *hv;
       self=siparent->dcf; V *sv=FAV(self); LINE(sv); siparent->dcc=hv[1];  // LINE sets pointers for subsequent line lookups
       // Clear all local bucket info in the definition, since it doesn't match the symbol table now
       // This will affect the current definition and all pyx executions of this definition.  We allow it because
       // it's for debug only.  The symbol table itself persists
       DO(AN(hv[0]), if(AT(line[i])&NAME){NAV(line[i])->bucket=0;});
      }
     thisframe->dcredef=0;
    }
   }
  }

  // Don't do the loop-exit test until debug has had the chance to update the execution line.  For example, we might be asked to reexecute the last line of the definition
  if(unlikely((UI)i>=(UI)(nGpysfctdl>>16)))break;  // normal exit at end of definition
  // process the control word according to its type
  I cwgroup;  // combined line#/flags/type for cw being executed
  // **************** switch by line type ********************
  switch(((cwgroup=cw[i].ig.group[0])&31)){  // highest cw is 33, but it aliases to 1 & there is no 32

  // The top cases handle the case of if. T do. B B B... end B B...      without looping back to the switch except for the if.
  case CBBLOCK: case CBBLOCKEND:  // placed first because likely case for unpredicted first line of definition
dobblock:
   // B-block (present on every sentence in the B-block)
   // run the sentence
   tpop(old); parseline(z,lbl1);
   // if there is no error, step to next line.  debug mode has set the value to use if any, or 0 to request a new line
   if(likely(z!=0)){bi=i; i+=((cwgroup>>5)&1)+1;  // go to next sentence, or to the one after that if it's harmless end. 
    if(unlikely((UI)i>=(UI)(nGpysfctdl>>16)))goto nextline;  // end of definition
    if((((cwgroup=cw[i].ig.group[0])^CBBLOCK)&0x1f)|jt->uflags.trace)goto nextline;  // not another B block
    goto dobblock;  // avoid indirect-branch overhead on the likely case
    // BBLOCK is usually followed by another BBLOCK, but another important followon is END followed by BBLOCK.  BBLOCKEND means
    // 'bblock followed by end that falls through', i. e. a bblock whose successor is i+2.  By handling that we process all sequences of if. T do. B end. B... without having to go through the switch;
    // this means the switch will learn to go to the if.
   }else if(unlikely((jt->jerr&(EVEXIT^EVDEBUGEND))==EVEXIT)){i=-1; continue;  // if 2!:55 requested, honor it regardless of debug status; also EVDEBUGEND which silently cuts everything back in that thread
   }else if(unlikely((nGpysfctdl&16)&&(jt->uflags.trace&TRACEDB1))){  // if we get an error return from debug, the user must be branching to a new line.  Do it
    if(jt->jerr==EVCUTSTACK)BZ(0);  // if Cut Stack executed on this line, abort the current definition, leaving the Cut Stack error to cause caller to flush the active sentence
    z=mtm,bi=i,i=debugnewi(i+1,thisframe,self);   // Remember the line w/error; fetch continuation line#. it is OK to have jerr set if we are in debug mode, but z must be a harmless value to avoid error protecting it
   }else if(unlikely(jt->jerr==EVTHROW)){
    // if the error is THROW, and there is a catcht. block, go there, otherwise pass the THROW up the line
// obsolete     if(nGpysfctdl&4&&tdv[tdi-1].t){i=tdv[tdi-1].t+1; RESETERR; z=mtm; POPIFTRYSTK}else BASSERT(0,EVTHROW);  // z might not be protected if we hit error
    if(nGpysfctdl&4){I j; for(j=cw[tdv[-1].b].go;cw[j].ig.indiv.type!=CEND;j=cw[j].go)if(cw[j].ig.indiv.type==CCATCHT){i=j+1; RESETERR; z=mtm; POPIFTRYSTK break;}} BASSERT(z!=0,EVTHROW);  // z might not be protected if we hit error
   // for other error, go to the error location; if that's out of range, keep the error; if not,
   // it must be a try. block, so clear the error (and if the error is ATTN/BREAK, clear the source of the error).
   //  Pop the try. stack, and restore debug\trapping status
// obsolete    // NOTE ERROR: if we are in a for. or select., going to the catch. will leave the stack corrupted,
// obsolete    // with the for./select. structures hanging on.  Solution would be to save the for/select stackpointer in the
// obsolete    // try. stack, so that when we go to the catch. we can cut the for/select stack back to where it
   // was when the try. was encountered
   }else{bi=i; i=cw[i].go; if(i<SMAX){if(BETWEENC(jt->jerr,EVATTN,EVBREAK))CLRATTN RESETERR; z=mtm; cv=forpopgoto(jt,cv,i,1); POPIFTRYSTK}  // Nondebug error.  If caught, we continue: make z valid then.  Pop try. stack always, for. stack if needed
   }
   goto nextline;

  case CIF: case CWHILE: case CELSEIF:
     // in a long run of only if./while. and B blocks, the only switches executed will go to the if. processor, predictably
   i=cw[i].go;  // Go to the next sentence, whatever it is
   if(unlikely((UI)i>=(UI)(nGpysfctdl>>16)))goto nextline;  // no fallthrough if line exits
   if(unlikely((((cwgroup=cw[i].ig.group[0])^CTBLOCK)&0xff)|jt->uflags.trace))goto nextline;  // avoid indirect-branch overhead on the likely case
   // fall through to...
  case CASSERT:
  case CTBLOCK:
tblockcase:
   // execute and parse line as if for B block, except save the result in t
   // If there is a possibility that the previous B result may become the result of this definition,
   // protect it during the frees during the T block.  Otherwise, just free memory
   if(likely(cwgroup&0x200))tpop(old);else z=gc(z,old);   // 2 means previous B can't be the result
   // Check for assert.  Since this is only for T-blocks we tolerate the test (rather than duplicating code)
   if(unlikely((cwgroup&0xff)==CASSERT)){
    if(JT(jt,assert)){parseline(t,lbl2); if(t&&!(NOUN&AT(t)&&all1(eq(num(1),t))))t=pee(line,cw+i,EVASSERT,nGpysfctdl<<(BW-2),callframe); t=t!=0?mtv:t; // if assert., signal post-execution error if result not all 1s.
       // An assert is an entire T-block and must clear t afterward lest t be freed before it is checked by an empty while. .  But we can't set t=0 without looking like an error.  So we use a safe permanent value, mtv.  
    }else{++i; goto nextline;}  // if ignored assert, go to NSI
   }else{parseline(t,lbl3);} // no assert: run the line
   // this is return point from running the line
   if(likely(t!=0)){ti=i,++i;  // if no error, continue on
    if(unlikely((UI)i>=(UI)(nGpysfctdl>>16)))goto nextline;  // exit if end of defn  scaf impossible!
    if(unlikely(((((cwgroup=cw[i].ig.group[0])^CDO)&0xff)|jt->uflags.trace)!=0))goto nextline;  // if next line not do.; T block extended to more than 1 line
    goto docase;  // avoid indirect-branch overhead on the likely case, if. T do.
   }else if(unlikely((jt->jerr&(EVEXIT^EVDEBUGEND))==EVEXIT)){i=-1; continue;  // if 2!:55 requested, honor it regardless of debug status; also EVDEBUGEND which silently cuts everything back in that thread
   }else if(unlikely((nGpysfctdl&16)&&(jt->uflags.trace&TRACEDB1))){  // if we get an error return from debug, the user must be branching to a new line.  Do it
    if(jt->jerr==EVCUTSTACK)BZ(0);  // if Cut Stack executed on this line, abort the current definition, leaving the Cut Stack error to cause caller to flush the active sentence
    z=mtm,bi=i,i=debugnewi(i+1,thisframe,self);   // Remember the line w/error; fetch continuation line#. it is OK to have jerr set if we are in debug mode, but z must be a harmless value to avoid error protecting it
// obsolete    }else if(unlikely(EVTHROW==jt->jerr)){if(nGpysfctdl&4&&tdv[tdi-1].t){i=tdv[tdi-1].t+1; RESETERR; POPIFTRYSTK}else BASSERT(0,EVTHROW);  // if throw., and there is a catch., do so
   }else if(unlikely(EVTHROW==jt->jerr)){if(nGpysfctdl&4){I j; for(j=cw[tdv[-1].b].go;cw[j].ig.indiv.type!=CEND;j=cw[j].go)if(cw[j].ig.indiv.type==CCATCHT){i=j+1; RESETERR; z=mtm; POPIFTRYSTK break;}} BASSERT(z!=0,EVTHROW);  // if throw., and there is a catch., do so
   }else{bi=i; i=cw[i].go; if(i<SMAX){RESETERR; z=mtm; cv=forpopgoto(jt,cv,i,1); POPIFTRYSTK}else z=0;}  // nondebug error: if we take error exit, we might not have protected z, which is not needed anyway; so clear it to prevent invalid use.  Pop try. stack always, for. stack if needed
     // if we are not taking the error exit, we still need to set z to a safe value since we might not have protected it.
   goto nextline;

  case CDO:
docase:
   // do. here is one following if., elseif., or while. .  It always follows a T block, and skips the
   // following B block if the condition is false.
  {A tt=t; tt=t?t:mtv;  // missing t looks like '' which is true
   //  Start by assuming condition is true; set to move to the next line then
   ++i;
   // Quick true cases are: nonexistent t; empty t; direct numeric t with low byte nonzero.  This gets most of the true.  We add in char types and BOX cause it's free (they are always true)
   if(likely(AN(tt)))if((-(AT(tt)&(B01|LIT|INT|INT2|INT4|FL|CMPX|QP|C2T|C4T|BOX))&-((I)CAV(tt)[0]))>=0){I nexti=cw[i-1].go;  // C cond is false if (type direct or BOX) and (value not 0).  J cond is true then.  Musn't fetch CAV[0] if AN==0
    // here the type is indirect or the low byte is 0.  We must compare more
    while(1){  // 2 loops if sparse
     if(likely(AT(tt)&INT+B01)){i=BIV0(tt)?i:nexti; break;} // INT and B01 are most common
     if(AT(tt)&FL){i=DAV(tt)[0]?i:nexti; break;}
     if(AT(tt)&INT2){i=I2AV(tt)[0]?i:nexti; break;}
     if(AT(tt)&INT4){i=I4AV(tt)[0]?i:nexti; break;}
     if(AT(tt)&CMPX){i=DAV(tt)[0]||DAV(tt)[1]?i:nexti; break;}
     if(AT(tt)&(RAT|XNUM)){i=!ISX0(XAV(tt)[0])?i:nexti; break;}
     if(AT(tt)&QP){i=EAV(tt)[0].hi?i:nexti; break;}
     if(!(AT(tt)&NOUN)){CHECKNOUN}  // will take error
     // other nonnumeric types (BOX, char) test true: i is set for that
     if(!ISSPARSE(AT(tt)))break;
     BZ(tt=denseit(tt)); if(AN(tt)==0)break;  // convert sparse to dense - this could make the length go to 0, in which case true
    }
   }
   }
   t=0;  // Indicate no T block, now that we have processed it
   if(unlikely((UI)i>=(UI)(nGpysfctdl>>16)))goto nextline;
   if(unlikely((((cwgroup=cw[i].ig.group[0])^CBBLOCK)&0x1f)|jt->uflags.trace))goto nextline;  // check for end of definition of special types
   goto dobblock;   // normal case, continue with B processing, without switch overhead

  // ************* The rest of the cases are accessed only by indirect branch or fixed fallthrough ********************
  case CTRY:
   {
    // try.  create a try-stack entry, step to next line
    BASSERT(tdv->ndx<NTD,EVLIMIT); tdv[1].ndx=tdv[0].ndx+1;   // verify not too many frames, and init next frame index
// obsolete    tryinit(tdv+tdi,i,cw);
    I j, isd=0; tdv->b=i; for(j=cw[i].go;cw[j].ig.indiv.type!=CEND;j=cw[j].go)isd|=cw[j].ig.indiv.type==CCATCHD; tdv->e=j;  // fill in begin/end lines, remember if catchd. seen
    tdv->trap=PUSHTRYSTK;
    // turn off debugging UNLESS there is a catchd; then keep on only if user set debug mode
    // if debugging is already off, it stays off
// obsolete     if(unlikely(jt->uflags.trace&TRACEDB)){jt->uflags.trace&=~TRACEDB; if((nGpysfctdl&16)&&(UC)tdv[tdi].d)jt->uflags.trace|=TRACEDB1&(JT(jt,dbuser));}
    if(unlikely(jt->uflags.trace&TRACEDB)){jt->uflags.trace&=~TRACEDB; if((nGpysfctdl&16)&&isd)jt->uflags.trace|=TRACEDB1&(JT(jt,dbuser));}  // scaf avoid branches
    // debugging is now off if we are trapping.  In that case, indicate that we are trapping errors, to prevent holding them for debug
    jt->emsgstate|=(~jt->uflags.trace&TRACEDB1)<<EMSGSTATETRAPPINGX;  // turn on trapping if not now debug
    // We allow verbose messages in case the catch. wants to display them.  This is different from u :: v
    ++tdv; nGpysfctdl|=4;  // bump tdv pointer, install index of next entry, set flag
    ++i;  // continue with NSI
   }
   goto nextline;
  case CCATCH: case CCATCHD: case CCATCHT:
   // Falling through to catch*., which ends the trapping if any.  Pop the try-stack, go to after end., reset debug state, restore trapping status.  If there are both catch.
   // and catchd., we could take an error to the catch. and then fall through to catchd., or we could branch from the catch. back to the try.; so we have to make sure that the
   // block on the try. stack is for the structure we are in.  Go through the control words of the current try. looking for a match; if found, pop the try block and goto its end+1;
   // otherwise just go to the next catch* in this structure - we'll hit end presently
   {I ni=cw[i].go; if(likely(nGpysfctdl&4)){I j; for(j=cw[tdv[-1].b].go;cw[j].ig.indiv.type!=CEND;j=cw[j].go)if(j==i){POPIFTRYSTK ni=tdv->e+1; break;}} i=ni;}
   goto nextline;
// obsolete &&i==cw[tdv[tdi-1].b].go){POPIFTRYSTK i=1+tdv[tdi].e;}else i=cw[i].go; break;
  case CTHROW:
   // throw.  Create a throw error
   BASSERT(0,EVTHROW);
  case CFOR: case CSELECT: case CSELECTN:
   // for./select. push the stack.  Use the C stack for the first one, after that allocate as needed, keeping blocks around
   if(cv==0){cv=&cdata; cv->bchn=0;  // for first block, use the canned area.  Indicate no previous blocks; there may be successors that we can reuse
   }else if(cv->fchn){cv=voidAV0(cv->fchn);  // if there is another element already allocated, move to it
   }else{A cd;
    // we have to allocate an element.  cv points to end of current chain
    GAT0E(cd,INT,(sizeof(CDATA)+SZI-1)>>LGSZI,0,{z=0; goto bodyend;}); ACINITZAP(cd) // allocate, rank 0, exiting with error if allocation failure.  Zap the block because it must persist over subsequent tpops; we will delete by hand
    cv->fchn=cd;  // forward-chain chain allocated A block to current CDATA block
    CDATA *newcv=voidAV0(cd);   // get address of CDATA portion of new block
    newcv->bchn=cv; newcv->fchn=0; cv=newcv;  // backward-chain CDATA areas; indicate no forward successor; advance to new block
   } 
   BZ(forinitnames(jt,cv,cwgroup&0xff,line[CWSENTX],i,cw[i].go));  // setup the names and start/end line#s, before we see the iteration value
   ++i;
   goto nextline;
  case CDOF:   // do. after for.
   // do. after for. .  If this is first time, initialize the iterator
   if(unlikely(cv->j<0)){
    BASSERT(t!=0,EVEMPTYT);   // Error if no sentences in T-block
    CHECKNOUN    // if t is not a noun, signal error on the last line executed in the T block
    BZ(forinit(cv,t)); t=0;
   }
   ++cv->j;  // step to first (or next) iteration
   if(likely(cv->indexsym!=0)){
    // for_xyz.  Manage the loop variables
    L *sympv=SYMORIGIN;  // base of symbol array
    A *aval=&sympv[cv->indexsym].val;  // address of iteration-count slot
    A iterct=*aval;  // A block for iteration count
    if(unlikely(AC(iterct)>1))BZ(iterct=swapitervbl(jt,iterct,aval));  // if value is now aliased, swap it out before we change it
    IAV0(iterct)[0]=cv->j;  // Install iteration number into the readonly index
    L *itemsym=&sympv[cv->itemsym];
    if(unlikely(!(cwgroup&0x200)))BZ(z=rat(z));   // if z might be the result, protect it over the free
    if(likely(cv->j<cv->niter)){  // if there are more iterations to do...
    // if xyz has been reassigned, fa the incumbent and reinstate the virtual block, advanced to the next item
     AK(cv->item)+=cv->itemsiz;  // advance to next item
     if(unlikely(itemsym->val!=cv->item)){
      // discard & free incumbent, switch to virtual, raise it
      A val=itemsym->val; fa(val) val=cv->item; ra(val) itemsym->val=val;
      itemsym->valtype=ATYPETOVALTYPE(INT); // also have to set the value type in the symbol, in case it was changed.  Any noun will do
     }
     ++i; continue;   // advance to next line and process it
    }
    // ending the iteration.  set xyz to i.0
    {A val=itemsym->val; fa(val)}  // discard & free incumbent, probably the virtual block.  If the virtual block, this is never the final free, which comes in unstackcv
    itemsym->val=mtv;  // after last iteration, set xyz to mtv, which is permanent
    itemsym->valtype=ATYPETOVALTYPE(INT); // also have to set the value type in the symbol, in case it was changed.  Any noun will do
   }else if(likely(cv->j<cv->niter)){++i; continue;}  // advance to next line and process it
   // if there are no more iterations, fall through...
  case CENDSEL:
   // end. for select., and do. for for. after the last iteration, must pop the stack - just once
   // Must rat() if the current result might be final result, in case it includes the variables we will delete in unstack
   // (scaf this is no longer needed since names are not deleted but the result case is rare)
   if(unlikely(!(cwgroup&0x200)))BZ(z=rat(z)); cv=unstackcv(cv,1);
   i=cw[i].go;    // continue at new location
   goto nextline;
#if 0   // obsolete 
  case CBREAKS:
  case CCONTS:
   // break./continue-in-while. must pop the stack if there is a select. nested in the loop.  These are
   // any number of SELECTN, up to the SELECT 
   if(unlikely(!(cwgroup&0x200)))BZ(z=rat(z));   // protect possible result from pop, if it might be the final result
   NOUNROLL do{I fin=cv->w==CSELECT; cv=unstackcv(cv,1); if(fin)break;}while(1);
    // fall through to...
  case CBREAK:
  case CCONT:  // break./continue. in while., outside of select.
   i=cw[i].go;   // After popping any select. off the stack, continue at new address
   // It must also pop the try. stack, if the destination is outside the try.-end. range
   if(unlikely(nGpysfctdl&4)){tdv=trypopgoto(jt,tdv,i); nGpysfctdl^=tdv->ndx?0:4;}
   goto nextline;
  case CBREAKF:
   // break. in a for. must first pop any active select., and then pop the for.
   // We just pop till we have popped a non-select.
   // Must rat() if the current result might be final result, in case it includes the variables we will delete in unstack
   if(unlikely(!(cwgroup&0x200)))BZ(z=rat(z));   // protect possible result from pop
   NOUNROLL do{I fin=cv->w; cv=unstackcv(cv,1); if((fin^CSELECT)>(CSELECT^CSELECTN))break;}while(1);  // exit on non-SELECT/SELECTN
   i=cw[i].go;     // continue at new location
   // It must also pop the try. stack, if the destination is outside the try.-end. range
   if(unlikely(nGpysfctdl&4)){tdv=trypopgoto(jt,tdv,i); nGpysfctdl^=tdv->ndx?0:4;}
   goto nextline;
  case CRETURN:
   // return.  Protect the result during free, pop the stack back to empty, set i (which will exit)
   i=cw[i].go;   // If there is a try stack, restore to initial debug state.  Probably safe to  do unconditionally
   if(unlikely(nGpysfctdl&4))POPTRYSTK(nGpysfctdl>>8);  // if we had an unfinished try. struct, restore original debug state
   goto nextline;
#endif
  case CCASE:
  case CFCASE:
   // case. and fcase. start a selection.  t has the result of the T block; we check to
   // make sure this is a noun, and save it on the stack in cv->t.  Then clear t
   if(!cv->t){
    // This is the first case.  That means the t block has the select. value.  Save it.
    BASSERT(t!=0,EVEMPTYT);  // error if select. case.
    CHECKNOUN    // if t is not a noun, signal error on the last line executed in the T block
    BZ(ras(t)); cv->t=t; t=0;  // protect t from free while we are comparing with it, save in stack
   }
   i=cw[i].go;  // Go to next sentence, which might be in the default case (if T block is empty)
   if(likely((UI)i<(UI)(nGpysfctdl>>16)))if(likely(!((((cwgroup=cw[i].ig.group[0])^CTBLOCK)&0xff)|jt->uflags.trace)))goto tblockcase;  // avoid indirect-branch overhead on the likely case, which is case. t-block do.
   goto nextline;  // if it's not a t-block, take the indirect branch
  case CDOSEL:   // do. after case. or fcase.
   // do. for case./fcase. evaluates the condition.  t is the result (a T block); if it is nonexistent
   // or not all 0, we advance to the next sentence (in the case); otherwise skip to next test/end
   ++i;  // go to NSI if case tests true
   if(t){    // if t is not a noun, signal error on the last line executed in the T block
    CHECKNOUN
    if(!((AT(t)|AT(cv->t))&BOX)){
     // if neither t nor cv is boxed, just compare for equality.  Boxed empty goes through the other path
     if(!equ(t,cv->t))i=cw[i-1].go;  // should perhaps take the scalar case specially & send it through singleton code
    }else{
     if(all0(eps(boxopen(cv->t),boxopen(t))))i=cw[i-1].go;  // if case tests false, jump around bblock   test is cv +./@:,@:e. boxopen t
    }
    // Clear t to ensure that the next case./fcase. does not reuse this value
    t=0;
   }
   if(likely((UI)i<(UI)(nGpysfctdl>>16)))if(likely(!((((cwgroup=cw[i].ig.group[0])^CBBLOCK)&0x1f)|jt->uflags.trace)))goto dobblock;  // avoid indirect-branch overhead on the likely  case. ... do. bblock
   goto nextline;
    
  case CGOTO: case CBREAKS: case CCONTS: case CBREAK: case CCONT: case CBREAKF: case CRETURN: // goto_label. or any break/continue/return.  Close any structures we branch out of
   i=cw[i].go;  // Go to the next sentence, whatever it is
   // Must rat() if the current result might be final result, in case it includes the variables we will delete in unstack
   if(unlikely(!(cwgroup&0x200)))BZ(z=rat(z));   // protect possible result from pop
   cv=forpopgoto(jt,cv,i,1);   // if the branch takes us outside a control structure, pop the for/select stack
   // It must also pop the try. stack, if the destination is outside the try.-end. range
   if(unlikely(nGpysfctdl&4)){tdv=trypopgoto(jt,tdv,i); nGpysfctdl^=tdv->ndx?0:4;}
   goto checkbreak;   // finish up through BREAK check
  default:   //   CELSE CWHILST CEND
   i=cw[i].go;  // Go to the next sentence, whatever it is
checkbreak:;
   if(unlikely(2<=__atomic_load_n(JT(jt,adbreakr),__ATOMIC_ACQUIRE))) {BASSERT(0,EVBREAK);} 
     // JBREAK0, but we have to finish the loop.  This is double-ATTN, and bypasses the TRY block
  }  // end of giant select
 nextline:;  // here to look at next line.  Line number is i
 }  // end of main loop
bodyend: ;  // we branch to here on fatal error, with z=0
 //  z may be 0 here and may become 0 before we exit
 // We still must not take an error exit in this runout.  We have to hang around to the end to restore symbol tables, pointers, etc.
 // if we did not get an error, the try. and for./select. stacks must be clear

 FDEPDEC(1);
 A prevlocsyms=(A)AM(locsym);  // get symbol table to return to, before we free the old one
 // check for pee
 if(likely(z!=0)){
  // There was a result (normal case)
  // If we are executing a verb (whether or not it started with 3 : or [12] :), make sure the result is a noun.
  // If it isn't, generate a post-execution error for the non-noun
  if(likely(AT(z)&NOUN)){
   // If we are returning a virtual block, we are going to have to realize it.  This is because it might be (indeed, probably is) backed by a local symbol that
   // is going to be summarily freed by the symfreeha() below.  We could modify symfreeha to recognize when we are freeing z, but the case is not common enough
   // to be worth the trouble
   realizeifvirtualE(z,);  // this just leaves z=0 if there is an error realizing
  }else if(AT(self)&ADV+CONJ){  // non-noun result, but OK from adv/conj
   // if we are returning a non-noun, we have to cleanse it of any implicit locatives that refer to the symbol table in use now.
   // It is OK to refer to other symbol tables, since they will be removed if they try to escape at higher lavels and in the meantime can be executed; but
   // there is no way we could have a reference to such an implied locative unless we also had a reference to the current table; so we replace only the
   // first locative in each branch
   z=fix(z,sc(FIXALOCSONLY|FIXALOCSONLYLOWEST));
  }else {pee(line,&cw[bi],EVNONNOUN,nGpysfctdl<<(BW-2),callframe); z=0;}  // signal error, set z to 'no result'
  if(likely(z!=0))z=EPILOGNORET(z);  // protect return value from being freed when the symbol table is.  Must also be before stack cleanup, in case the return value is xyz_index or the like.  If error, leave stack to be freed at restart point
 }else{
  // No result.  Must be an error, or final exit from suspension
  cv=forpopgoto(jt,cv,-1,1);   // clear the for/select stack
  if(unlikely(nGpysfctdl&4)){trypopgoto(jt,tdv,-1);}  // pop all try structs, restore original debug state
// obsolete   if(unlikely(nGpysfctdl&4))POPTRYSTK(nGpysfctdl>>8);  // if we had an unfinished try. struct
 }

 if(unlikely(nGpysfctdl&16)){debz();}   // pair with the deba if we did one

#if 0  // obsolete 
 // clear for/select stack, protecting any result found thereon
 if(likely(cv==0)){  // the for/select stack has been popped back to initial state
  // Normal path.  protect the result block and free everything allocated here, possibly including jt->locsyms if it was cloned (it is on the stack now)
  if(likely(z!=0))z=EPILOGNORET(z);  // protect return value from being freed when the symbol table is.  Must also be before stack cleanup, in case the return value is xyz_index or the like.  If error, leave stack to be freed at restart point
// obsolete  else tpop(_ttop);
   // We have to make sure that we clear all stored values, because they may point to virtual blocks, even unincorpable ones, in callers.
   // tpop will do the trick for cloned symbol tables, and EPILOG calls tpop.  This frees any cloned symbol tables,
   // and below we free the values from an original local table
 }else{
  // Unusual path with an unclosed contruct (e. g. return. from inside for. loop).  We have to free up the for. stack, but the return value might be one of the names
  // to be deleted on the for. stack, so we must protect the result before we pop the stack.  BUT, EPILOG frees all locally-allocated blocks, which might include the symbol
  // table that we need to pop from.  So we protect the symbol table during the cleanup of the result and stack.
  ra(locsym);  // protect local symtable - not contents
  if(likely(z!=0))z=EPILOGNORET(z);  // protect return value from being freed when the symbol table is.  Must also be before stack cleanup, in case the return value is xyz_index or the like.  See sbove for tpop
// obsolete  else tpop(_ttop);
  NOUNROLL while(cv){cv=unstackcv(cv,0);}  // clean up any remnants left on the for/select stack
  fa(locsym);  // unprotect local syms.  This deletes them if they were cloned
 }
#endif
 // locsym may have been freed now, if it was cloned

 // blocks in the for./select. stack are zapped and reused as needed; must be freed en bloc on completion
 A freechn=cdata.fchn; while(freechn){A nextchn=((CDATA*)voidAV0(freechn))->fchn; fa(freechn); freechn=nextchn;}   // free the allocated chain of for./select. blocks, whose contents have been unstacked
 // Pop the stack of private symbol tables
 SYMSETLOCAL(prevlocsyms);

 // If, while debug is off, we hit an error in the master thread that is not going to be intercepted, add a debug frame for the private-namespace chain and leave the freeing for later
 // We don't do this if jt->jerr is set: that's the special result for comming out of debug; or when WSFULL, since there may be no memory
 if(unlikely(z==0))if(jt->jerr && jt->jerr!=EVWSFULL && !(jt->uflags.trace&TRACEDB1) && THREADID(jt)==0 && !(jt->emsgstate&EMSGSTATETRAPPING)){
   deba(DCPM+(bi<<8),locsym,hv[1],self); RETF(0);  // push a debug frame for this error
 }

 // If we are using the original local symbol table, clear it (free all values, free non-permanent names) for next use.  We know it hasn't been freed yet
 // We detect original symbol table by rank flag ARLSYMINUSE - other symbol tables are assigned rank 0.
 // Tables are born with ARNAMEADDED off.  It gets set when a name is added.  Setting back to initial state here, we clear ARNAMEADDED
 if(likely(nGpysfctdl&32)){symfreeha(locsym); __atomic_store_n(&AR(locsym),ARLOCALTABLE,__ATOMIC_RELEASE);}
 // Now that we have deleted all the local symbols, we can see if we were returning one.
 // See if EPILOG pushed a pointer to the block we are returning.  If it did, and the usecount we are returning is 1, set this
 // result as inplaceable and install the address of the tpop stack into AM (as is required for all inplaceable blocks).  If the usecount is inplaceable 1,
 // we don't do this, because it is possible that the AM slot was inherited from higher up the stack.
 // Note that we know we are not returning a virtual block here, so it is OK to write to AM
 // BUT: SPARSE value must NEVER be inplaceable, because the children aren't handled correctly during assignment; and READONLY values must never be inplaceable
 if(likely(z!=0))if(likely(((_ttop!=jt->tnextpushp)==AC(z))&(~AFLAG(z)>>AFROX))){ACRESET(z,(ACINPLACE&~AT(z))|ACUC1) AZAPLOC(z)=_ttop;}  // AC can't be 0.  The block is not in use elsewhere
 RETF(z);
}

// execution of u : v, selecting the version of self to use based on  valence
static DF1(xv1){A z; R dfv1(z,  w,FAV(self)->fgh[0]);}
static DF2(xv2){A z; R dfv2(z,a,w,FAV(self)->fgh[1]);}


// Nilad.  The caller has just executed an entity to produce an operator.  If we are debugging/pm'ing, AND the operator comes from a named entity, we need to extract the
// name so we can debug/time it.  We do this by looking at the debug stack: if we are executing a CALL, we get the name from there.  If we are
// executing PARSE or other, we must be executing a truly anonymous operator, and we return 0
static F1(jtxopcall){R jt->uflags.trace&&jt->sitop&&DCCALL==jt->sitop->dctype?jt->sitop->dca:0;}  // debug or pm, and CALL type.  sitop may be 0 if deb/pm turned on in the middle of a sentence

// This handles adverbs/conjs that refer to x/y.  Install a[/w] into the derived verb as f/h, and copy the flags
// point g in the derived verb to the original self
// If we have to add a name for debugging purposes, do so
// Flag the operator with VOPR, and remove VFIX for it so that the compound can be fixed
DF2(jtxop2){F2PREFIP;A ff,x;
 ARGCHK2(a,w);
 self=AT(w)&(ADV|CONJ)?w:self; w=AT(w)&(ADV|CONJ)?0:w; // we are called as u adv or u v conj
 ff=fdef(0,CCOLON,VERB, jtxdefn,jtxdefn, a,self,w,  (VXOP|VFIX|VJTFLGOK1|VJTFLGOK2)^FAV(self)->flag, RMAX,RMAX,RMAX);  // inherit other flags
 R (x=xopcall(0))?namerefop(x,ff):ff;
}


// w is a box containing enqueued words for the sentences of a definition, jammed together
// 8: nv found 4: mu found 2: x found 1: y found.  u./v. count as u./v.
static I jtxop(J jt,A w){I i,k;
 // init flags to 'not found'
 I fndflag=0;
 A *wv=AAV(w);
   
 I in=AN(w);
 // Loop over each word, starting at beginning where the references are more likely
 for(i=0;i<in;++i) {
  A w=QCWORD(wv[i]);  // w is box containing a queue value.  If it's a name, we inspect it
  if(AT(w)&NAME){
   // Get length/string pointer
   I n=AN(w); C *s=NAV(w)->s;
   if(n){
    // Set flags if this is a special name, or an indirect locative referring to a special name in the last position, or u./v.
    if(n==1||(n>=3&&s[n-3]=='_'&&s[n-2]=='_')){
     if(s[n-1]=='n'||s[n-1]=='v')fndflag|=8;
     if(s[n-1]=='m'||s[n-1]=='u')fndflag|=4;
     else if(s[n-1]=='x')fndflag|=2;
     else if(s[n-1]=='y')fndflag|=1;
    }   // 'one-character name'
   }  // 'name is not empty'
  } // 'is name'
  if(AT(w)&VERB){
    if((FAV(w)->id&-2)==CUDOT)fndflag|=(4<<(FAV(w)->id&1));  // u./v.
  }
  // exit if we have seen enough: mnuv plus x.  No need to wait for y.  If we have seen only y, keep looking for x
  if(fndflag>=8+4+2)R fndflag;
 }  // loop for each word
 R fndflag;  // return what we found
}

// handle m : 0.  deftype=m.
// For 0 : and 13 :, the result is a string.  For other types, it is a list of boxes, one per
// line of the definition.  DDs in the input will have been translated to 9 : string form.
// Stop when we hit ) line, or EOF
static A jtcolon0(J jt, I deftype){A l,z;C*p,*q,*s;A *sb;I m,n;
 n=0;
 I isboxed=BETWEENC(deftype,1,9);  // do we return boxes? yes for 1-8; no for 0 (noun), 9 ({{ }}), and 13 :
 // Allocate the return area, which we extend as needed
 RZ(z=exta(isboxed?INT:LIT,1,1,isboxed?24:300));  sb=AAV1(z); s=CAV1(z);
// obsolete  if(isboxed){RZ(z=exta(BOX,1L,1L,20L)); sb=AAV(z);}
// obsolete  else{RZ(z=exta(LIT,1L,1L,300L)); s=CAV(z);}
 while(1){
  RE(l=jgets("\001"));   // abort if error on input
  if(!l)break;  // exit loop if EOF.  The incomplete definition will be processed
  if(deftype!=0)RZ(l=ddtokens(l,0b1010));  // if non-noun def, handle DDs, for explicit def, return string, allow jgets().  Leave noun contents untouched
  // check for end: ) by itself, possibly with spaces
  m=AN(l); p=q=CAV(l); 
  NOUNROLL while(p<q+m&&' '==*p)++p; if(p<q+m&&')'==*p){NOUNROLL while(p<q+m&&' '==*++p); if(p>=m+q)break;}  // if ) with nothing else but blanks, stop
  // There is a new line.  Append it to the growing result.
  if(isboxed){
   if(unlikely((C2T+C4T)&AT(l)))RZ(l=cvt(LIT,l));  // each line must be LIT
   NOUNROLL while(AN(z)<=n+1){RZ(z=ext(0,z)); sb=AAV1(z);}  // extend the result if necessary
   sb[n]=incorp(l); ++n; // append the line, increment line number
  }else{
   NOUNROLL while(AN(z)<=n+m){RZ(z=ext(0,z)); s=CAV1(z);}  // extend the result if necessary
   MC(s+n,q,m); n+=m; s[n]=CLF; ++n;  // append LF at end of each line, increment 
  }
 }
 AT(z)=isboxed?BOX:LIT;  // If boxed, set that type (suppressed till now to avoid needless clears, since all is nonrecursive)
 AN(z)=AS(z)[0]=n;  // set final count of boxes or characters
 R z;  // return boxes or string
// obsolete  // Return the string.  No need to trim down the list of boxes, as it's transitory
// obsolete  if(isboxed){ R z;}  // boxed: return the valid boxes, one per line
// obsolete  R str(n,s);  // string: null-terminate it (there must be room)
}    /* enter nl terminated lines; ) on a line by itself to exit */

// w is character array or list
// if table, take , w ,. LF    if list take ,&LF^:(LF~:{:) w)
static F1(jtlineit){
 R 1<AR(w)?ravel(stitch(w,scc(CLF))):AN(w)&&CLF==CAV(w)[AN(w)-1]?w:over(w,scc(CLF));
}

// w is a rank-1 string.  We scan it for {{ }} and return the boxed result.  self is invalid
static DF1(jtboxddline){RZ(w=ddtokens(w,0b1110)) R box(w);}


// Convert ASCII w to boxed lines.  w is the argument to m : w and thus might be from 9 : w or might contain
// 9 : w.  All we do here is look for non-embedded LF, i. e. LFs in the current line
// We split into boxed lines on those LFs.  Any 9 : n strings will be processed during execution of this definition
// If the input is a table, we don't look for anything, just box the lines
// userm is the m from the m : string that originated this w.  If userm is not 9, this is a one-line definition and we have to
// scan it for LF and {{ }}
static A jtsent12c(J jt,A w,I userm){C*p,*q,*r,*s,*x;A z;
 ASSERT(!AN(w)||LIT&AT(w),EVDOMAIN);
 ASSERT(2>=AR(w),EVRANK);
 if(AR(w)>1)R rank1ex(w,DUMMYSELF,1,jtboxddline); // table, just box lines individually after checking for {{ }} (which come when we have an AR)

 // otherwise we have a single string.  Could be from 9 : string.  If not, scan it for {{ }}
 if(userm!=9){RZ(w=ddtokens(w,0b1110))}  // scan for {{ }}.  Don't allow calling for another line, and return result as string
 // 9 : string, perhaps.  scan it for LF
 if(!(AN(w)&&DDSEP==CAV(w)[AN(w)-1]))RZ(w=over(w,scc(DDSEP)));  // add LF if missing
 // Lines are separated by DDSEP, and there may be DDSEP embedded in strings.  Convert the whole thing to words, which will
 // leave the embedded DDSEP embedded; then split on the individual DDSEP tokens
 // tokenize the lines.  Each LF is its own token.
 A wil; RZ(wil=wordil(w)); ASSERT(AM(wil)>=0,EVOPENQ) makewritable(wil); I wiln=AS(wil)[0]; I (*wilv)[2]=voidAV(wil); // line index, and number of lines; array of (start,end+1) for each line
 // Compact the word-list to a line-list.  Go through the words looking for LF.  For each line, add an entry to the line-list with all the characters except the LF
 I i=0;  // start of dyad, input pointer through wilv
 I currlinest=0;  // character# at which current line (ending in next LF) starts
 I linex=0;  // index of next output (start,end) to fill
 C *wv=CAV(w);  // the character list backing the words
 while(i<wiln){
  // we have just finished a line.  currlinest is the first character position of the next line
  // a real line.  scan to find next DDSEP.  There must be one.
  NOUNROLL while(wv[wilv[i][0]]!=DDSEP)++i;  // advance to LF
  // add the new line - everything except the LF - to the line list
  wilv[linex][0]=currlinest; wilv[linex][1]=wilv[i][0];  // add the line
  ++linex; currlinest=wilv[i][0]+1;  // advance line pointer, skip over the LF to start of next line
  ++i;  // skip over the LF we processed
 }
 // Now we have compacted all the lines.  Box them
 AS(wil)[0]=linex;  // advance to dyad, set its length
 R jtboxcut0(jt,wil,w,ds(CWORDS));
}    /* literal fret-terminated or matrix sentences into monad/dyad */

// Audit w to make sure it contains all strings; convert to LIT if needed.  Also translate LF to VT
static A jtsent12b(J jt,A w){A t,*wv,y,*yv;I j,*v;
 ASSERT(1>=AR(w),EVRANK);
 wv=AAV(w); 
 GATV(y,BOX,AN(w),AR(w),AS(w)); yv=AAV(y);
 DO(AN(w), RZ(yv[i]=incorp(mkwris(vs(C(wv[i]))))); C *s=CAV(yv[i]); DO(AN(yv[i]), if(unlikely(s[i]==0xa))s[i]=0xb;) )
 // We honor LF as end-of-line even in the middle of a sentence.  This is new, & some old code expects LF to be swallowed by a comment.
 // For compatibility we transfer the LF to VT, which is swallowed by a comment, and convert VT to LF during display
 R y;
}    /* boxed sentences into monad/dyad */

// If *t is a local name, replace it with a pointer to a shared copy, namely the block in the symbol-name already.  Install QC opointers in tv
// Also install bucket info into a local name (but leave the hashes calculated for others)
// in any case, if dobuckets is 0, remove the bucket field (NOT bucketx).  Clearing the bucket field will prevent
// a name's escaping and being used in another context with invalid bucket info.  bucketx must survive in case it holds
// a valid hash of a locative name
// actstv points to the chain headers, actstn is the number of chains
// all the chains have had the non-PERMANENT flag cleared in the pointers
// recur is set if *t is part of a recursive noun.
static A jtcalclocalbuckets(J jt, A *t, LX *actstv, I actstn, I dobuckets, I recur){LX k;
 A tv=QCWORD(*t);  // the actual NAME block
 I tqc=QCTYPE(*t);  // the name type
 L *sympv=SYMORIGIN;  // base of symbol table
 if(!(NAV(tv)->flag&(NMLOC|NMILOC))){  // don't store if we KNOW we won't be looking up in the local symbol table - and bucketx contains a hash/# for NMLOC/NMILOC
  I4 compcount=0;  // number of comparisons before match
  // tv is a simplename.  We will install the bucket/index fields
  // Get the bucket number by reproducing the calculation in the symbol-table routine
  I4 bucket=(I4)SYMHASH(NAV(tv)->hash,actstn);  // bucket number of name hash
  // search through the chain, looking for a match on name.  If we get a match, the bucket index is the one's complement
  // of the number of items compared before the match.  If we get no match, the bucket index is the number
  // of items compared (= the number of items in the chain)
  for(k=actstv[bucket];k;++compcount,k=sympv[k].next){  // k chases the chain of symbols in selected bucket
   if(NAV(tv)->m==NAV(sympv[k].name)->m&&!memcmpne(NAV(tv)->s,NAV(sympv[k].name)->s,NAV(tv)->m)){
    // match found.  this is a local name.  Replace it with the shared copy, flag as shared, set negative bucket#
    // suppress this step if the type is ornamented, i. e. if it is name:: - then we need a flagged copy
    A oldtv=tv;
    if(likely(!(AT(tv)&NAMEABANDON))){  // not name::
     tv=sympv[k].name;  // use shared copy
     if(recur){ras(tv); fana(oldtv);} // if we are installing into a recursive box, increment/decr usecount new/old.  No audit on the fa() since tstack is temporarily invalid
     NAV(tv)->flag|=NMSHARED;  // tag the shared copy as shared
    }
    // Remember the exact location of the symbol.  It will not move as long as this symbol table is alive.  We can
    // use it only when we are in this primary symbol table
    NAV(tv)->symx=k;  // keep index of the allocated symbol
    compcount=~compcount;  // negative bucket indicates found symbol
    break;
   }
  }
  NAV(tv)->bucket=bucket;  // fill in the bucket in the (possibly modified) name
  NAV(tv)->bucketx=compcount;
 }
 if(!dobuckets)NAV(tv)->bucket=0;  // remove bucket if this name not allowed to have them
 R (A)((I)tv|tqc);  // return combined pointer/type
}

EVERYFS(onmself,jtonm,0,0,VFLAGNONE)  // create self to pass into every

// create local-symbol table for a definition
//
// The goal is to save time allocating/deallocating the symbol table for a verb; and to
// save time in name lookups.  We scan the tokens, looking for assignments, and extract
// all names that are the target of local assignment (including fixed multiple assignments).
// We then allocate a symbol table sufficient to hold these values, and a symbol for each
// one.  These symbols are marked LPERMANENT.  When an LPERMANENT symbol is deleted, its value is
// cleared but the symbol remains.
// Then, all simplenames in the definition, whether assigned or not, are converted to bucket/index
// form, using the known size of the symbol table.  If the name was an assigned name, its index
// is given; otherwise the index tells how many names to skip before starting the name search.
//
// This symbol table is created with no values.  When the explicit definition is started, it
// is used; values are filled in as they are defined & removed at the end of execution (but the symbol-table
// entries for them are not removed).  If
// a definition is recursive, it will create a new symbol table, starting it off with the
// permanent entries from this one (with no values).  We create this table with rank 0, and we set
// the rank to 1 while it is in use, to signify that it must be cloned rather than used inplace.

// l is the A block for all the words/queues used in the definition
// c is the table of control-word info used in the definition
// type is the m operand to m : n, indicating part of speech to be produced
// dyad is 1 if this is the dyadic definition
// flags is the flag field for the verb we are creating; indicates whether uvmn are to be defined
//
// We save the symbol chain numbers for y/x in the AM field of the SYMB block
A jtcrelocalsyms(J jt, A l, A c,I type, I dyad, I flags){A actst,*lv,pfst,t,wds;C *s;I j,ln;
 // Allocate a pro-forma symbol table to hash the names into
 RZ(pfst=stcreate(2,40,0L,0L));
 // Do a probe-for-assignment for every name that is locally assigned in this definition.  This will
 // create a symbol-table entry for each such name
 // Start with the argument names.  We always assign y, and x EXCEPT when there is a monadic guaranteed-verb
 RZ(probeisres(mnuvxynam[5],pfst));if(!(!dyad&&(type>=3||(flags&VXOPR)))){RZ(probeisres(mnuvxynam[4],pfst));}
 if(type<3){RZ(probeisres(mnuvxynam[2],pfst)); RZ(probeisres(mnuvxynam[0],pfst));}
 if(type==2){RZ(probeisres(mnuvxynam[3],pfst)); RZ(probeisres(mnuvxynam[1],pfst));}
 // Go through the definition, looking for local assignment.  If the previous token is a simplename, add it
 // to the table.  If it is a literal constant, break it into words, convert each to a name, and process.
 ln=AN(l); lv=AAV(l);  // Get # words, address of first box
 for(j=1;j<ln;++j) {   // start at 1 because we look at previous word
  t=QCWORD(lv[j-1]);  // t is the previous word
  // look for 'names' =./=: .  If found (and the names do not begin with `), replace the string with a special form: a list of boxes where each box contains a name.
  // This form can appear only in compiled definitions
  if(AT(QCWORD(lv[j]))&ASGN&&AT(t)&LIT&&AN(t)&&CAV(t)[0]!=CGRAVE){
   A neww; RZ(neww=words(t));  // find names; if string ill-formed, we might as well catch it now
   if(AN(neww)){  // ignore blank string
    A newt=every(neww,(A)&onmself);  // convert every word to a NAME block
    if(newt){lv[j-1]=QCINSTALLTYPE(t=incorp(newt),QCNOUN); AT(t)|=BOXMULTIASSIGN;}else RESETERR  // if no error, mark the block as MULTIASSIGN type and save it in the compiled definition; also set as t for below.  If error, catch it later
   }
  }

  if((AT(QCWORD(lv[j]))&ASGN+ASGNLOCAL)==(ASGN+ASGNLOCAL)) {  // local assignment
   if(AT(QCWORD(lv[j]))&ASGNTONAME){    // preceded by name?
    // Lookup the name, which will create the symbol-table entry for it
    // name:: causes a little trouble.  The name carries with it the :: flag, but we will eventually replace all refs with the srade ref from
    // this table.  That means we have to remove the :: flag from the stored value, lest every reference appear flagged just because the last one was.
    // Note that we are here looking only before =., so we are specifically checking for name:: =. ... .  This should be an error, and we might catch
    // it when executed; but we are just making sure that it doesn't make the refs invalid.  name:: also sets NAMEXY, and we have to leave that because
    // any valid ref to mnuvxy will need that set; so there is a chance that name:: =. will result in an ordinary reference to the name's having the NAMEXY
    // flag.  That won't hurt anything significant.
    L *nml; RZ(nml=probeisres(t,pfst)); AT(nml->name)&=~NAMEABANDON;   // put name in symbol table, with ABANDON flag cleared
   } else if(AT(t)&LIT) {
    // LIT followed by =.  Probe each word.  Now that we support lists of NAMEs, this is used only for AR assignments
    // First, convert string to words
    s=CAV(t);   // s->1st character; remember if it is `
    if(wds=words(s[0]==CGRAVE?str(AN(t)-1,1+s):t)){I kk;  // convert to words (discarding leading ` if present)
     I wdsn=AN(wds); A *wdsv = AAV(wds), wnm;
     for(kk=0;kk<wdsn;++kk) {
      // Convert word to NAME; if local name, add to symbol table
      if((wnm=onm(wdsv[kk]))) {
       if(!(NAV(wnm)->flag&(NMLOC|NMILOC))){L *nml; RZ(nml=probeisres(wnm,pfst)); AT(nml->name)&=~NAMEABANDON;}  // see above
      } else RESETERR
     }
    } else RESETERR  // if invalid words, ignore - we don't catch it here
   }else if((AT(t)&BOX+BOXMULTIASSIGN)==BOX+BOXMULTIASSIGN){  // not NAME, not LIT; is it NAMEs box?
    // the special form created above.  Add each non-global name to the symbol table
    A *tv=AAV(t); DO(AN(t), if(!(NAV(tv[i])->flag&(NMLOC|NMILOC))){L *nml; RZ(nml=probeisres(tv[i],pfst)); AT(nml->name)&=~NAMEABANDON;})
   }
  } // end 'local assignment'
 }  // for each word in sentence

 // Go through the control-word table, looking for for_xyz.  Add xyz and xyz_index to the local table too.
 I cn=AN(c); CW *cwv=(CW*)AV(c);  // Get # control words, address of first
 for(j=0;j<cn;++j) {   // look at each control word
  if(cwv[j].ig.indiv.type==CFOR){  // for.
   I cwlen = AN(QCWORD(lv[cwv[j].ig.indiv.sentx]));
   if(cwlen>4){  // for_xyz.
    // for_xyz. found.  Lookup xyz and xyz_index
    A xyzname = str(cwlen+1,CAV(QCWORD(lv[cwv[j].ig.indiv.sentx]))+4);  // +1 is -5 for_. +6 _index
    RZ(probeisres(nfs(cwlen-5,CAV(xyzname)),pfst));  // create xyz
    MC(CAV(xyzname)+cwlen-5,"_index",6L);    // append _index to name
    RZ(probeisres(nfs(cwlen+1,CAV(xyzname)),pfst));  // create xyz_index
   }
  }
 }

 // Count the assigned names, and allocate a symbol table of the right size to hold them.  We won't worry too much about collisions, since we will be assigning indexes in the definition.
 // We choose the smallest feasible table to reduce the expense of clearing it at the end of executing the verb
 I pfstn=AN(pfst); LX*pfstv=LXAV0(pfst),pfx; I asgct=0; L *sympv=SYMORIGIN;
 for(j=SYMLINFOSIZE;j<pfstn;++j){  // for each hashchain
  for(pfx=pfstv[j];pfx=SYMNEXT(pfx),pfx;pfx=sympv[pfx].next){++asgct;}  // chase the chain and count.  The chains have MSB flag, which must be removed
 }

 asgct = asgct + ((asgct+6)>>1); // leave just 33% empty space because buckets & symbol addresses do most of the work
 RZ(actst=stcreate(2,asgct,0L,0L));  // Allocate the symbol table we will use
 *(UI4*)LXAV0(actst)=(UI4)((SYMHASH(NAV(mnuvxynam[4])->hash,AN(actst)-SYMLINFOSIZE)<<16)+SYMHASH(NAV(mnuvxynam[5])->hash,AN(actst)-SYMLINFOSIZE));  // get the yx bucket indexes for a table of this size, save in first hashchain

 // Transfer the symbols from the pro-forma table to the result table, hashing using the table size
 // For fast argument assignment, we insist that the arguments be the first symbols added to the table.
 // So we add them by hand - just y and possibly x.  They will be added later too, perhaps
 RZ(probeisres(ca(mnuvxynam[5]),actst));if(!(!dyad&&(type>=3||(flags&VXOPR)))){RZ(probeisres(ca(mnuvxynam[4]),actst));}
 for(j=SYMLINFOSIZE;j<pfstn;++j){  // for each hashchain
  for(pfx=pfstv[j];pfx=SYMNEXT(pfx);pfx=SYMORIGIN[pfx].next){L *newsym;
   A nm=SYMORIGIN[pfx].name;
   // If we are transferring an ACPERMANENT name, we have to clone it, because the name may be local & if it is we may install bucket info or a symbol index
   if(ACISPERM(AC(nm)))RZ(nm=ca(nm));   // only cases are mnuvxy
   RZ(newsym=probeisres(nm,actst));  // create new symbol (or possibly overwrite old argument name)
   newsym->flag = SYMORIGIN[pfx].flag|LPERMANENT;   // Mark as permanent
  }
 }
 I actstn=AN(actst); LX*actstv=LXAV0(actst);  // # hashchains in new symbol table, and pointer to hashchain table

 // Go through all the newly-created chains and clear the non-PERMANENT flag that was set in each root and next pointer.  This flag is set to
 // indicate that the symbol POINTED TO is non-permanent.
 sympv=SYMORIGIN;  // refresh pointer to symbols
 for(j=SYMLINFOSIZE;j<actstn;++j){  // for each hashchain
  actstv[j]=SYMNEXT(actstv[j]); for(pfx=actstv[j];pfx;pfx=sympv[pfx].next){sympv[pfx].next=SYMNEXT(sympv[pfx].next);}  // set PERMANENT for all symbols in the table
 }

 // Go back through the words of the definition, and add bucket/index information for each simplename, and cachability flag
 // If this definition might return a non-noun (i. e. is an adverb/conjunction not operating on xy) there is a problem.
 // In that case, the returned result might contain local names; but these names contain bucket information
 // and are valid only in conjunction with the symbol table for this definition.  To prevent the escape of
 // incorrect bucket information, don't have any (this is easier than trying to remove it from the returned
 // result).  The definition will still benefit from the preallocation of the symbol table.
 for(j=0;j<ln;++j) {
  if(AT(t=QCWORD(lv[j]))&NAME) {
   t=QCWORD(lv[j]=jtcalclocalbuckets(jt,&lv[j],actstv,actstn-SYMLINFOSIZE,type>=3 || flags&VXOPR,0));  // install bucket info into name
   // if the name is not shared, it is not a simple local name.
   // If it is also not indirect, x., or u., it is eligible for caching - if that is enabled
   if(jt->namecaching && !(NAV(t)->flag&(NMILOC|NMDOT|NMIMPLOC|NMSHARED)))NAV(t)->flag|=NMCACHED;
  }else if((AT(t)&BOX+BOXMULTIASSIGN)==BOX+BOXMULTIASSIGN){
   A *tv=AAV(t); DO(AN(t), tv[i]=QCWORD(jtcalclocalbuckets(jt,&tv[i],actstv,actstn-SYMLINFOSIZE,type>=3 || flags&VXOPR,AFLAG(t)&BOX));)  // calculate details about the boxed names.  Remove flags in strings
  }
 }
 R actst;
}

// l is the A block for all the words/queues used in the definition
// c is the table of control-word info used in the definition
// type is the m operand to m : n, indicating part of speech to be produced
// We preparse what we can in the definition
static I pppp(J jt, A l, A c){I j; A fragbuf[20], *fragv=fragbuf+1; I fragl=sizeof(fragbuf)/sizeof(fragbuf[0])-1;  // leave 1 pad word before frag to allow for overfetch in parsea
 // Go through the control-word table, looking at each sentence
 I cn=AN(c); CW *cwv=(CW*)AV(c);  // Get # control words, address of first
 A *lv=AAV(l);  // address of words in sentences
 for(j=0;j<cn;++j) {   // look at each control word
  if(((((I)1<<(BW-CBBLOCK-1))|((I)1<<(BW-CTBLOCK-1)))<<(cwv[j].ig.indiv.type&31))<0){  // BBLOCK or TBLOCK
   // scan the sentence for PPPP.  If found, parse the PPPP and replace the sequence in the sentence; reduce the length
   A *lvv=lv+cwv[j].ig.indiv.sentx;  // pointer to sentence words
   I startx=0, endx=cwv[j].ig.indiv.sentn;  // start and end+1 index of sentence
   // loop till we have found all the parens
   while(1){
    // Look forward for )
    while(startx<endx && !(AT(QCWORD(lvv[startx]))&RPAR))++startx; if(startx==endx)break;  // find ), exit loop if none, finished
    // Scan backward looking for (, to get length, and checking for disqualifying entities
    I rparx=startx; // remember where the ) is
    while(--startx>=0 && !(AT(QCWORD(lvv[startx]))==LPAR)){  // look for matching (   use = because LPAR can be a NAMELESS flag
     if(AT(QCWORD(lvv[startx]))&RPAR+ASGN+NAME)break;  // =. not allowed; ) indicates previous disqualified block; NAME is unknowable
     if(AT(QCWORD(lvv[startx]))&VERB && FAV(QCWORD(lvv[startx]))->flag2&VF2IMPLOC)break;  // u. v. not allowed: they are half-names
     if(AT(QCWORD(lvv[startx]))&CONJ && FAV(QCWORD(lvv[startx]))->id==CIBEAM)break;  // !: not allowed: might produce adverb/conj to do who-knows-what
     if(AT(QCWORD(lvv[startx]))&CONJ && FAV(QCWORD(lvv[startx]))->id==CCOLON && !(AT(QCWORD(lvv[startx-1]))&VERB))break;  // : allowed only in u : v form
    }
    if(startx>=0 && (startx+2<rparx) && (AT(QCWORD(lvv[startx]))==LPAR)){  // ( ... ) but not () or ( word )
     // The ) was matched and the () can be processed.
     // See if the () block was a (( )) block.  If it is, we will execute it even if it contains verbs
     I doublep = (rparx+1<endx) && (AT(QCWORD(lvv[rparx+1]))==RPAR) && (startx>0) && (AT(QCWORD(lvv[startx-1]))==LPAR);  // is (( ))?
     if(doublep){--startx, ++rparx;  // (( )), expand the look to include outer ()
     }else{
      // Not (( )).  We have to make sure no verbs in the fragment will be executed.  They might have side effects, such as increased space usage.
      // copy the fragment between () to a temp buffer, replacing any verb with [:
      if(fragl<rparx-startx-1){A fb; GATV0(fb,INT,rparx-startx-1,0) fragv=AAV0(fb); fragl=AN(fb);}  // if the fragment buffer isn't big enough, allocate a new one
      DO(rparx-startx-1, fragv[i]=AT(QCWORD(lvv[startx+i+1]))&VERB?QCINSTALLTYPE(ds(CCAP),QCVERB):lvv[startx+i+1];)  // copy the fragment, not including (), with verbs replaced
      // parse the temp for error, which will usually be an attempt to execute a verb
      parsea(fragv,rparx-startx-1);
     }
     if(likely(jt->jerr==0)){
      // no error: parse the actual () block
      // mark the block as PPPP if it will need extra parens: (( )) or noun or non-noun & not invisible modifier, which always has ( ) added
      A pfrag; RZ(pfrag=parsea(&lvv[startx+1],rparx-startx-1)); makewritable(pfrag); INCORP(pfrag); AFLAGORLOCAL(pfrag,(doublep | !!(AT(pfrag)&NOUN) | (AT(pfrag)&FUNC && !BETWEENC(FAV(pfrag)->id,CHOOK,CADVF)))<<AFDPARENX);  // indicate that the value came from ( non-hook )  or (( ))
      // Replace the () block with its parse, close up the sentence, zero the ending area
      lvv[startx]=QCINSTALLTYPE(pfrag,ATYPETOVALTYPE(AT(pfrag))); DO(endx-(rparx+1), lvv[startx+1+i]=lvv[rparx+1+i];) DP(rparx-startx, lvv[endx+i]=0;) 
      // Adjust the end pointer and the ) position
      cwv[j].ig.indiv.sentn=endx-=rparx-startx; rparx=startx;  // back up to account for discarded tokens; resume as if the parse result was at ) position
     }else{RESETERR} // skipping because of error; clear error indic
    }
    // Whether we skipped or not, rpar now has the adjusted position of the ) and endx is correct relative to it.  Advance to next ) search
    startx=rparx+1;  // continue look after )
   }
  }
 }
 R 1;
}

// a is a local symbol table, possibly in use
// result is a copy of it, ready to use.  All PERMANENT symbols are copied over and given empty values, without inspecting any non-PERMANENT ones
// The rank-flag of the table is 'cloned'
A jtclonelocalsyms(J jt, A a){A z;I j;I an=AN(a); LX *av=LXAV0(a),*zv;
 RZ(z=stcreate(2,AN(a),0L,0L)); zv=LXAV0(z); AR(z)|=ARLCLONED;  // allocate the clone; zv->clone hashchains; set flag to indicate cloned
 // Copy the first hashchain, which has the x/v hashes
 zv[0]=av[0]; // Copy as LX; really it's a UI4
 // We don't need to save all SYMLINFOSIZE slots for local symbol tables
 // Go through each hashchain of the model, after the first one.  We know the non-PERMANENT flags are off
 for(j=SYMLINFOSIZE;j<an;++j) {LX *zhbase=&zv[j]; LX ahx=av[j]; LX ztx=0; // hbase->chain base, hx=index of current element, ztx is element to insert after
  SYMRESERVE(1)   // this relocates symbols
  while(SYMNEXTISPERM(ahx)) {L *l;  // for each permanent entry...
   l=symnew(zhbase,SYMNEXT(ztx));   // make sure symbol is available; append new symbol after tail (or head, if tail is empty), as PERMANENT.
   *zhbase=SYMNEXT(*zhbase);  // zhbase points to the pointer to the entry we just added.  First time, that's the chain base
   A nm=(SYMORIGIN)[ahx].name;
   l->name=nm; ra(l->name);  // point symbol table to the name block, and increment its use count accordingly
   l->flag=(SYMORIGIN)[ahx].flag&(LINFO|LPERMANENT);  // Preserve only flags that persist
   ztx = ztx?(SYMORIGIN)[ztx].next : *zhbase;  // ztx=index to value we just added.  We avoid address calculation because of the divide.  If we added
      // at head, the added block is the new head; otherwise it's pointed to by previous tail
   SYMRESERVEPREFSUFF(1,I lsymno=l-SYMORIGIN;, l=&(SYMORIGIN)[lsymno];)   // relocate l if the symbols are relocated
   zhbase=&l->next;  // after the first time, zhbase is the chain field of the tail
   ahx = (SYMORIGIN)[ahx].next;  // advance to next symbol
  }
 }
 R z;
}

F2(jtcolon){F2PREFIP;A d,h,*hv,m;C*s;I flag=VFLAGNONE,n,p;
 ARGCHK2(a,w);PROLOG(778);
 A z; fdefallo(z)
 if(VERB&AT(a)){  // v : v case
  ASSERT(AT(w)&VERB,EVDOMAIN);   // v : noun is an error
  // If nested v : v, prune the tree
  if(CCOLON==FAV(a)->id&&FAV(a)->fgh[0]&&VERB&AT(FAV(a)->fgh[0])&&VERB&AT(FAV(a)->fgh[1]))a=FAV(a)->fgh[0];  // look for v : v; don't fail if fgh[0]==0 (namerefop).  Must test fgh[0] first
  if(CCOLON==FAV(w)->id&&FAV(w)->fgh[0]&&VERB&AT(FAV(w)->fgh[0])&&VERB&AT(FAV(w)->fgh[1]))w=FAV(w)->fgh[1];
  fdeffill(z,0,CCOLON,VERB,xv1,xv2,a,w,0L,((FAV(a)->flag&FAV(w)->flag)&VASGSAFE),mr(a),lr(w),rr(w)) R z; // derived verb is ASGSAFE if both parents are 
 }
 ASSERT(AT(w)&NOUN,EVDOMAIN);   // noun : verb is an error
 RE(n=i0(a));  // m : n; set n=value of a argument
 I col0;  // set if it was m : 0
 if(col0=equ(w,num(0))){RZ(w=colon0(n)); }   // if m : 0, read up to the ) .  If 0 : n, return the string unedited
 if(n==0){ra0(w); RCA(w);}  // noun - it's a string, return it.  Give it recursive usecount
 if((C2T+C4T)&AT(w))RZ(w=cvt(LIT,w));
 I splitloc=-1;   // will hold line number of : line
 if(10<n){ASSERT(AT(w)&LIT,EVDOMAIN) s=CAV(w); p=AN(w); if(p&&CLF==s[p-1])RZ(w=str(p-1,s));}  // if tacit form, discard trailing LF
 else{  // not tacit translator - preparse the body
  // we want to get all forms to a common one: boxed string(s) rank<2.  If we went through m : 0, we are in that form
  // already.
  if(!col0)if(BOX&AT(w)){RZ(w=sent12b(C(w)))  // list of boxes - convert to character, no analysis
  }else{  // character
   RZ(w=jtsent12c(jt,w,n)) // if this is NOT 9 : w, cause a one-line definition to be scanned for {{ }}.  9 : strings have already been scanned
  }
  // If there is a control line )x at the top of the definition, parse it now and discard it from m
  if(likely(AN(w)!=0))if(unlikely(AN(C(AAV(w)[0]))&&CAV(C(AAV(w)[0]))[0]==')')){
   // there is a control line.  parse it.  Cut to words
   A cwds=wordil(C(AAV(w)[0])); RZ(cwds); ASSERT(AM(cwds)==2,EVDOMAIN);  // must be exactly 2 words: ) and type
   ASSERT(((IAV(cwds)[1]-IAV(cwds)[0])|(IAV(cwds)[3]-IAV(cwds)[2]))==1,EVDOMAIN);  // the ) and the next char must be 1-letter words  
   C ctltype=CAV(C(AAV(w)[0]))[IAV(cwds)[2]];  // look at the second char, which must be one of acmdv*  (n is handled in ddtokens)
   I newn=-1; newn=ctltype=='a'?1:newn; newn=ctltype=='c'?2:newn; newn=ctltype=='m'?3:newn; newn=ctltype=='d'?4:newn; newn=ctltype=='v'?3:newn; newn=ctltype=='*'?9:newn;  // choose type based on char
   ASSERT(newn>=0,EVDOMAIN);  // error if invalid char
   n=newn;  // accept the type the user specified
   // discard the control line
   RZ(w=beheadW(w));
   // Noun DD
  }
  // find the location of the ':' divider line, if any.  But don't recognize : on the last line, since it could
  // conceivably be the return value from a modifier
  A *wv=AAV(w); DO(AN(w)-1, I st=0; A wvc=C(*wv); 
    DO(AN(wvc), I c=CAV(wvc)[i]; if(c!=':'&&c!=' '){st=0; break;} if(c!=' ')if(st==1){s=0; break;}else st=1;)
    if(st==1){splitloc=wv-AAV(w); break;} ++wv;)
  // split the definition into monad and dyad.
  I mn=splitloc<0?AN(w):splitloc; I nn=splitloc<0?0:AN(w)-splitloc-1;
  RZ(m=take(sc(mn),w)); RZ(d=take(sc(-nn),w));
  INCORP(m); INCORP(d);  // we are incorporating them into hv[]
  if(4==n){if((-AN(m)&(AN(d)-1))<0)d=m; m=mtv;}  //  for 4 :, make the single def given the dyadic one
  GAT0(h,BOX,2*HN,1); hv=AAV(h);
  if(n){B b;  // if not noun, audit the valences as valid sentences and convert to a queue to send into parse()
   RE(b=preparse(m,hv,hv+1)); if(b)flag|=VTRY1; hv[2]=JT(jt,retcomm)?m:mtv;
   RE(b=preparse(d,hv+HN,hv+HN+1)); if(b)flag|=VTRY2; hv[2+HN]=JT(jt,retcomm)?d:mtv;
  }
 }
 // The h argument is logically h[2][HN] where the boxes hold (parsed words, in a row);(info for each control word);(original commented text (optional));(local symbol table)
 // Non-noun results cannot become inputs to verbs, so we do not force them to be recursive
 if((1LL<<n)&0x206){  // types 1, 2, 9
  I fndflag=xop(hv[0])|xop(hv[0+HN]);   // 8=mu 4=nv 2=x 1=y, combined for both valences
  // for 9 : n, figure out best type after looking at n
  if(n==9){
   I defflg=(fndflag&((splitloc>>(BW-1))|-4))|1; n=CTLZI(defflg); n=(0x2143>>(n<<2))&0xf; // replace 9 by value depending on what was seen; if : seen, ignore x
   if(n==4){hv[HN]=hv[0]; hv[0]=mtv; hv[HN+1]=hv[1]; hv[1]=mtv; hv[HN+2]=hv[2]; hv[2]=mtv; flag=(flag&~VTRY2)+VTRY1; }  // if we created a dyadic verb, shift the monad over to the dyad and clear the monad, incl try flag
  } 
  if(n<=2){  // adv or conj after autodetection
    flag|=REPSGN(-(fndflag&3))&VXOPR;   // if this def refers to xy, set VXOPR
   // if there is only one valence defined, that will be the monad.  Swap it over to the dyad in two cases: (1) it is a non-operator conjunction: the operands will be the two verbs;
   // (2) it is an operator with a reference to x
   if(((-AN(m))&(AN(d)-1)&((SGNIFNOT(flag,VXOPRX)&(1-n))|(SGNIF(flag,VXOPRX)&SGNIF(fndflag,1))))<0){A*u=hv,*v=hv+HN,x; DQ(HN, x=*u; *u++=*v; *v++=x;);}  // if not, it executes on uv only; if conjunction, make the default the 'dyad' by swapping monad/dyad
   // for adv/conj, flag has operator status from here on
  }
 }
 flag|=VFIX;  // ensures that f. will not look inside n : n
 // Create a symbol table for the locals that are assigned in this definition.  It would be better to wait until the
 // definition is executed, so that we wouldn't take up the space for library verbs; but since we don't explicitly free
 // the components of the explicit def, we'd better do it now, so that the usecounts are all identical
 if(4>=n) {
  // explicit definitions.  Create local symbol table and pppp
  I hnofst=0; do{  // for each valence
   // Don't bother to create a symbol table for an empty definition, since it is a domain error
   if(AN(hv[hnofst+1])){
    RZ(hv[hnofst+3] = incorp(crelocalsyms(hv[hnofst+0], hv[hnofst+1],n,!!hnofst,flag)));  // words,cws,type,dyad,flag
    pppp(jt, hv[hnofst+0], hv[hnofst+1]);  // words,cws,type,dyad,flag
   }
  }while((hnofst+=HN)<=HN);
 }
 switch(n){
 case 3:  fdeffill(z,0,CCOLON, VERB, jtxdefn,jtxdefn,       num(n),0L,h, flag|VJTFLGOK1|VJTFLGOK2, RMAX,RMAX,RMAX); break;
 case 1:  fdeffill(z,0,CCOLON, ADV,  flag&VXOPR?jtxop2:jtxdefn,jtvalenceerr,    num(n),0L,h, flag, RMAX,RMAX,RMAX); break;
 case 2:  fdeffill(z,0,CCOLON, CONJ, jtvalenceerr,flag&VXOPR?jtxop2:jtxdefn, num(n),0L,h, flag, RMAX,RMAX,RMAX); break;
 case 4:  fdeffill(z,0,CCOLON, VERB, jtxdefn,jtxdefn,       num(n),0L,h, flag|VJTFLGOK1|VJTFLGOK2, RMAX,RMAX,RMAX); break;
 case 13: z=vtrans(w); break;
 default: ASSERT(0,EVDOMAIN);
 }
 // EPILOG is called for because of the allocations we made, but it is essential to make sure the pfsts created during crelocalsyms get deleted.  They have symbols with no value
 // that will cause trouble if 18!:_2 is executed before they are expunged.  As a nice side effect, all explicit definitions are recursive.
 EPILOG(z);
}

// input reader for direct definition
// This is a drop-in for tokens().  It takes a string and env, and returns tokens created by enqueue().  Can optionally return string
//
// Any DDs found are collected and converted into ( m : string ).  This is done recursively.  If an unterminated
// DD is found, we call jgets() to get the next line, taking as many lines as needed to leave with a valid line.
// The string for a DD will contain a trailing LF plus one LF for each LF found inside the DD.
//
// Bits 0-1 of env are as for enqueue()
//
// Bit 2 of env suppresses the call to jgets().  It will be set if it is known that there is no way to get more
// input, for example if the string comes from (". y), from an event, or from m : 'string'.  If an unterminated DD is found when bit 2 is set,
// the call fails with control error
//
// Bit 3 of env is set if the caller wants the returned value as a string rather than as enqueued words
//
// If the call to jgets() returns EOF, indicating end-of-script, that is also a control error
A jtddtokens(J jt,A w,I env){
// TODO: Use LF for DDSEP, support {{), make nouns work
 PROLOG(000);F1PREFIP;
 ARGCHK1(w);
 // find word boundaries, remember if last word is NB
 A wil; RZ(wil=wordil(w));  // get index to words
 C *wv=CAV(w); I nw=AS(wil)[0]; I (*wilv)[2]=voidAV(wil);  // cv=pointer to chars, nw=#words including final NB   wilv->[][2] array of indexes into wv word start/end
 // scan for start of DD/end of DD.
 I firstddbgnx;  // index of first/last start of DD
 I ddschbgnx=0; // place where we started looking for DD
 for(firstddbgnx=ddschbgnx;firstddbgnx<nw;++firstddbgnx){US ch2=*(US*)(wv+wilv[firstddbgnx][0]); ASSERT(!(ch2==DDEND&&(wilv[firstddbgnx][1]-wilv[firstddbgnx][0]==2)),EVEMPTYDD) if(ch2==DDBGN&&(wilv[firstddbgnx][1]-wilv[firstddbgnx][0]==2))break; }
   // search for {{; error if first thing found is }}; firstddbgnx is index of {{
 if(firstddbgnx>=nw){ASSERT(AM(wil)>=0,EVOPENQ) R env&8?w:enqueue(wil,w,env&3);}    //   If no DD chars found, and caller wants a string, return w fast.  There may be a latent quote error to attend to
 // loop till all DDs found
 while(firstddbgnx<nw){
  // We know that firstddbgnx is DDBGN
  // If there are words before the DDBGN, back the starting word to include leading whitespace.  This loses space between two adjacent DDs, bfd
  if(ddschbgnx<firstddbgnx)wilv[ddschbgnx][0]=ddschbgnx>0?wilv[ddschbgnx-1][1]:0;  // back up to start of line or end of word
  I *fillv=&wilv[ddschbgnx][1]; DQ(2*(firstddbgnx-ddschbgnx)-1, *fillv++=wilv[firstddbgnx][0];)  // set each end-of-word pointer to end at the start of [ddbgnx], and all start pointers except the first.  This preserves spacing from schbgn to firstddbgn
  I ddendx=-1, ddbgnx=firstddbgnx;  // end/start of DD, indexes into wilv.  This will be the pair we process.  We advance ddbgnx and stop when we hit ddendx
  I scanstart=firstddbgnx;  // start looking for DDEND/nounDD at the first known DDBGN.  But if there turns out to be no DDEND, advance start ptr to avoid rescanning
  while(1){I i;  // loop till we find a complete DD
   // w/wilv has the accumulated words in the lines of the DD.  scanstart indexes the first word of the most-recent line
   for(i=scanstart;i<nw;++i){
    US ch2=*(US*)(wv+wilv[i][0]);  // digraph for next word
    if(ch2==DDEND&&(wilv[i][1]-wilv[i][0]==2)){ddendx=i; break;}  // if end, break, we can process
    if(ch2==DDBGN&&(wilv[i][1]-wilv[i][0]==2)){
     //  Nested/noun DD found, OR looking at the first {{.  We have to go back and preserve the spacing for everything that precedes it
     ddbgnx=i;  // set new start pointer, when we find an end
     // Move all the words before the DD into one long megaword - if there are any.  We mustn't disturb the DD itself
     // Close up after the megaword with empties
     I *fillv=&wilv[firstddbgnx][1]; DQ(2*(ddbgnx-firstddbgnx)-1, *fillv++=wilv[ddbgnx][0];)  // preserve spacing from firstddbgnx to new {{
     if(AN(w)>=wilv[i][1]+2 && wv[wilv[i][0]+2]==')' && wv[wilv[i][0]+3]=='n'){  // is noun DD?
      // if the nested DD is a noun, break to process it immediately
      ddendx=0;  // use the impossible starting }} to signify noun DD
      break;  // go process the noun DD
     }
    }
   }  // find DD pair
   if(ddendx>=0)break;  // we found an end, and we started at a begin, so we have a pair, or a noun DD if endx=0.  Go process it

   // Here the current line ended with no end DD or noun DD.  We have to continue onto the next line
   ASSERT(AM(wil)>=0,EVOPENQ);  // if the line didn't contain noun DD, we have to give error if open quote
   ASSERT(!(env&4),EVEMPTYDD);   // Abort if we are not allowed to continue (as for an event or ". y)
   scanstart=AM(wil);  // Get # words, not including NB.  We have looked at em all, so start next look after all of them
   A neww=jgets("\001");  // fetch next line, in raw mode
   RE(0); ASSERT(neww!=0,EVEMPTYDD); // fail if jgets failed, or if it returned EOF - problem either way
   // join the new line onto the end of the old one (after discarding trailing NB in the old).  Must add an LF character and a word for it
   w=jtapip(jtinplace,w,scc(DDSEP));   // append a separator, which is all that remains of the original line
   jtinplace=(J)((I)jtinplace|JTINPLACEW);  // after the first one, we can certainly inplace on top of w
   I oldchn=AN(w);  // len after adding LF, before adding new line
   RZ(w=jtapip(jtinplace,w,neww));   // join the new character list onto the old, inplace if possible
   // Remove the trailing comment if any from wil, and add a word for the added LF
   makewritable(wil);  // We will modify the block rather than curtailing it
   AS(wil)[0]=scanstart; AN(wil)=2*scanstart;  // set AS and AN to discard comment
   A lfwd; fauxblockINT(lffaux,2,2); fauxINT(lfwd,lffaux,2,2)  AS(lfwd)[0]=2; AS(lfwd)[1]=1; IAV(lfwd)[0]=oldchn-1; IAV(lfwd)[1]=oldchn; 
   RZ(wil=jtapip(jtinplace,wil,lfwd));  // add a new word for added LF.  # words in wil is now scanstart+1
   ++scanstart;  // update count of words already examined so we start after the added LF word
   A newwil; RZ(newwil=wordil(neww));  // get index to new words
   I savam=AM(newwil);  // save AM for insertion in final new string block
   makewritable(newwil);  // We will modify the block rather than adding to it
   I *nwv=IAV(newwil); DO(AN(newwil), *nwv++ += oldchn;)  // relocate all the new words so that they point to chars after the added LF
   RZ(wil=jtapip(jtinplace,wil,newwil));   // add new words after LF, giving old LF new
   if(likely(savam>=0)){AM(wil)=savam+scanstart;}else{AM(wil)=savam;}  // set total # words in combined list, not including final comment; remember if error scanning line
   wv=CAV(w); nw=AS(wil)[0]; wilv=voidAV(wil);  // refresh pointer to word indexes, and length
  }

  if(ddendx==0){
   // ******* NOUN DD **********
   // we found a noun DD at ddbgnx: process it, replacing it with its string.  Look for delimiters on the first line, or at the start of a later line
   I nounstart=wilv[ddbgnx][0]+4, wn=AN(w);  // the start of the DD string, the length of the string
   I enddelimx=wn;  // will be character index of the ending }}, not found if >=wn-1, first line empty if =wn
   // characters after nounstart cannot be examined as words, since they might have mismatched quotes.  Scan them for }}
   if(nounstart<wn){  // {{)n at EOL does not add LF
    for(enddelimx=nounstart;enddelimx<wn-1;++enddelimx)if(wv[enddelimx]==(DDEND&0xff) && wv[enddelimx+1]==(DDEND>>8))break;
   }
   if(enddelimx>=wn-1){
    ASSERT(!(env&4),EVEMPTYDD);   // Abort if we are not allowed to consume a new line (as for an event or ". y)
    // if a delimiter was not found on the first line, consume lines until we hit a delimiter
    while(1){
     // append the LF for the previous line (unless an empty first line), then the new line itself, to w
     if(enddelimx<wn){
      w=jtapip(jtinplace,w,scc(DDSEP));   // append a separator
      jtinplace=(J)((I)jtinplace|JTINPLACEW);  // after the first one, we can certainly inplace on top of w
     }
     enddelimx=0;   // after the first line, we always install the LF
     A neww=jgets("\001");  // fetch next line, in raw mode
     RE(0); ASSERT(neww!=0,EVEMPTYDD); // fail if jgets failed, or if it returned EOF - problem either way
     // join the new line onto the end of the old one
     I oldchn=AN(w);  // len after adding LF, before adding new line
     RZ(w=jtapip(jtinplace,w,neww));   // join the new character list onto the old, inplace if possible
     jtinplace=(J)((I)jtinplace|JTINPLACEW);  // after the first one, we can certainly inplace on top of w
     wv=CAV(w);   // refresh data pointer.  Number of words has not changed, nor have indexes
     // see if the new line starts with the delimiter - if so, we're done looking
     if(AN(w)>=oldchn+2 && wv[oldchn]==(DDEND&0xff) && wv[oldchn+1]==(DDEND>>8)){enddelimx=oldchn; break;}
    }
   }
   // We have found the end delimiter, which starts at enddelimx.  We reconstitute the input line: we convert the noun DD to a quoted string
   // and append the unprocessed part of the last line.  For safety, we put spaces around the string.  We rescan the combined line without
   // trying to save the scan pointer, since the case is rare
   A remnant; RZ(remnant=str(AN(w)-enddelimx-2,CAV(w)+enddelimx+2));  // get a string for the preserved tail of w
   AS(wil)[0]=ddbgnx; RZ(w=unwordil(wil,w,0));  // take everything up to the {{)n - it may have been put out of order
   A spacea; RZ(spacea=scc(' ')); RZ(w=apip(w,spacea));  // put space before quoted string
   RZ(w=apip(w,strq(enddelimx-nounstart,wv+nounstart)));  // append quoted string
   RZ(w=apip(w,spacea));  // put space after quoted string
   RZ(w=apip(w,remnant));  // install unprocessed chars of original line
   // line is ready.  Process it from the beginning
   RZ(wil=wordil(w));  // get index to words
   wv=CAV(w); nw=AS(wil)[0]; wilv=voidAV(wil);  // cv=pointer to chars, nw=#words including final NB   wilv->[][2] array of indexes into wv word start/end
   ddschbgnx=0;  // start scan back at the beginning
  }else{
   // ********* NORMAL DD *******
   // We have found an innermost non-noun DD, running from ddbgnx to ddendx.  Convert it to ( 9 : 'string' ) form
   // convert all the chars of the DD to a quoted string block
   // if the first line is empty (meaning the first word is a linefeed by itself), skip it
   I firstcharx=wilv[ddbgnx+1][0]; if(wv[firstcharx]=='\n')++firstcharx;  // just one skipped line
   A ddqu; RZ(ddqu=strq(wilv[ddendx][0]-firstcharx,wv+firstcharx));
   // append the string for the start/end of DD
   I bodystart=AN(w), bodylen=AN(ddqu), trailstart=wilv[ddendx][1];  // start/len of body in w, and start of after-DD text
   RZ(ddqu=jtapip(jtinplace,ddqu,str(8,") ( 9 : ")));
   // append the new stuff to w
   RZ(w=jtapip(jtinplace,w,ddqu));
   wv=CAV(w);   // refresh data pointer.  Number of words has not changed, nor have indexes
   // Replace ddbgnx and ddendx with the start/end strings.  Fill in the middle, if any, with everything in between
   wilv[ddbgnx][0]=AN(w)-6; wilv[ddbgnx][1]=AN(w);  //  ( 9 :  
   wilv[ddbgnx+1][0]=bodystart; I *fillv=&wilv[ddbgnx+1][1]; DQ(2*(ddendx-ddbgnx)-1, *fillv++=bodystart+bodylen+2;)  // everything in between, and trailing )SP
   // continue the search.
   // The characters below ddendx are out of order and there will be trouble if we try to preserve
   // the user's spacing by grouping them.  We must run the ending lines together, to save their spacing, and then
   // refresh w and wil to get the characters in order.  We really need this only when there is a nested or noun DD, but that's hard to detect
   if(++ddendx<nw){wilv[ddendx][0]=trailstart; wilv[ddendx][1]=bodystart; AN(wil)=2*(AS(wil)[0]=ddendx+1);}  // make one string of DDEND to end of string
   RZ(w=unwordil(wil,w,0)); RZ(wil=wordil(w));  // run chars in order; get index to words
   wv=CAV(w); nw=AS(wil)[0]; wilv=voidAV(wil);  // cv=pointer to chars, nw=#words including final NB   wilv->[][2] array of indexes into wv word start/end
   ddschbgnx=0; wilv[0][0]=0;  // start scan back at the beginning.  Preserve leading space; too bad about interior spacing
  }

  // We have replaced one DD with its equivalent explicit definition.  Rescan the line, starting at the first location where DDBGN was seen
  for(firstddbgnx=ddschbgnx;firstddbgnx<nw;++firstddbgnx){US ch2=*(US*)(wv+wilv[firstddbgnx][0]); ASSERT(!(ch2==DDEND&&(wilv[firstddbgnx][1]-wilv[firstddbgnx][0]==2)),EVEMPTYDD) if(ch2==DDBGN&&(wilv[firstddbgnx][1]-wilv[firstddbgnx][0]==2))break; }
   // search for {{; error if first thing found is }}; firstddbgnx is index of {{
 }
 ASSERT(AM(wil)>=0,EVOPENQ);  // we have to make sure there is not an unscannable remnant at the end of the last line
 // All DDs replaced. convert word list back to either text form or enqueued form
 // We searched starting with ddschbgnx looking for DDBGN, and din't find one.  Now we need to lump
 // the words together, since the spacing may be necessary
 if(ddschbgnx<nw){wilv[ddschbgnx][1]=wilv[nw-1][1]; AN(wil)=2*(AS(wil)[0]=ddschbgnx+1);}  // make one word of the last part

 w=unwordil(wil,w,0);  // the word list is where everything is.  Collect to string
 if(!(env&8))w=tokens(w,env&3);  // enqueue if called for
 EPILOG(w);
}
