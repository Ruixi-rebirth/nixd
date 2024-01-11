/// \file
/// \brief AST nodes.
///
/// Declares the AST nodes used by the parser, the nodes may be used in latter
/// stages, for example semantic analysis.
/// It is expected that they may share some nodes, so they are reference
/// counted.

#pragma once

#include "nixf/Basic/Range.h"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace nixf {

class Node {
public:
  enum NodeKind {
    NK_InterpolableParts,
    NK_BeginExpr,
    NK_ExprInt,
    NK_ExprFloat,
    NK_ExprString,
    NK_EndExpr,
  };

private:
  NodeKind Kind;
  OffsetRange Range;

protected:
  explicit Node(NodeKind Kind, OffsetRange Range) : Kind(Kind), Range(Range) {}

public:
  [[nodiscard]] NodeKind getKind() const { return Kind; }
  [[nodiscard]] OffsetRange getRange() const { return Range; }
  [[nodiscard]] const char *getBegin() const { return Range.Begin; }
  [[nodiscard]] const char *getEnd() const { return Range.End; }
};

class Expr : public Node {
protected:
  explicit Expr(NodeKind Kind, OffsetRange Range) : Node(Kind, Range) {
    assert(NK_BeginExpr <= Kind && Kind <= NK_EndExpr);
  }
};

using NixInt = int64_t;
using NixFloat = double;

class ExprInt : public Expr {
  NixInt Value;

public:
  ExprInt(OffsetRange Range, NixInt Value)
      : Expr(NK_ExprInt, Range), Value(Value) {}
  [[nodiscard]] NixInt getValue() const { return Value; }
};

class ExprFloat : public Expr {
  NixFloat Value;

public:
  ExprFloat(OffsetRange Range, NixFloat Value)
      : Expr(NK_ExprFloat, Range), Value(Value) {}
  [[nodiscard]] NixFloat getValue() const { return Value; }
};

class StringPart {
  enum StringPartKind {
    SPK_Escaped,
    SPK_Interpolation,
  } Kind;
  std::string Escaped;
  std::shared_ptr<Expr> Interpolation;

public:
  explicit StringPart(std::string Escaped)
      : Kind(SPK_Escaped), Escaped(std::move(Escaped)), Interpolation(nullptr) {
  }

  explicit StringPart(std::shared_ptr<Expr> Expr)
      : Kind(SPK_Interpolation), Interpolation(std::move(Expr)) {}

  StringPartKind getKind() { return Kind; }
};

class InterpolatedParts : public Node {
  std::vector<StringPart> Fragments;

public:
  InterpolatedParts(OffsetRange Range, std::vector<StringPart> Fragments)
      : Node(NK_InterpolableParts, Range), Fragments(std::move(Fragments)) {}

  [[nodiscard]] const std::vector<StringPart> &getFragments() const {
    return Fragments;
  };
};

class ExprString : public Expr {
  std::shared_ptr<InterpolatedParts> Parts;

public:
  ExprString(OffsetRange Range, std::shared_ptr<InterpolatedParts> Parts)
      : Expr(NK_ExprString, Range), Parts(std::move(Parts)) {}

  [[nodiscard]] const std::shared_ptr<InterpolatedParts> &getParts() const {
    return Parts;
  }
};

} // namespace nixf
