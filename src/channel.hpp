#ifndef CHANNEL_H
#define CHANNEL_H

#include <list>
#include <mutex>
#include <memory>
#include <tuple>
#include <optional>
#include <condition_variable>
#include <iostream>

namespace mpsc {
    namespace detail {
        template<typename T>
        struct Inner {
            std::list<T> queue{ };
            std::size_t senders{ 1 };
        };

        template<typename T>
        struct Shared {  
            Inner<T> inner;          
            std::mutex lock{ };
            std::condition_variable signal{ };
        };
    }

    template<typename T>
    class Sender {
    public:        
        explicit Sender( std::shared_ptr<detail::Shared<T>> shared ) noexcept
            : shared_{ std::move( shared ) }
        { }

        Sender( const Sender& other );
        Sender& operator=( const Sender<T>& other );

        Sender( Sender&& ) = default;
        Sender& operator=( Sender&& ) = default;

        auto send( T value ) -> void;

        ~Sender( ) noexcept;

    private:
        std::shared_ptr<detail::Shared<T>> shared_;
    };

    template<typename T>
    inline Sender<T>::Sender( const Sender& other ) {
        std::scoped_lock{ other.shared_->lock };
        shared_ = other.shared_;
        shared_->inner.senders += 1;
    }

    template<typename T>
    inline Sender<T>& Sender<T>::operator=( const Sender& other ) {
        std::scoped_lock{ other.shared_->lock };
        shared_ = other.shared_;
        shared_->inner.senders += 1;
        return *this;
    }

    template<typename T>
    inline Sender<T>::~Sender( ) noexcept {
        bool was_last{ false };
        if ( shared_ ) {
            std::scoped_lock{ shared_->lock };
            shared_->inner.senders -= 1;
            was_last = shared_->inner.senders == 0;
        }   
        if ( was_last ) {
            shared_->signal.notify_one( );
        }    
    }

    template<typename T>
    inline auto Sender<T>::send( T value ) -> void {
        std::unique_lock lock{ shared_->lock };
        shared_->inner.queue.push_back( std::move( value ) );
        lock.unlock( );
        shared_->signal.notify_one( );
    }

    template<typename T>
    class Receiver {
    public:
        explicit Receiver( std::shared_ptr<detail::Shared<T>> shared ) noexcept
            : shared_{ std::move( shared ) }
        { }

        Receiver( Receiver&& ) = default;
        Receiver& operator=( Receiver&& ) = default;

        Receiver( const Receiver& ) = delete;
        Receiver& operator=( const Receiver& ) = delete;

        auto recv( ) -> std::optional<T>;

        auto try_recv( ) -> std::optional<T>;

    private: 

        auto recv_buffer( ) -> std::optional<T>;

        auto recv_queue( ) -> std::optional<T>;

        std::shared_ptr<detail::Shared<T>> shared_;
        std::list<T> buffer_{ };
    };

    template<typename T>
    auto Receiver<T>::try_recv( ) -> std::optional<T> {
        if ( auto value{ recv_buffer( ) }; value.has_value( ) ) {
            return value;
        }

        std::scoped_lock lock{ shared_->lock };
        if ( auto value{ recv_queue( ) }; value.has_value( ) ) {
            return value;
        }
        return std::nullopt;
    }

    template<typename T>
    auto Receiver<T>::recv( ) -> std::optional<T> {
        if ( auto value{ recv_buffer( ) }; value.has_value( ) ) {
            return value;
        }

        std::unique_lock lock{ shared_->lock };
        while ( true ) {
            if ( auto value{ recv_queue( ) }; value.has_value( ) ) {
                return value;
            }
            else if ( shared_->inner.senders < 1 ) {
                return std::nullopt;
            }
            shared_->signal.wait( lock );
        }        
    }

    template<typename T>
    auto Receiver<T>::recv_queue( ) -> std::optional<T> {
        if ( !shared_->inner.queue.empty( ) ) {                
            auto value{ std::move( shared_->inner.queue.front( ) ) };
            shared_->inner.queue.pop_front( );
            std::swap( shared_->inner.queue, buffer_ );
            return value;
        }
        return std::nullopt;
    }

    template<typename T>
    auto Receiver<T>::recv_buffer( ) -> std::optional<T> {
        if ( !buffer_.empty( ) ) {
            auto value{ std::move( buffer_.front( ) ) };
            buffer_.pop_front( );
            return value;
        }
        return std::nullopt;
    }

    template<typename T>
    std::pair<Sender<T>, Receiver<T>> create_unbounded( ) {
        auto shared{ std::make_shared<detail::Shared<T>>( ) };
        return { Sender<T>{ shared }, Receiver<T>{ shared } };
    } 
}

#endif