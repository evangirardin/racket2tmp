#include "lex.h"

extern bool church;
extern bool full;

std::ostream& operator<<(std::ostream& out, const ExpNode& exp) {
  return exp.print(out);
}
std::ostream& operator<<(std::ostream& out, const StmtNode& stmt) {
  return stmt.print(out);
}
std::ostream& operator<<(std::ostream& out, const ProgNode& exp) {
  return exp.print(out);
}

std::ostream& IdNode::print(std::ostream& out) const {
  out << "Var<\"" << id << "\">";
  return out;
}

std::ostream& IfNode::print(std::ostream& out) const {
  out << "If<" << cond << ',' << then << ',' << otherwise << '>';
  return out;
}

std::ostream& StrNode::print(std::ostream& out) const {
  out << "String<" << str << ">";
  return out;
}

std::ostream& PrimOpNode::print(std::ostream& out) const {
  out << "PrimOp<\"" << op << "\">";
  return out;
}

std::ostream& ProgNode::print(std::ostream& out) const {
  if (full && stmts.size() > 0) out << "PrintStmts<DoEval<";
  for (int i = 0; i < stmts.size(); ++i) {
    out << "Stmt<" << *stmts[i] << ',';
  }
  out << "EmptyStmt" << std::string(stmts.size(), '>');
  if (full && stmts.size() > 0) out << ">> result;";
  out << std::endl;
  return out;
}

std::ostream& DefnNode::print(std::ostream& out) const {
  out << "Define<\"" << label << "\"," << exp << ">";
  return out;
}

// TODO! USE VARIADIC TEMPLATES
std::ostream& AndOrNode::print(std::ostream& out) const {
  out << "AndOr<" << std::boolalpha << and_is_true;
  for (auto& junct : juncts) out << ',' << *junct;
  out << '>';
  return out;
}

std::ostream& NumNode::print(std::ostream& out) const {
  if (!church)
    out << "Int<" << num << '>';
  else {
    out << "Lambda<\"f\", Lambda<\"x\", ";
    for (int i = 0; i < num; ++i) {
      out << "App<Var<\"f\">, ";
    }
    out << "Var<\"x\">>>" << std::string(num, '>');
  }
  return out;
}

std::ostream& BoolNode::print(std::ostream& out) const {
  if (!church)
    out << "Bool<" << std::boolalpha << boolean << '>';
  else
    out << "Lambda<\"x\",Lambda<\"y\", Var<\"" << std::boolalpha << (boolean ? "x" : "y") << "\">>>";
  return out;
}

std::ostream& CharNode::print(std::ostream& out) const {
  out << "Char<\'" << letter << "\'>";
  return out;
}

std::ostream& VoidNode::print(std::ostream& out) const {
  out << "Void";
  return out;
}

std::ostream& QuotedNode::print(std::ostream& out) const {
  out << "Quoted<" << exp << '>';
  return out;
}

std::ostream& ListNode::print(std::ostream& out) const {
  out << "QuotedList<";
  if (items.size() > 0) out << *items[0];
  for (size_t i = 1; i < items.size(); ++i)
    out << ',' << *items[i];
  out << '>';
  return out;
}

std::ostream& BeginNode::print(std::ostream& out) const {
  out << "Begin<";
  if (stmts.size() > 0) out << *stmts[0];
  else out << "";
  for (size_t i = 1; i < stmts.size(); ++i)
    out << ',' << *stmts[i];
  out << '>';
  return out;
}
/*
std::ostream& LetNode::print(std::ostream& out) const {
  out << "Let<Begin<" << (defns.size() > 0 ? *defns[0] : "");
  for (size_t i = 1; i < defns.size(); ++i)
    out << ',' << *defns[i];
  out << ">,Begin<" << expns[0]; // guaranteed exists, else parser error
  for (size_t i = 1; i < expns.size(); ++i)
    out << ',' << *expns[i];
  out << ">>";
  return out;
}
*/
std::ostream& LambdaNode::print(std::ostream& out) const {
  for (int i = 0; i < ids.size(); ++i) {
    out << "Lambda<\"" << *ids[i] << "\",";
  }
  out << body << std::string(ids.size(), '>');
  return out;
}

std::ostream& AltLambdaNode::print(std::ostream& out) const {
  out << "AltLambda<\"" << rest_id << "\"," << body << '>';
  return out;
}

std::ostream& AppNode::print(std::ostream& out) const {
  out << "App<" << fun;
  for (size_t i = 0; i < args.size(); ++i)
    out << ',' << *args[i];
  out << '>';
  return out;
}
