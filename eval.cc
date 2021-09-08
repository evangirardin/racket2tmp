#include <type_traits>
#include <iostream>
#include <string>
template<int N> struct Int {};
template<bool B> struct Bool {};
template<char C> struct Char {};
template<uint32_t N>
struct LitStr {
  char buf[N]{};
  constexpr LitStr(const char (&s)[N]) {
    for (uint32_t i = 0; i < N; ++i)
      buf[i] = s[i];
  }
};
template<LitStr S> struct String {};
template<typename Exp, typename NextStmt> struct Stmt {};
struct EmptyStmt {};
template<LitStr Label, typename Exp> struct Define {};
template<LitStr Label> struct Var {};
template<LitStr Param, typename Body> struct Lambda {};
template<LitStr Id, typename Body> struct AltLambda {};
template<typename Fun, typename... Args> struct App {};
template<LitStr Fun> struct PrimOp {};
template<typename Condition, typename Then, typename Else> struct If {};
//template<typename Head, typename Tail> struct Cons {};
//struct Empty {};
template<typename...> struct Begin {};
struct BeginEmpty {};
template<typename Exp>
struct Let {};
struct Void {}; // UGGHHHHH

template<typename> struct Quoted {};
template<typename... Items> struct QuotedList {};

template<LitStr E> struct Error {};
template<typename T> struct IsError : std::false_type {};
template<LitStr E> struct IsError<Error<E>> : std::true_type {};

// Binary operators
template<char Op, typename Fst, typename Snd>
requires (Op == '+' || Op == '-' || Op == '*' || Op == '/' || Op == '%' || Op == '&' || Op == '|')
struct Bin {};
template<LitStr Op, typename Fst, typename Snd>
struct Test {};
template<bool, typename...> struct AndOr {};

// Unary operators
template<char Op, typename Exp>
requires (Op == '+' || Op == '-' || Op == '*' || Op == '/' || Op == '~')
struct Unary {};

/* Environments */
struct EnvEmpty {};
template<LitStr Label, typename Exp, typename EnvRest> struct EnvNode {};
template<LitStr Label, typename Env> struct EnvNodeFinder;
// EnvNodeFinder: found
template<LitStr Label, typename Exp, typename EnvRest>
struct EnvNodeFinder<Label, EnvNode<Label,Exp,EnvRest>> {
  using value = Exp;
};
// EnvNodeFinder: not found
template<LitStr Label> struct EnvNodeFinder<Label, EnvEmpty> { using value = Error<"Identifier not found in scope">; };
// EnvNodeFinder: recursive
template<LitStr Label, LitStr OtherLabel, typename Exp, typename EnvRest>
struct EnvNodeFinder<Label, EnvNode<OtherLabel,Exp,EnvRest>> {
  using value = typename EnvNodeFinder<Label, EnvRest>::value;
};

/* Closures */
template<typename Fun, typename Env> struct Closure {};

/* Evaluation */
// Eval: Literals (assumed fallback)
template<typename Exp, typename Env>
struct Eval { using value = Exp; };
// Eval: Var
template<LitStr Label, typename Env>
struct Eval<Var<Label>, Env> {
  using value = typename EnvNodeFinder<Label, Env>::value;
};
// VAR SPECIAL CASES FOR PRIMOPS
template<LitStr Label, typename Env>
requires (std::same_as<PrimOp<Label>,PrimOp<"+">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"-">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"*">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"/">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"%">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"cons">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"first">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"rest">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"append">>)
      || (std::same_as<PrimOp<Label>,PrimOp<"list">>)
struct Eval<Var<Label>, Env> {
  using value = PrimOp<Label>;
};

// Eval: Lambda
template<LitStr Param, typename Body, typename Env>
struct Eval<Lambda<Param, Body>, Env> {
  using value = Closure<Lambda<Param,Body>, Env>;
};
template<LitStr Id, typename Body, typename Env>
struct Eval<AltLambda<Id, Body>, Env> {
  using value = Error<"rest-id lambda expressions are not supported, they are bad">;
};
// EvalAppHelper
template<typename, typename, typename...> struct EvalAppHelper { using value = Error<"Application failure">; };
template<LitStr Param, typename Body, typename Env, typename OldEnv, typename Arg, typename... Args>
struct EvalAppHelper<Closure<Lambda<Param,Body>,Env>, OldEnv, Arg, Args...> {
  using value = typename App<typename Eval<Body,EnvNode<Param,Arg,Env>>::value, Args...>::value;
};

