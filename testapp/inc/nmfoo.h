#pragma once

#include    <iostream>

namespace foonm
{

class Foo
{
public:
    Foo();
    ~Foo();

public:
    void bar();
};

class Bar
{
public:
    Bar();
    ~Bar();

public:
    void foo(foonm::Foo* = NULL);
};

Foo* InitFooFromBar( Bar* = NULL );
Bar* InitBarFromFoo( Foo* = NULL );
std::string* DescribeIt( void* = NULL );

};
