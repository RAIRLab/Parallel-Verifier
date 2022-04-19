

SExpressionTest.exe: sExpressionTest.cpp SExpression.o hyperslateUtils.o
	g++ -g -o $@ $^

SExpression.o: SExpression.cpp SExpression.hpp
	g++ -g -c -o $@ $<

hyperslateUtils.o: hyperslateUtils.cpp hyperslateUtils.hpp
	g++ -g -c -o $@ $<

clean:
	rm *.o
	rm *.exe