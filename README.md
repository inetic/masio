# MASIO (Monadic Asio)

Standard continuation signatures (r in our case is always void):

               +-> Run <---+
               |           |
               + Rest +    |
               |      |    |
    Cont a : { (a -> r) -> r }
    
    Cont a >>= (\a -> Cont b) = Cont b
    
    +-> Cont a <--+          +-> Cont b <--+
    |             |          |             |
    [(a -> r) -> r] -> [a -> [(b -> r) -> r]] -> [(b -> r) -> r]
     |      |     |           |      |     |
     + Rest +     |           + Rest +     |
     |            |           |            |
     +-> Run <----+           +-> Run <----+

But in our case, Cont is a Monad transformer parametrized
over the Error monad. So the signature becomes:

    Cont a : { (E a -> r) -> r }

    [(E a -> r) -> r] -> [a -> [(E b -> r) -> r]] -> [(E b -> r) -> r]

