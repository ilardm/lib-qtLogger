#include    <iostream>
#include    <stdio.h>

#include    <QString>
#include    <QSettings>

#include    "main.h"
#include    "foo.h"
#include    "nmfoo.h"

#include    "libqtlogger.h"
#include    "consoleappender.h"
#include    "fileappender.h"

using namespace ilardm::lib::qtlogger;

int main( int argc, char** argv )
{
    QSettings settings( QSettings::IniFormat,
                        QSettings::UserScope,
                        "ilardm",
                        "qtloggersample"
                        );
    LQTL_START_LOGGING( &settings );
    LQTL_UNUSED_VARIABLE( __qtLoggerConfigFileSet );
    LQTL_ADD_LOG_WRITER( new ConsoleAppender() );
    LQTL_ADD_LOG_WRITER( new FileAppender( QString("test-application.log") ));

    LOG_DEBUG("startup");
    LOG_DEBUGX( "argv[0]: '%s' hex:",
                argv[0], sizeof( char )*strlen( argv[0] ),
                argv[0]
              );

    // test hexdata
    char hexdata[24];
    memset( hexdata, 0, sizeof( hexdata ) );
    strncpy( hexdata + 4, "hexdata", 7 );
    LOG_DEBUGX( "test hexdata:",
                hexdata, sizeof( hexdata )
                );

#ifdef _DEBUG
    std::clog << "_DEBUG" << std::endl;
#endif

#ifdef _RELEASE
    std::clog << "_RELEASE" << std::endl;
#endif

    QString sampleSection("qtloggersample");
    QString lastRunKey("last run");
    QString lastRun("");

    LOG_DEBUG( "read present settings section '%s key '%s'",
               LQTL_QSTRINGCHAR( sampleSection ),
               LQTL_QSTRINGCHAR( lastRunKey )
               );
    settings.beginGroup( sampleSection );
    lastRun = settings.value( lastRunKey, QString("(null)") ).toString();
    settings.endGroup();
    LOG_DEBUG( "value: '%s'",
               LQTL_QSTRINGCHAR( lastRun )
               );

    lastRun = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    LOG_DEBUG( "save section '%s' key '%s' value '%s'",
               LQTL_QSTRINGCHAR( sampleSection ),
               LQTL_QSTRINGCHAR( lastRunKey),
               LQTL_QSTRINGCHAR( lastRun )
               );
    settings.beginGroup( sampleSection );
    settings.setValue( lastRunKey, lastRun );
    settings.endGroup();
    settings.sync();

    std::cout << "press enter to continue" << std::endl;
    getchar();

    int i = 0;
    const char* buf = "0xdeadbeef";
    LOG_DEBUG(      "debug %d",     ++i );
    LOG_DEBUGX(     "debugx %d",    buf, strlen(buf), ++i );
    LOG_DEBUGF(     "debugf %d",    ++i );
    LOG_DEBUGXF(    "degugxf %d",   buf, strlen(buf), ++i );
    LOG_LOG(        "log %d",       ++i );
    LOG_LOGX(       "logx %d",      buf, strlen(buf), ++i );
    LOG_LOGF(       "logf %d",      ++i );
    LOG_LOGXF(      "logxf %d",     buf, strlen(buf), ++i );
    LOG_WARN(       "warning %d",   ++i );
    LOG_WARNX(      "warnx %d",     buf, strlen(buf), ++i );
    LOG_WARNF(      "warnf %d",     ++i );
    LOG_WARNXF(     "warnxf %d",    buf, strlen(buf), ++i );
    LOG_ERROR(      "error %d",     ++i );
    LOG_ERRORX(     "errorx %d",    buf, strlen(buf), ++i );

    QString smth("qstring test");
    LOG_DEBUG( "smth: %s", LQTL_QSTRINGCHAR(smth) );

    std::cout << "press enter to continue" << std::endl;
    getchar();

    Foo* foo = new Foo();
    if ( foo )
    {
        foo->bar( NULL );
        delete( foo );
    }
    else
    {
        LOG_WARN( "unable to create foo object" );
    }

    foonm::Foo* nmfoo = new foonm::Foo();
    if ( nmfoo )
    {
        nmfoo->bar();
        delete( nmfoo );
        nmfoo = foonm::InitFooFromBar( NULL );
    }
    else
    {
        LOG_WARN( "unable to create nmfoo object" );
    }

    foonm::Bar* nmbar = new foonm::Bar();
    if ( nmbar )
    {
        nmbar->foo();
        delete( nmbar );
        nmbar = foonm::InitBarFromFoo( NULL );
    }
    else
    {
        LOG_WARN( "unable to create nmbar object" );
    }

    foonm::DescribeIt();

    // log-level on-air editing
    LOG_DEBUG("log-level on-air editing");
    std::clog << "press enter to continue" << std::endl;
    getchar();
    QMap< QString, QtLogger::MODULE_LEVEL* > mmap = LQTL_GET_KNOWN_MODULES_LEVELS();
    QList< QString > mm_modules = mmap.keys();
    foreach ( QString module, mm_modules )
    {
        std::cout << "module: "
                << module.toStdString()
                << "\tlevel: "
                << mmap.value( module )->level
                << " '" << LQTL_DESCRIBE_LLEVEL( mmap.value( module )->level ).toStdString() << "'"
                << std::endl;
    }

    LOG_DEBUG("exit from main application");

    LQTL_FINISH_LOGGING();
    return 0;
}