template<typename> struct IsPrimOp : std::false_type {};
template<LitStr S> struct IsPrimOp<PrimOp<S>> : std::true_type {};

template<LitStr Op, typename OldEnv, typename... Args>
struct EvalAppHelper<PrimOp<Op>, OldEnv, Args...> {
  using value = typename Eval<App<PrimOp<Op>, Args...>, OldEnv>::value;
};
// Eval: App
template<typename Fun, typename Env, typename... Args> requires (!IsPrimOp<Fun>::value)
struct Eval<App<Fun,Args...>, Env> {
  using value = Error<"Application failure">;
};/*
template<LitStr Label, typename Env, typename... Args>
struct Eval<App<Var<Label>, Args...>, Env> {
  using value = typename Eval<App<typename Eval<Var<Label>,Env>::value, Args...>, Env>::value;
};
*/
template<typename> struct IsVar : std::false_type {};
template<LitStr Label> struct IsVar<Var<Label>> : std::true_type {};

template<typename Fun, typename Env> requires (!IsVar<Fun>::value && !IsPrimOp<Fun>::value) // 0 args - is this even used?
struct Eval<App<Fun>, Env> {
  using value = typename Eval<Fun,Env>::value;
};
template<typename Fun, typename Env> requires (IsVar<Fun>::value && !IsPrimOp<Fun>::value) // 0 args (primop disambiguation)
struct Eval<App<Fun>, Env> {
  using value = typename Eval<App<typename Eval<Fun,Env>::value>,Env>::value;
};

template<typename Fun, typename Env, typename Arg, typename... Args> requires (!IsPrimOp<Fun>::value) // > 0 args
struct Eval<App<Fun,Arg,Args...>, Env> {
  using value = typename EvalAppHelper<typename Eval<Fun,Env>::value, Env, typename Eval<Arg,Env>::value>::value;
};

template<LitStr Label, typename Env, typename Arg, typename... Args> // specialization to disambiguate PrimOp
struct Eval<App<Var<Label>,Arg,Args...>, Env> {
  using value = typename Eval<App<typename Eval<Var<Label>,Env>::value,Arg,Args...>, Env>::value;
};

template<char Op, typename, typename> struct EvalPrimOpHelper {
  using value = Error<"Primitive operation failure">;
};
template<char Op, int N1, int N2>
requires (Op == '+' || Op == '-' || Op == '*' || Op == '/' || Op == '%')
struct EvalPrimOpHelper<Op, Int<N1>, Int<N2>> {
  using value = typename std::conditional<
    (Op == '%' || Op == '/') && (N2 == 0),
    Error<"Attempted division/modulo by zero">,
    Int<
      (Op == '+') ? (N1 + N2) : (
      (Op == '-') ? (N1 - N2) : (
      (Op == '*') ? (N1 * N2) : (
      (Op == '/') ? (N1 / (N2 ? N2 : 1)) : (
      (Op == '%') ? (N1 % (N2 ? N2 : 1)) : 0))))
    >
  >::type;
};

// Eval: PrimOp
/*
template<LitStr Fun, typename Env, typename... Args>
struct Eval<App<PrimOp<Fun>, Args...>, Env> {};
*/
// [+, *]
template<typename PlusStar, typename Env, typename... Args>
requires (std::same_as<PlusStar, PrimOp<"+">> || std::same_as<PlusStar, PrimOp<"*">>)
struct Eval<App<PlusStar, Args...>, Env> {
  using value = Int<std::is_same<PlusStar,PrimOp<"*">>::value>;
};

template<typename PlusStar, typename Env>
requires (std::same_as<PlusStar, PrimOp<"+">> || std::same_as<PlusStar, PrimOp<"*">>)
struct Eval<App<PlusStar>, Env> {
  using value = Int<std::is_same<PlusStar,PrimOp<"*">>::value>;
};

