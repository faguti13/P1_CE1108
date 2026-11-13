#include "ir_tac.h"
#include "cfg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static Nodo *mk(TipoNodo t, const char *s) { Nodo *n=calloc(1,sizeof *n); n->tipo=t; n->texto=strdup(s?s:"");return n; }
int main(void){
 Nodo *root=mk(NODO_PROGRAMA,""), *imp=mk(NODO_LISTA,""), *body=mk(NODO_LISTA,"");
 Nodo *fun=mk(NODO_FUNCION,"main"), *type=mk(NODO_TIPO,"vacio"), *args=mk(NODO_LISTA,""), *stmts=mk(NODO_BLOQUE,"");
 Nodo *decl=mk(NODO_DECLARACION,"a"), *typ=mk(NODO_TIPO,"ent"), *val=mk(NODO_LITERAL,"3"); val->val=3;typ->sig=val;decl->hijo=typ;
 Nodo *wh=mk(NODO_MIENTRAS,""), *cond=mk(NODO_EXP,"<"), *va=mk(NODO_IDENT,"a"),*ten=mk(NODO_LITERAL,"10");ten->val=10;va->sig=ten;cond->hijo=va;
 Nodo *b=mk(NODO_BLOQUE,""), *as=mk(NODO_ASIGNACION,"="), *lhs=mk(NODO_IDENT,"a"),*plus=mk(NODO_EXP,"+"),*aa=mk(NODO_IDENT,"a"),*one=mk(NODO_LITERAL,"1");one->val=1;aa->sig=one;plus->hijo=aa;lhs->sig=plus;as->hijo=lhs;b->hijo=as;cond->sig=b;wh->hijo=cond; decl->sig=wh;stmts->hijo=decl;
 root->hijo=imp;imp->sig=body;body->hijo=fun;fun->hijo=type;type->sig=args;args->sig=stmts;
 CodigoTac c;GrafoFlujo g;tac_generar(root,&c);tac_imprimir(&c,stdout);cfg_construir(&c,&g);cfg_imprimir(&c,&g,stdout);
 if (c.n < 6 || g.n < 3) {
     return 1;
 }

 cfg_liberar(&g);
 tac_liberar(&c);
 return 0;
}
