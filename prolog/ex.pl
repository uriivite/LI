prod([], 0).
prod([X], X):-!.
prod([X|L], P):-prod(L, P1), P is X * P1.

pescalar([], [_], _):-fail.
pescalar([_], [], _):-fail.
pescalar([X], [Y], P):- P is X * Y, !.
pescalar([X|L1], [Y|L2], P):-pescalar(L1, L2, P1), P is P1 + (X * Y).

union([], [], []).
union(L, [], L):-!.
union(L, [X|L2], [X|OUT]):-not(member(X, L)), union(L, L2, OUT), !.
union(L, [X|L2], OUT):-member(X, L), union(L, L2, OUT), !.

intersection(_, [], []):-!.
intersection(L1, [X|L2], [X|OUT]):-member(X, L1), intersection(L1, L2, OUT),!.

concat([], L, L).
concat([Head1|Tail1], L2, [Head1|TailO]):-concat(Tail1, L2, TailO).

last([], _):-fail.
last(L, X):-concat(_, [X], L), !.

reverse([], []).
reverse([Head|Tail], L):-reverse(Tail, L1), concat(L1, [Head], L).

fib(1, 1):-!.
fib(2, 1):-!.
fib(N, F):-
    N1 is N - 1,
    fib(N1, F1),
    N2 is N - 2,
    fib(N2, F2),
    F is F1 + F2.