template<typename PlusStar, typename Env, typename Arg, typename... Args>
requires (std::same_as<PlusStar, PrimOp<"+">> || std::same_as<PlusStar, PrimOp<"*">>)
struct Eval<App<PlusStar, Arg, Args...>, Env> {
  using value = typename std::conditional<
    IsError<typename Eval<Arg,Env>::value>::value,
    typename Eval<Arg,Env>::value,
    typename EvalPrimOpHelper<(std::is_same<PlusStar,PrimOp<"+">>::value ? '+' : '*'), typename Eval<Arg,Env>::value, typename Eval<App<PlusStar, Args...>,Env>::value>::value
  >::type;
};

template<typename MinusSlash, typename Env, typename Arg1, typename Arg2, typename... Args>
requires (std::same_as<MinusSlash, PrimOp<"-">> || std::same_as<MinusSlash, PrimOp<"/">>)
struct Eval<App<MinusSlash, Arg1, Arg2, Args...>, Env> {
  using value = typename Eval<App<MinusSlash,
    typename EvalPrimOpHelper<
      (std::is_same<MinusSlash,PrimOp<"-">>::value ? '-' : '/'),
      typename Eval<Arg1,Env>::value,
      typename Eval<Arg2,Env>::value
    >::value,
    Args...
  >, Env>::value;
};

template<typename MinusSlash, typename Env, typename Arg1, typename Arg2>
requires (std::same_as<MinusSlash, PrimOp<"-">> || std::same_as<MinusSlash, PrimOp<"/">>)
struct Eval<App<MinusSlash, Arg1, Arg2>, Env> {
  using value = typename EvalPrimOpHelper<
    (std::is_same<MinusSlash,PrimOp<"/">>::value ? '/' : '-'),
    typename Eval<Arg1,Env>::value,
    typename Eval<Arg2,Env>::value
  >::value;
};

template<typename MinusSlash, typename Env, typename Arg>
requires (std::same_as<MinusSlash, PrimOp<"-">> || std::same_as<MinusSlash, PrimOp<"/">>)
struct Eval<App<MinusSlash, Arg>, Env> {
  using value = typename std::conditional<
    IsError<typename Eval<Arg,Env>::value>::value,
    typename Eval<Arg,Env>::value,
    typename EvalPrimOpHelper<
      (std::is_same<MinusSlash,PrimOp<"/">>::value ? '/' : '-'),
      Int<std::is_same<MinusSlash,PrimOp<"/">>::value>,
      typename Eval<Arg,Env>::value
    >::value
  >::type;
};

template<typename MinusSlash, typename Env>
requires (std::same_as<MinusSlash, PrimOp<"-">> || std::same_as<MinusSlash, PrimOp<"/">>)
struct Eval<App<MinusSlash>, Env> {
  using value = Error<"- and / require at least 1 argument">;
};

template<typename Env, typename Arg1, typename Arg2>
struct Eval<App<PrimOp<"%">,Arg1,Arg2>, Env> {
  using value = typename EvalPrimOpHelper<'%', typename Eval<Arg1,Env>::value, typename Eval<Arg2,Env>::value>::value;
};

// List operations -- QQList = Quoted<QuotedList<...>>
template<typename> struct IsQQList : std::false_type {};
template<typename... Lst> struct IsQQList<Quoted<QuotedList<Lst...>>> : std::true_type {};
template<typename> struct IsEmptyQQList : std::false_type {};
template<> struct IsEmptyQQList<Quoted<QuotedList<>>> : std::true_type {};

template<typename> struct IsPrimitiveData : std::false_type {}; // this sucks
template<int N> struct IsPrimitiveData<Int<N>> : std::true_type {};
template<bool B> struct IsPrimitiveData<Bool<B>> : std::true_type {};
template<char C> struct IsPrimitiveData<Char<C>> : std::true_type {};
template<LitStr S> struct IsPrimitiveData<String<S>> : std::true_type {};

template<typename T, typename Env> requires (IsPrimitiveData<T>::value)
struct Eval<Quoted<T>, Env> { // Quoted lits are eval'd into plain lits
  using value = T;
};

template<typename Env, int N>
struct Eval<Quoted<Int<N>>, Env> { // Quoted nums are eval'd into plain nums
  using value = Int<N>;
};

