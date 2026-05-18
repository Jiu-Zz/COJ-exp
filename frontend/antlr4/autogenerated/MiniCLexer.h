
// Generated from MiniC.g4 by ANTLR 4.12.0

#pragma once


#include "antlr4-runtime.h"




class  MiniCLexer : public antlr4::Lexer {
public:
  enum {
    T_L_PAREN = 1, T_R_PAREN = 2, T_L_BRACKET = 3, T_R_BRACKET = 4, T_SEMICOLON = 5, 
    T_L_BRACE = 6, T_R_BRACE = 7, T_ASSIGN = 8, T_COMMA = 9, T_EQ = 10, 
    T_NE = 11, T_LT = 12, T_LE = 13, T_GT = 14, T_GE = 15, T_AND = 16, T_OR = 17, 
    T_NOT = 18, T_ADD = 19, T_SUB = 20, T_MUL = 21, T_DIV = 22, T_MOD = 23, 
    T_RETURN = 24, T_IF = 25, T_ELSE = 26, T_WHILE = 27, T_BREAK = 28, T_CONTINUE = 29, 
    T_INT = 30, T_VOID = 31, T_ID = 32, T_CONST = 33, WS = 34, LINE_COMMENT = 35, 
    BLOCK_COMMENT = 36
  };

  explicit MiniCLexer(antlr4::CharStream *input);

  ~MiniCLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

