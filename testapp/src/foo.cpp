#include    <iostream>

#include    "foo.h"
#include    "libqtlogger.h"

using namespace ilardm::lib::qtlogger;

Foo::Foo()
{
#ifdef _DEBUG
    LOG_DEBUGF();
#endif

    // TODO
}

Foo::~Foo()
{
#ifdef _DEBUG
    LOG_DEBUGF();
#endif

    // TODO
}

void Foo::bar( void* bar )
{
#ifdef _DEBUG
    LOG_DEBUGF( "%p", bar );
#endif

    if ( !bar )
    {
        LOG_WARN( "null pointer passed" );
    }

    LOG_WARNF( "not impemented" );
}