template<typename Env, typename Arg1, typename Arg2>
struct Eval<App<PrimOp<"cons">,Arg1,Arg2>, Env> {
  using value = typename std::conditional<
    IsQQList<typename Eval<Arg2,Env>::value>::value,
    typename Eval<App<PrimOp<"cons">, Arg1, typename Eval<Arg2,Env>::value>, Env>::value,
    Error<"cons requires a list second argument">
  >::type;
};
template<typename Env, typename Arg1, typename... Lst>
struct Eval<App<PrimOp<"cons">,Arg1,Quoted<QuotedList<Lst...>>>, Env> {
  using value = Quoted<QuotedList<typename Eval<Arg1,Env>::value, Lst...>>;
};

template<typename Env, typename Arg>
struct Eval<App<PrimOp<"first">,Arg>, Env> {
  using value = typename std::conditional<
    IsQQList<typename Eval<Arg,Env>::value>::value && !IsEmptyQQList<typename Eval<Arg,Env>::value>::value,
    typename Eval<App<PrimOp<"first">, typename Eval<Arg,Env>::value>, Env>::value,
    Error<"first requires a nonempty list">
  >::type;
};
template<typename Env, typename Item, typename... Lst>
struct Eval<App<PrimOp<"first">,Quoted<QuotedList<Item, Lst...>>>, Env> {
  using value = Item;
};
template<typename Env, typename Arg>
struct Eval<App<PrimOp<"rest">,Arg>, Env> {
  using value = typename std::conditional<
    IsQQList<typename Eval<Arg,Env>::value>::value && !IsEmptyQQList<typename Eval<Arg,Env>::value>::value,
    typename Eval<App<PrimOp<"rest">, typename Eval<Arg,Env>::value>, Env>::value,
    Error<"rest requires a nonempty list">
  >::type;
};
template<typename Env, typename Item, typename... Lst>
struct Eval<App<PrimOp<"rest">,Quoted<QuotedList<Item, Lst...>>>, Env> {
  using value = Quoted<QuotedList<Lst...>>;
};
template<typename Env, typename Arg1, typename Arg2>
struct Eval<App<PrimOp<"append">,Arg1,Arg2>, Env> {
  using value = typename std::conditional<
    IsQQList<typename Eval<Arg1,Env>::value>::value && IsQQList<typename Eval<Arg2,Env>::value>::value,
    typename Eval<App<PrimOp<"append">,typename Eval<Arg1,Env>::value,typename Eval<Arg2,Env>::value>, Env>::value,
    Error<"append requires two list arguments">
  >::type;
};
template<typename Env, typename... Lst1, typename... Lst2>
struct Eval<App<PrimOp<"append">,Quoted<QuotedList<Lst1...>>,Quoted<QuotedList<Lst2...>>>, Env> {
  using value = Quoted<QuotedList<Lst1...,Lst2...>>;
};
template<typename... Args, typename Env>
struct Eval<App<PrimOp<"list">,Args...>, Env> {
  using value = Quoted<QuotedList<typename Eval<Args,Env>::value...>>;
};

// EvalIfHelper [True]
template<typename, typename Then, typename> struct EvalIfHelper { using value = Then; };
// [false]
template<typename Then, typename Else>
struct EvalIfHelper<Bool<false>, Then, Else> { using value = Else; };

// Eval: Test/inequalities
template<LitStr S1, LitStr S2>
struct LitStrEqual : std::false_type {};
template<LitStr S>
struct LitStrEqual<S,S> : std::true_type {};

template<LitStr Op, typename Fst, typename Snd>
struct TestEvalHelper {
  using value = Error<"Arithmetic operation failure">;
};
template<LitStr Op, int N1, int N2>
struct TestEvalHelper<Op,Int<N1>,Int<N2>> {
  using value = Bool<
    LitStrEqual<Op,">">::value ? (N1 > N2) : (
    LitStrEqual<Op,">=">::value ? (N1 >= N2) : (
    LitStrEqual<Op,"<">::value ? (N1 < N2) : (
    LitStrEqual<Op,"<=">::value ? (N1 <= N2) : (
    LitStrEqual<Op,"=">::value ? (N1 == N2) : false))))
  >;
};
template<LitStr Op, typename Fst, typename Snd, typename Env>
struct Eval<Test<Op,Fst,Snd>, Env> {
  using value = typename TestEvalHelper<Op,typename Eval<Fst,Env>::value,typename Eval<Snd,Env>::value>::value;
};

