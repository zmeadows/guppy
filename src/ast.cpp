#include "ast.h"

void ASTExternNode::accept(ASTFunctor &functor) const { functor.apply(*this); }
void ASTDefnNode::accept(ASTFunctor &functor) const { functor.apply(*this); }
void VariableExpr::accept(ASTFunctor &functor) const { functor.apply(*this); }
void LiteralDoubleExpr::accept(ASTFunctor &functor) const { functor.apply(*this); }
void BinOpExpr::accept(ASTFunctor &functor) const { functor.apply(*this); }
void CallExpr::accept(ASTFunctor &functor) const { functor.apply(*this); }
