//
// Created by 最上川 on 2021/12/16.
//

#ifndef CODESNIPPET_TEMPLATE_EXPRESSION_H
#define CODESNIPPET_TEMPLATE_EXPRESSION_H

#include <iostream>
#include <cmath>
// 表达式类型

// 文字量
class DExprLiteral
{
    double a_;
public:
    DExprLiteral(double a) : a_(a) {}
    double operator()(double x) const { return a_; }
};

// 自变量
class DExprIdentity
{
public:
    double operator()(double x) const { return x;}
};

// 双目操作
template<typename A, typename B, typename Op>
class DBinExprOp
{
    A a_;
    B b_;
public:
    DBinExprOp(const A& a, const B& b) : a_(a), b_(b) { }
    double operator()(double x) const { return Op::apply(a_(x), b_(x)); }
};

// 单目操作
template<typename A, typename Op>
class DUnaryExprOp
{
    A a_;
public:
    DUnaryExprOp(const A& a) : a_(a) { }
    double operator()(double x) const { return Op::apply(a_(x)); }
};

// 表达式
template<typename A>
class DExpr
{
    A a_;
public:
    DExpr() {}
    DExpr(const A& a) : a_(a) { }
    double operator()(double x) const { return a_(x); }
};

// 运算符，模板参数A, B为参加运算的表达式类型
// operator/, division
class DApDiv { public:static double apply(double a, double b) { return a / b; } };
template<typename A, typename B>
DExpr<DBinExprOp<DExpr<A>, DExpr<B>, DApDiv>>
operator/(const DExpr<A>& a, const DExpr<B>& b)
{
    typedef DBinExprOp<DExpr<A>, DExpr<B>, DApDiv> ExprT;
    return DExpr<ExprT>(ExprT(a, b));
}


// operator +, addition
class DApAdd { public:static double apply(double a, double b) { return a + b; } };
template<typename A, typename B>
DExpr<DBinExprOp<DExpr<A>, DExpr<B>, DApAdd>>
operator+(const DExpr<A>& a, const DExpr<B>& b)
{
    typedef DBinExprOp<DExpr<A>, DExpr<B>, DApAdd> ExprT;
    return DExpr<ExprT>(ExprT(a, b));
}

// sqrt(), square rooting
class DApSqrt {public:static double apply(double a) { return std::sqrt(a); }};
template<typename A>
DExpr<DUnaryExprOp<DExpr<A>, DApSqrt>>
sqrt(const DExpr<A>& a)
{
    typedef DUnaryExprOp<DExpr<A>, DApSqrt> ExprT;
    return DExpr<ExprT>(ExprT(a));
}

// operator-, negative sign
class DApNeg{public:static double apply(double a) { return -a; } };
template<typename A>
DExpr<DUnaryExprOp<DExpr<A>, DApNeg>>
operator-(const DExpr<A>& a)
{
    typedef DUnaryExprOp<DExpr<A>, DApNeg> ExprT;
    return DExpr<ExprT>(ExprT(a));
}

// evaluate()
template<typename Expr>
void evaluate(const DExpr<Expr>& expr, double start, double end, double step)
{
    for (double i = start; i < end; i += step)
        std::cout << expr(i) << ' ';
}
#endif //CODESNIPPET_TEMPLATE_EXPRESSION_H