// Eval: Stmt [Define]
template<LitStr Label, typename Exp, typename Next, typename Env>
struct Eval<Stmt<Define<Label,Exp>, Next>, Env> {
  using value = typename std::conditional<
    IsError<typename Eval<Exp,Env>::value>::value,
    typename Eval<Exp,Env>::value,
    typename Eval<Next,EnvNode<Label,typename Eval<Exp,Env>::value,Env>>::value
  >::type;
};

// Eval: Stmt [Exp]
template<typename Exp, typename Next, typename Env>
struct Eval<Stmt<Exp,Next>, Env> {
  using value = typename std::conditional<
    IsError<typename Eval<Exp,Env>::value>::value,
    typename Eval<Exp,Env>::value,
    typename std::conditional<
      IsError<typename Eval<Next,Env>::value>::value,
      typename Eval<Next,Env>::value,
      Stmt<typename Eval<Exp,Env>::value, typename Eval<Next,Env>::value>
    >::type
  >::type;
};

// Eval: If
template<typename Condition, typename Then, typename Else, typename Env>
struct Eval<If<Condition,Then,Else>, Env> {
  using value = typename EvalIfHelper<typename Eval<Condition,Env>::value, typename Eval<Then,Env>::value, typename Eval<Else,Env>::value>::value;
};

template<typename... Rest, typename Env>
struct Eval<Begin<Rest...>, Env> {
  using value = Error<"fatal error in begin (are you sure this happened just now?)">;
};

// Eval: Begin [Define]
template<LitStr Label, typename Exp, typename... Rest, typename Env>
struct Eval<Begin<Define<Label,Exp>,Rest...>, Env> {
  using value = typename Eval<Stmt<Define<Label,Exp>,Begin<Rest...>>, Env>::value;
};

// Eval: Begin [Exp]
template<typename Env>
struct Eval<Begin<>, Env> {
  using value = Void;
};

template<typename Exp, typename Env>
struct Eval<Begin<Exp>, Env> {
  using value = typename Eval<Exp,Env>::value;
};

template<typename Exp, typename... Rest, typename Env>
struct Eval<Begin<Exp,Rest...>, Env> {
  using value = typename Eval<Begin<Rest...>,Env>::value;
};

// Eval: Bin
template<char Op, typename Fst, typename Snd>
struct BinEvalHelper {
  using value = Error<"Arithmetic/logical operation failure">;
};
template<char Op, int N1, int N2> requires (!(Op == '&' || Op == '|'))
struct BinEvalHelper<Op,Int<N1>,Int<N2>> {
  using value = Int<
    (Op == '+') ? (N1 + N2) : (
    (Op == '-') ? (N1 - N2) : (
    (Op == '*') ? (N1 * N2) : (
    (Op == '/') ? (N1 / N2) : (
    (Op == '%') ? (N1 % N2) : 0))))
  >;
};
template<char Op, bool B1, bool B2> requires (Op == '&' || Op == '|')
struct BinEvalHelper<Op,Bool<B1>,Bool<B2>> {
  using value = Bool<
    (Op == '&') ? (B1 && B2) : (
    (Op == '|') ? (B1 || B2) : 0)
  >;
};

template<char Op, LitStr E1, LitStr E2>
struct BinEvalHelper<Op,Error<E1>,Error<E2>> { using value = Error<E1>; };
template<char Op, LitStr E, typename Snd>
struct BinEvalHelper<Op,Error<E>,Snd> { using value = Error<E>; };
template<char Op, typename Fst, LitStr E>
struct BinEvalHelper<Op,Fst,Error<E>> { using value = Error<E>; };

template<char Op, typename Fst, typename Snd, typename Env>
struct Eval<Bin<Op,Fst,Snd>, Env> {
  using value = typename BinEvalHelper<Op, typename Eval<Fst,Env>::value, typename Eval<Snd,Env>::value>::value;
};

