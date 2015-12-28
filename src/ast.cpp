#include "ast.h"

void ExternASTNode::inject(NodeTraverser &functor) const { functor.apply(*this); }
void DefnASTNode::inject(NodeTraverser &functor) const { functor.apply(*this); }
void VariableASTExpr::inject(ExprTraverser &functor) const { functor.apply(*this); }
void LiteralDoubleASTExpr::inject(ExprTraverser &functor) const { functor.apply(*this); }
void BinOpASTExpr::inject(ExprTraverser &functor) const { functor.apply(*this); }
void CallASTExpr::inject(ExprTraverser &functor) const { functor.apply(*this); }
