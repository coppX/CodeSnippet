//
// Created by FDC on 2021/12/14.
//

template<bool c, typename Then, typename Else> class IF_ {};

template<typename Then, typename Else>
class IF_<true, Then, Else> { public: typedef Then reType; };

template<typename Then, typename Else>
class IF_<false, Then, Else> { public: typedef Else reType; };

template<template<typename> class Condition, typename Statement>
class WHILE_ {
    template<typename Statement> class STOP { public: typedef Statement reType; };
public:
    typedef typename
        IF_<Condition<Statement>::ret,
        WHILE_<Condition, typename Statement::Next>,
        STOP<Statement>>::reType::reType
    reType;
};
