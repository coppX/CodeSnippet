//
// Created by 最上川 on 2021/12/16.
//

#include "template_expression.h"

int main()
{
    DExpr<DExprIdentity> x;
    evaluate(-x / sqrt(DExpr<DExprLiteral>(1.0) + x), 0.0, 10.0, 1.0);
}