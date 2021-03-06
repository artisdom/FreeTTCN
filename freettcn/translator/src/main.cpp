//
// Copyright (C) 2007 Mateusz Pusz
//
// This file is part of freettcn (Free TTCN) compiler.

// freettcn is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// freettcn is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with freettcn; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "ttcn3Lexer.h"
#include "ttcn3Parser.h"
#include "translator.h"
#include "logger.h"
#include "dumper.h"
#include "freettcn/tools/exception.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>


namespace freettcn {

  namespace translator {

    static void DisplayRecognitionError(pANTLR3_BASE_RECOGNIZER recognizer, pANTLR3_UINT8 *tokenNames)
    {
      pANTLR3_EXCEPTION ex = recognizer->state->exception;
      
      freettcn::translator::CTranslator &translator = freettcn::translator::CTranslator::Instance();
      unsigned line = ex->line;
      unsigned line2;
      unsigned pos = ex->charPositionInLine + 1;
      unsigned pos2;
      std::stringstream msg;
      std::stringstream msg2;
      std::string note;
      
      // How we determine the next piece is dependent on which thing raised the error.
      switch(recognizer->type) {
      case ANTLR3_TYPE_LEXER:
        {
          pANTLR3_LEXER lexer = (pANTLR3_LEXER)recognizer->super;
          
          if(ex->type == ANTLR3_NO_VIABLE_ALT_EXCEPTION)
            msg << "Cannot match to any predicted input";
          else
            msg << (pANTLR3_UINT8)ex->message;

          ANTLR3_INT32 width;
          
          width = ANTLR3_UINT32_CAST(( (pANTLR3_UINT8)lexer->input->data + (lexer->input->size(lexer->input) )) - (pANTLR3_UINT8)(ex->index));

          if(width >= 1) {
            if(isprint(ex->c))
              msg << " near '" << (char)ex->c << "'";
            else
              msg << " near char(0x" << std::hex << std::setw(2) << (int)ex->c << std::dec << ")";
          }
          else {
            msg << " '<EOF>'";
            
            // prepare second error
            line2 = lexer->rec->state->tokenStartLine;
            pos2 = lexer->rec->state->tokenStartCharPositionInLine;
            //            msg << fileName << ":" << (ANTLR3_UINT32)(lexer->rec->state->tokenStartLine) <<
            //              ":" << (ANTLR3_UINT32)(lexer->rec->state->tokenStartCharPositionInLine) << ": ";
            width = ANTLR3_UINT32_CAST(((pANTLR3_UINT8)lexer->input->data + (lexer->input->size(lexer->input))) - (pANTLR3_UINT8)lexer->rec->state->tokenStartCharIndex);
            if(width >= 1)
              msg2 << "The lexer was matching from: " << std::string((char *)lexer->rec->state->tokenStartCharIndex, 0, 20) << std::endl;
            else
              msg2 << "The lexer was matching from the end of the line" << std::endl;
            
            note = "Above errors indicates a poorly specified lexer RULE or unterminated input element such as: \"STRING[\"]";
          }
        }
        break;

      case ANTLR3_TYPE_PARSER:
        {
          pANTLR3_COMMON_TOKEN token = (pANTLR3_COMMON_TOKEN)ex->token;
          
          if(ex->type == ANTLR3_NO_VIABLE_ALT_EXCEPTION)
            msg << "Cannot match '" << token->getText(token)->chars << "' to any predicted input";
          else {
            msg << (pANTLR3_UINT8)ex->message;
            
            if(token) {
              if (token->type == ANTLR3_TOKEN_EOF)
                msg << " at '<EOF>'";
              else {
                if(ex->type == ANTLR3_MISSING_TOKEN_EXCEPTION) {
                  if(!tokenNames)
                    msg << " [" << ex->expecting << "]";
                  else {
                    if(ex->expecting == ANTLR3_TOKEN_EOF)
                      msg << " '<EOF>'";
                    else
                      msg << " '" << tokenNames[ex->expecting] << "'";
                  }
                }
                else {
                  msg << " near '" << token->getText(token)->chars << "'";
                }
              }
            }
            
            if(ex->type == ANTLR3_UNWANTED_TOKEN_EXCEPTION) {
              if(tokenNames) {
                if(ex->expecting == ANTLR3_TOKEN_EOF)
                  msg << " ('<EOF>' expected)";
                else
                  msg << " ('" << tokenNames[ex->expecting] << "' expected)";
              }
            }
          }
        }
        break;
        
      default:
        std::cerr << "DisplayRecognitionError() called by unknown parser type - provide override for this function" << std::endl;
        return;
      }
      
      switch(ex->type) {
      case ANTLR3_UNWANTED_TOKEN_EXCEPTION:
        // Indicates that the recognizer was fed a token which seesm to be
        // spurious input. We can detect this when the token that follows
        // this unwanted token would normally be part of the syntactically
        // correct stream. Then we can see that the token we are looking at
        // is just something that should not be there and throw this exception.
        //
        if(recognizer->type != ANTLR3_TYPE_PARSER) {
          if(tokenNames == NULL)
            ANTLR3_FPRINTF(stderr, " : Extraneous input...");
          else {
            if(ex->expecting == ANTLR3_TOKEN_EOF)
              ANTLR3_FPRINTF(stderr, " : Extraneous input - expected <EOF>\n");
            else
              ANTLR3_FPRINTF(stderr, " : Extraneous input - expected %s ...\n", tokenNames[ex->expecting]);
          }
        }
        break;
        
      case ANTLR3_MISSING_TOKEN_EXCEPTION:
        // Indicates that the recognizer detected that the token we just
        // hit would be valid syntactically if preceeded by a particular 
        // token. Perhaps a missing ';' at line end or a missing ',' in an
        // expression list, and such like.
        break;
        
      case ANTLR3_RECOGNITION_EXCEPTION:
        // Indicates that the recognizer received a token
        // in the input that was not predicted. This is the basic exception type 
        // from which all others are derived. So we assume it was a syntax error.
        // You may get this if there are not more tokens and more are needed
        // to complete a parse for instance.
        break;
        
      case ANTLR3_MISMATCHED_TOKEN_EXCEPTION:
        // We were expecting to see one thing and got another. This is the
        // most common error if we coudl not detect a missing or unwanted token.
        // Here you can spend your efforts to
        // derive more useful error messages based on the expected
        // token set and the last token and so on. The error following
        // bitmaps do a good job of reducing the set that we were looking
        // for down to something small. Knowing what you are parsing may be
        // able to allow you to be even more specific about an error.
        //
        if(!tokenNames)
          ANTLR3_FPRINTF(stderr, " : syntax error...\n");
        else {
          if(ex->expecting == ANTLR3_TOKEN_EOF)
            ANTLR3_FPRINTF(stderr, " : expected <EOF>\n");
          else
            ANTLR3_FPRINTF(stderr, " : expected %s ...\n", tokenNames[ex->expecting]);
        }
        break;
        
      case ANTLR3_NO_VIABLE_ALT_EXCEPTION:
        // We could not pick any alt decision from the input given
        // so god knows what happened - however when you examine your grammar,
        // you should. It means that at the point where the current token occurred
        // that the DFA indicates nowhere to go from here.
        //
        if(recognizer->type != ANTLR3_TYPE_LEXER && recognizer->type != ANTLR3_TYPE_PARSER)
          ANTLR3_FPRINTF(stderr, " : cannot match to any predicted input...\n");
        break;
        
      case ANTLR3_MISMATCHED_SET_EXCEPTION:
        {
          ANTLR3_UINT32 count;
          ANTLR3_UINT32 bit;
          ANTLR3_UINT32 size;
          ANTLR3_UINT32 numbits;
          pANTLR3_BITSET errBits;
          
          // This means we were able to deal with one of a set of
          // possible tokens at this point, but we did not see any
          // member of that set.
          //
          ANTLR3_FPRINTF(stderr, " : unexpected input...\n  expected one of : ");
          
          // What tokens could we have accepted at this point in the
          // parse?
          //
          count   = 0;
          errBits = antlr3BitsetLoad(ex->expectingSet);
          numbits = errBits->numBits(errBits);
          size    = errBits->size(errBits);
          
          if(size > 0) {
            // However many tokens we could have dealt with here, it is usually
            // not useful to print ALL of the set here. I arbitrarily chose 8
            // here, but you should do whatever makes sense for you of course.
            // No token number 0, so look for bit 1 and on.
            //
            for(bit = 1; bit < numbits && count < 8 && count < size; bit++) {
              // TODO: This doesn;t look right - should be asking if the bit is set!!
              //
              if  (tokenNames[bit]) {
                ANTLR3_FPRINTF(stderr, "%s%s", count > 0 ? ", " : nullptr, tokenNames[bit]); 
                count++;
              }
            }
            ANTLR3_FPRINTF(stderr, "\n");
          }
          else {
            ANTLR3_FPRINTF(stderr, "Actually dude, we didn't seem to be expecting anything here, or at least\n");
            ANTLR3_FPRINTF(stderr, "I could not work out what I was expecting, like so many of us these days!\n");
          }
        }
        break;
        
      case ANTLR3_EARLY_EXIT_EXCEPTION:
        // We entered a loop requiring a number of token sequences
        // but found a token that ended that sequence earlier than
        // we should have done.
        //
        ANTLR3_FPRINTF(stderr, " : missing elements...\n");
        break;
        
      default:
        
        // We don't handle any other exceptions here, but you can
        // if you wish. If we get an exception that hits this point
        // then we are just going to report what we know about the
        // token.
        //
        ANTLR3_FPRINTF(stderr, " : syntax not recognized...\n");
        std::cout << "Unknown exception type: " << ex->type << std::endl;
        break;
      }
      
      translator.Error(CLocation(translator.File(), line, pos), msg.str());
      if(!msg2.str().empty())
        translator.Error(CLocation(translator.File(), line2, pos2), msg2.str());
      if(!note.empty())
        translator.Note(CLocation(translator.File(), line2, pos2), note);
    }

  }  // namespace translator

}  // namespace freettcn