// Eval: Unary
template<char Op, typename Exp>
struct UnaryEvalHelper { using value = Error<"Unary arithmetic/logical operator failure">; };
template<char Op, int N> requires (Op != '~')
struct UnaryEvalHelper<Op,Int<N>> {
  using value = Int<
    (Op == '+' || Op == '*') ? N : (
    (Op == '-') ? -N : (
    (Op == '/') ? 1/N : 0))
  >;
};
template<char Op, bool B> requires (Op == '~')
struct UnaryEvalHelper<Op,Bool<B>> {
  using value = Bool<!B>;
};

template<char Op, typename Exp, typename Env>
struct Eval<Unary<Op,Exp>, Env> {
  using value = typename std::conditional<
    IsError<typename Eval<Exp,Env>::value>::value,
    typename Eval<Exp,Env>::value,
    typename UnaryEvalHelper<Op, typename Eval<Exp,Env>::value>::value
  >::type;
};

template<typename> struct IsFalse : std::false_type {};
template<bool B> struct IsFalse<Bool<B>> : std::true_type {};

// Eval: And/Or
template<bool AndIsTrue, typename Env, typename... Juncts> struct Eval<AndOr<AndIsTrue,Juncts...>, Env> {};
template<bool AndIsTrue, typename Env, typename Junct, typename... Rest>
struct Eval<AndOr<AndIsTrue,Junct,Rest...>, Env> {
  using value = typename std::conditional<
    AndIsTrue,
    typename std::conditional< // And
      IsFalse<typename Eval<Junct,Env>::value>::value,
      Bool<false>,
      typename Eval<AndOr<true,Rest...>,Env>::value
    >::type,
    typename std::conditional< // Or
      !IsFalse<typename Eval<Junct,Env>::value>::value,
      typename Eval<Junct,Env>::value,
      typename Eval<AndOr<false,Rest...>,Env>::value
    >::type
  >::type;
};
template<bool AndIsTrue, typename Junct, typename Env>
struct Eval<AndOr<AndIsTrue,Junct>, Env> {
  using value = typename std::conditional<
    AndIsTrue,
    typename std::conditional< // And
      IsFalse<typename Eval<Junct,Env>::value>::value,
      Bool<false>,
      typename Eval<Junct,Env>::value
    >::type,
    typename std::conditional< // Or
      !IsFalse<typename Eval<Junct,Env>::value>::value,
      typename Eval<Junct,Env>::value,
      Bool<false>
    >::type
  >::type;
};
template<bool AndIsTrue, typename Env>
struct Eval<AndOr<AndIsTrue>, Env> {
  using value = Bool<AndIsTrue>;
};
/*
template<typename DefnList, typename Env>
struct LetEnvBuilder {
  using value = Error<"Non-definition appeared in let definition list">;
};
template<LitStr Label, typename Exp, typename... Rest, typename Env>
struct LetEnvBuilder<Begin<Define<Label,Exp>,Rest...>,Env> {
  using value = LetEnvBuilder<Begin<Rest...>,EnvNode<Label,typename Eval<Exp,Env>::value,Env>>
};
template<typename Env>
struct LetEnvBuilder<Begin<>,Env> {
  using value = Env;
};

template<typename DefnList, typename ExpnList, typename Env>
struct Eval<Let<DefnList,ExpnList>, Env> {
  using value = Eval<ExpnList,LetEnvBuilder<DefnList,Env>>;
}
*/

// Pre-defined variables, as per https://docs.racket-lang.org/htdp-langs/intermediate-lam.html
template<typename T>
using DoEval = typename Eval<T,EnvNode<"true",Bool<true>,EnvNode<"false",Bool<false>,EnvNode<"empty",Quoted<QuotedList<>>,EnvEmpty>>>>::value;

template<typename T>
void Print() {
  std::string type{__PRETTY_FUNCTION__};
  std::cout << type.substr(23, type.find_first_of(']')-23) << std::endl;
}

template<typename> struct PrintStmts {
  PrintStmts() { std::cout << "ERROR: Print failure" << std::endl; }
};
template<> struct PrintStmts<EmptyStmt> {
  PrintStmts() {}
};
template<typename Exp, typename Rest>
struct PrintStmts<Stmt<Exp,Rest>> {
  PrintStmts() { Print<Exp>(); PrintStmts<Rest> rest; }
};
template<LitStr E> struct PrintStmts<Error<E>> {
  PrintStmts() { Print<Error<E>>(); }
};
