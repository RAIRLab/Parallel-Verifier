

SExpressionTest.exe: sExpressionTest.cpp SExpression.o
	g++ -o $@ $<

SExpression.o: SExpression.cpp
	g++ -c -o $@ $<