int main(int argc, char *argv[])
{
  try {
    if(argc < 2) {
      std::cerr << "Input file not specified!!!" << std::endl;
      return EXIT_FAILURE;
    }
  
    // check if input file exist
    std::ifstream inFile(argv[1]);
    if(!inFile.is_open()) {
      std::cerr << "Input file '" << argv[1] << "' not found!!!" << std::endl;
      return EXIT_FAILURE;
    }
  
    // create file name
    std::string filePath(argv[1]);
    size_t nameStart = filePath.find_last_of("/\\");
    size_t extStart = filePath.find_last_of(".");
    std::string fileName(filePath, nameStart + 1, extStart - nameStart);
    fileName += "cpp";

    // prepare output stream
    std::ofstream outFile(fileName.c_str());
    if(!outFile.is_open()) {
      std::cerr << "Cannot create output file '" << fileName << "'!!!" << std::endl;
      return EXIT_FAILURE;
    }
  
    // create logger & translator
    freettcn::translator::CLogger logger;
    freettcn::translator::CDumper dumper(outFile);
    freettcn::translator::CTranslator translator(argv[1], logger);
    
    // create input stream
    pANTLR3_UINT8 fName((pANTLR3_UINT8)argv[1]);
    pANTLR3_INPUT_STREAM input = antlr3AsciiFileStreamNew(fName);
    if(!input)
      std::cerr << "Unable to open file " << (char *)fName << " due to malloc() failure1" << std::endl;
  
    // create lexer
    pttcn3Lexer lexer = ttcn3LexerNew(input);
    if(!lexer) {
      std::cerr << "Unable to create the lexer due to malloc() failure1" << std::endl;
      return EXIT_FAILURE;
    }
    // override warning messages for lexer
    lexer->pLexer->rec->displayRecognitionError = freettcn::translator::DisplayRecognitionError;
  
    // create tokens stream from input file
    pANTLR3_COMMON_TOKEN_STREAM tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
    if(!tokens) {
      std::cerr << "Out of memory trying to allocate token stream" << std::endl;
      return EXIT_FAILURE;
    }

    // create parser
    pttcn3Parser parser = ttcn3ParserNew(tokens);
    if(!parser) {
      std::cerr << "Out of memory trying to allocate parser" << std::endl;
      return EXIT_FAILURE;
    }
    // override warning messages for parser
    parser->pParser->rec->displayRecognitionError = freettcn::translator::DisplayRecognitionError;
  
    if(argc >= 3 && std::string(argv[2]) == "-t") {
      // print tokens stream
      pANTLR3_UINT8 *tokenNames = parser->pParser->rec->state->tokenNames;
      if(!tokenNames) {
        std::cerr << "Token names not initiated!!!" << std::endl;
        return EXIT_FAILURE;
      }

      // estimate the longest token name
      size_t maxTokenLength = 0;
      pANTLR3_VECTOR vector = tokens->getTokens(tokens);
      for(unsigned i=0; i<vector->size(vector); i++) {
        pANTLR3_COMMON_TOKEN token = (pANTLR3_COMMON_TOKEN)vector->get(vector, i);
        maxTokenLength = std::max(maxTokenLength, std::string((char *)tokenNames[token->getType(token)]).size());
      }
   
      // print tokens
      for(unsigned i=0; i<vector->size(vector); i++) {
        pANTLR3_COMMON_TOKEN token = (pANTLR3_COMMON_TOKEN)vector->get(vector, i);
        std::cout << "Token[" << std::setw(3) << std::right << token->getType(token) << "] - " <<
          std::setw(maxTokenLength) << std::left << tokenNames[token->getType(token)] << " = " <<
          token->getText(token)->chars << std::endl;
      }
      return EXIT_SUCCESS;
    }
  
    // parse tokes stream
    parser->ttcn3Module(parser);

    
    if(translator.WarningNum() || translator.ErrorNum()) {
      std::cerr << filePath << ": Errors: " << translator.ErrorNum() << "; Warnings: " << translator.WarningNum() << std::endl;
      return EXIT_FAILURE;
    }
    
    // dump to output file
    translator.Dump(dumper);
    
    // must manually clean up
    //    parser->free(parser);
    tokens->free(tokens);
    lexer->free(lexer);
    input->close(input);
  }
  catch(freettcn::Exception &ex) {
    std::cerr << "Error: Unhandled freettcn exception caught:" << std::endl;
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(std::exception &ex) {
    std::cerr << "Error: Unhandled system exception caught:" << std::endl;;
    std::cerr << ex.what() << std::endl;;
    return EXIT_FAILURE;
  }
  catch(...) {
    std::cerr << "Unknown exception caught!!!" << std::endl;
    return EXIT_FAILURE;
  }
  
  return EXIT_SUCCESS;
}
