#include "ast.h"

void ExternASTNode::inject(NodeTraverser &traverser) const { traverser.apply_to(*this); }
void DefnASTNode::inject(NodeTraverser &traverser) const { traverser.apply_to(*this); }
void VariableASTExpr::inject(ExprTraverser &traverser) const { traverser.apply_to(*this); }
void LiteralDoubleASTExpr::inject(ExprTraverser &traverser) const { traverser.apply_to(*this); }
void BinOpASTExpr::inject(ExprTraverser &traverser) const { traverser.apply_to(*this); }
void CallASTExpr::inject(ExprTraverser &traverser) const { traverser.apply_to(*this); }
