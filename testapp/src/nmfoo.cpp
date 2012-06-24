#include    "nmfoo.h"

#include    "libqtlogger.h"

using namespace ilardm::lib::qtlogger;

namespace foonm
{

Foo::Foo()
{
    LOG_DEBUGF();
}

Foo::~Foo()
{
    LOG_DEBUGF();
}

void Foo::bar()
{
    LOG_DEBUG();
}

// ------------------------------

Bar::Bar()
{
    LOG_DEBUGF();
}

Bar::~Bar()
{
    LOG_DEBUGF();
}

void Bar::foo( foonm::Foo* p )
{
    LOG_DEBUG();
}

Foo* InitFooFromBar( Bar* p )
{
    LOG_DEBUG();

    return NULL;
}

Bar* InitBarFromFoo( Foo* p )
{
    LOG_DEBUG();

    return NULL;
}

std::string* DescribeIt( void* p )
{
    LOG_DEBUG();

    return NULL;
}

};
