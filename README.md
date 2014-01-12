# MASIO (Monadic Asio)

Monadic interface for the Boost.Asio library.

Task a :: StateT (ErrorT (ContT IO)) a

* ContT for passing handlers to Asio
* ErrorT for automatic handling of errors
* StateT for canceling a chain of Tasks


