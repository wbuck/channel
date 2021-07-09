#include "channel.hpp"
#include <iostream>
#include <thread>
#include <unistd.h>
#include <tuple>
#include <string>
#define FMT_HEADER_ONLY
#include <fmt/format.h>


class Test { 
public:
    explicit Test( std::string value )
        : value_{ std::move( value ) }
    { }

    std::string_view operator( )( ) {
        return value_;
    }

private:
    std::string value_;
};

static void worker1( mpsc::Receiver<Test> rx ) {
    while ( true ) {
        if ( auto value{ rx.recv( ) }; value.has_value( ) ) {
            fmt::print( "{}\n", value->operator( )( ) );
        }
        else {
            return;
        } 
    }
}

static void worker2( mpsc::Sender<Test> tx ) {
    auto count = 10;
    while ( --count > 0 ) {
        sleep( 1 );
        tx.send( Test{ std::to_string( count ) } );
    }
    std::cout << "Dropping\n";
}

int main( ) {
    std::pair pair{ mpsc::create_unbounded<Test>( ) };

    std::thread t1{ [ rx = std::move( pair.second ) ] ( ) mutable { 
        worker1( std::move( rx ) ); 
    } };

    auto tx{ pair.first };
    std::thread t2{ [ tx = std::move( tx ) ] { 
        worker2( std::move( tx ) ); 
    } };

    {
        auto tx2{ std::move( pair.first ) };
        auto count{ 6 };
        while ( --count > 0 ) {
            sleep( 1 );
            tx2.send( Test{ std::to_string( count ) } );
        }
        std::cout << "Dropping\n";
    }

    t1.join( );
    t2.join( );
}