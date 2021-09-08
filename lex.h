#ifndef _LEX_H_
#define _LEX_H_

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

struct StmtNode {
  virtual std::ostream& print(std::ostream&) const = 0;
  virtual ~StmtNode() {}
  friend std::ostream& operator<<(std::ostream& out, const StmtNode& stmt);
};

struct ExpNode : public StmtNode {
  virtual std::ostream& print(std::ostream&) const = 0;
  virtual ~ExpNode() {}
  virtual ExpNode* clone() const = 0;
  friend std::ostream& operator<<(std::ostream& out, const ExpNode& exp);
};

template<typename T> struct Cloneable : public ExpNode {
  using ExpNode::ExpNode;
  virtual ExpNode* clone() const {
    return new T{static_cast<const T&>(*this)};
  }
};

class DefnNode : public StmtNode {
  public:
    DefnNode(std::string& l, ExpNode& e) : label{l}, exp{e} {}
    ~DefnNode() { delete &label; delete &exp; }
    std::ostream& print(std::ostream&) const;
  protected:
    const std::string& label;
    const ExpNode& exp;
};

class ProgNode {
  public:
    ProgNode(std::vector<StmtNode*>& s) : stmts{s} {}
    ~ProgNode() {
      for (auto& stmt : stmts) delete stmt;
      delete &stmts;
    }
    std::ostream& print(std::ostream&) const;
    friend std::ostream& operator<<(std::ostream& out, const ProgNode& exp);
  protected:
    const std::vector<StmtNode*>& stmts;
};

class IfNode : public Cloneable<IfNode> {
  public:
    IfNode(ExpNode& cond, ExpNode& then, ExpNode& otherwise)
      : cond{cond}, then{then}, otherwise{otherwise} {}
    ~IfNode() { delete &cond; delete &then; delete &otherwise; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const ExpNode& cond;
    const ExpNode& then;
    const ExpNode& otherwise;
};

class IdNode : public Cloneable<IdNode> {
  public:
    IdNode(std::string& s) : id{s} {}
    ~IdNode() { delete &id; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::string& id;
};

class NumNode : public Cloneable<NumNode> {
  public:
    NumNode(int n) : num{n} {}
    std::ostream& print(std::ostream&) const override;
  protected:
    const int num;
};

class BoolNode : public Cloneable<BoolNode> {
  public:
    BoolNode(bool b) : boolean{b} {}
    std::ostream& print(std::ostream&) const override;
  protected:
    const bool boolean;
};

class CharNode : public Cloneable<CharNode> {
  public:
    CharNode(char c) : letter{c} {}
    std::ostream& print(std::ostream&) const override;
  protected:
    const char letter;
};

class AndOrNode : public Cloneable<AndOrNode> {
  public:
    AndOrNode(bool b, std::vector<StmtNode*>& juncts) : and_is_true{b}, juncts{juncts} {}
    ~AndOrNode() {
      for (auto& junct : juncts) delete junct;
      delete &juncts;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const bool and_is_true;
    const std::vector<StmtNode*>& juncts;
};

class PrimOpNode : public Cloneable<PrimOpNode> {
  public:
    PrimOpNode(std::string& op) : op{op} {}
    ~PrimOpNode() { delete &op; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::string& op;
};

class BeginNode : public Cloneable<BeginNode> {
  public:
    BeginNode(std::vector<StmtNode*>& stmts) : stmts{stmts} {}
    ~BeginNode() {
      for (auto& stmt : stmts) delete stmt;
      delete &stmts;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::vector<StmtNode*>& stmts;
};
/*
class LetNode : public Cloneable<LetNode> {
  public:
    LetNode(std::vector<StmtNode*>& defns, std::vector<StmtNode*>& expns) : defns{defns}, expns{expns} {}
    ~LetNode() {
      for (auto& defn : defns) delete defn;
      for (auto& expn : expns) delete expn;
      delete &defns;
      delete &expns;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::vector<StmtNode*>& defns;
    const std::vector<StmtNode*>& expns;
};
*/
struct VoidNode : public Cloneable<VoidNode> {
  std::ostream& print(std::ostream&) const override;
};
/*
struct EmptyNode : public Cloneable<EmptyNode> {
  std::ostream& print(std::ostream&) const override;
};
*/
class QuotedNode : public Cloneable<QuotedNode> {
  public:
    QuotedNode(ExpNode& exp) : exp{exp} {}
    ~QuotedNode() { delete &exp; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const ExpNode& exp;
};

class ListNode : public Cloneable<ListNode> {
  public:
    ListNode(std::vector<StmtNode*>& items) : items{items} {}
    ~ListNode() {
      for (auto& item : items) delete item;
      delete &items;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::vector<StmtNode*>& items;
};

struct QuasiNode : public Cloneable<QuasiNode> {};

class StrNode : public Cloneable<StrNode> {
  public:
    StrNode(std::string& str) : str{str} {}
    ~StrNode() { delete &str; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::string& str;
};

class LambdaNode : public Cloneable<LambdaNode> {
  public:
    LambdaNode(std::vector<std::string*>& l, ExpNode& b) : ids{l}, body{b} {}
    ~LambdaNode() {
      for (auto& id : ids) delete id;
      delete &ids; delete &body;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::vector<std::string*>& ids;
    const ExpNode& body;
};

class AltLambdaNode : public Cloneable<AltLambdaNode> {
  public:
    AltLambdaNode(std::string& s, ExpNode& b) : rest_id{s}, body{b} {}
    ~AltLambdaNode() { delete &rest_id; delete &body; }
    std::ostream& print(std::ostream&) const override;
  protected:
    const std::string& rest_id;
    const ExpNode& body;
};

class AppNode : public Cloneable<AppNode> {
  public:
    AppNode(ExpNode& l, std::vector<StmtNode*>& r) : fun{l}, args{r} {}
    ~AppNode() {
      for (auto& arg : args) delete arg;
      delete &fun; delete &args;
    }
    std::ostream& print(std::ostream&) const override;
  protected:
    const ExpNode& fun;
    const std::vector<StmtNode*>& args;
};

#endif
