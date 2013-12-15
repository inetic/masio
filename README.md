MASIO : Monadic Asio 

-------------------------------------------------------------------

Standard continuation signatures (r in our case is always void):

Cont a : { (a -> r) -> r }
Cont a >>= (\a -> Cont b) = Cont b

But Cont is actually a Monad transformer parametrized
over the Error monad. So the actual signature is:

Cont a : { (E a -> r) -> r }

