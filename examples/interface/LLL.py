import El;

m = 10
n = 5

A = El.Matrix(El.dTag)
El.Uniform( A, m, n, 0., 10.)
El.Round( A )
El.Print( A, "A" )

B=El.Matrix(El.dTag)

El.Copy( A, B )
QR = El.LLL(B,0.75)
El.Print(B,"B(0.75)")
delta = El.LLLDelta(QR)
print "delta=", delta
print ""

El.Copy( A, B )
QR = El.LLL(B,0.5)
El.Print(B,"B(0.5)")
delta = El.LLLDelta(QR)
print "delta=", delta

