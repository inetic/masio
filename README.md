# MASIO (Monadic Asio)

Monadic interface for the Boost.Asio library.

At the center of masio is the Task object which is roughly isomorphic to the monad

Task a :: StateT (ErrorT (ContT IO)) a

* ContT for passing handlers to Asio
* ErrorT for handling error automatically between two Tasks
* StateT for canceling a chain of Tasks

The StateT is something I'm not sure about as it is not pure: the cancelation state is set outside of the asynchronous chain and checked between each bind operation.

## Example code

    asio::io_service ios;

    Canceler canceler;
    Canceler p1_canceler;

    typedef system_clock::time_point Time;

    unsigned int duration0 = 123; // milliseconds
    unsigned int duration1 = 234;

    auto p0 = wait(ios, duration0)
           >= [&p1_canceler](none_t) {
                p1_canceler.cancel();
                return success<Time>(now());
              }
           >= [](Time t) { return success(t); };

    auto p1 = with_canceler( p1_canceler
                           , wait(ios, duration1) >> success<Time>(now()))
           >= [](Time t) { return success(t); };
      
    auto p = all<Time>(p0, p1);

    using Results = pair<Error<Time>, Error<Time>>;

    p.run(canceler, [] (Error<Results> ers) {
                       // ers is the result of the computation.
                       // In this case, ers.first shall be success
                       // and ers.second shall be error::operation_aborted
                     });
                  
    ios.run();
