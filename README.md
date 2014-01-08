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

    CancelerPtr canceler    = make_shared<Canceler>();
    CancelerPtr p1_canceler = make_shared<Canceler>();

    typedef system_clock::time_point Time;

    unsigned int duration0 = 123; // milliseconds
    unsigned int duration1 = 234;

    Task<Time>::Ptr p0 = sleep<Time>(ios, duration0, [p1_canceler]() {
        p1_canceler->cancel();
        return success<Time>(now());
      })
      ->bind<Time>([](Time t) { return success(t); });

    Task<Time>::Ptr p1 = with_canceler<Time>( p1_canceler
                                            , sleep<Time>(ios, duration1, []() {
        return success<Time>(now());
      }))
      ->bind<Time>([](Time t) { return success(t); });
      
    All<Time>::Ptr p = all<Time>(p0, p1);

    p->run(canceler, [] (Error<std::vector<Error<Time>>> ers) {
                       // ers is the result of the computation
                       // ers[0] shall be success and ers[1] shall be error::operation_aborted
                     });
                  
    ios.run();
