#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <lexer.hpp>
#include <stack>
#include <format>
#include <args.hpp>
#include <iostream>

using namespace llvm;

LLVMContext context;
IRBuilder<> builder(context);
Module mod("bfcc_mod", context);

Function *bfcc_internal_create_add_function()
{
  FunctionType *addType = FunctionType::get(builder.getInt64Ty(), {builder.getInt64Ty(), builder.getInt64Ty()}, false);
  Function *addFunction = Function::Create(addType, Function::ExternalLinkage, "__bfcc_add", mod);

  BasicBlock *add = BasicBlock::Create(context, "__bfcc_add_block", addFunction);

  BasicBlock *oldBlock = builder.GetInsertBlock();
  builder.SetInsertPoint(add);

  auto Args = addFunction->arg_begin();
  Value *A = Args++;
  Value *B = Args;

  Value *res = builder.CreateAdd(A, B, "res");

  verifyFunction(*addFunction);
  builder.CreateRet(res);

  builder.SetInsertPoint(oldBlock);

  return addFunction;
}

typedef struct
{
  BasicBlock *cond;
  BasicBlock *body;
  BasicBlock *end;
} WhileBlock;

void compile(Lexer &lexer, Settings &settings)
{

  // create main
  FunctionType *mainType = FunctionType::get(builder.getInt32Ty(), false);
  Function *mainFunction = Function::Create(mainType, Function::ExternalLinkage, "main", mod);

  BasicBlock *entry = BasicBlock::Create(context, "entry", mainFunction);
  builder.SetInsertPoint(entry);

  // create functions
  Function *addFunction = bfcc_internal_create_add_function();

  // extern functions
  Function *llvm_putchar = Function::Create(FunctionType::get(builder.getInt32Ty(), builder.getInt32Ty(), false), Function::ExternalLinkage, "putchar", mod);
  Function *llvm_getchar = Function::Create(FunctionType::get(builder.getInt8Ty(), false), Function::ExternalLinkage, "getchar", mod);
  Function *llvm_malloc = Function::Create(FunctionType::get(builder.getPtrTy(), builder.getInt64Ty(), false), Function::ExternalLinkage, "malloc", mod);
  Function *llvm_free = Function::Create(FunctionType::get(builder.getVoidTy(), builder.getPtrTy(), false), Function::ExternalLinkage, "free", mod);
  FunctionCallee llvm_memset = mod.getOrInsertFunction("memset", builder.getPtrTy(), builder.getPtrTy(), builder.getInt32Ty(), builder.getInt64Ty());

  Value *tape = builder.CreateAlloca(builder.getPtrTy(), nullptr, "tape");
  Value *pointer = builder.CreateAlloca(builder.getPtrTy(), nullptr, "pointer");

  Value *tape_ptr = builder.CreateCall(llvm_malloc, {builder.getInt64(30000)});
  builder.CreateCall(llvm_memset, {tape_ptr});

  builder.CreateStore(tape_ptr, tape);
  builder.CreateStore(tape_ptr, pointer);

  std::stack<WhileBlock> loops;

  Token token = lexer.NextToken();
  do
  {
    if (token.type == TOK_PLUS)
    {
      Value *tmp = builder.CreateLoad(builder.getPtrTy(), pointer);
      builder.CreateStore(builder.CreateAdd(builder.CreateLoad(builder.getInt8Ty(), tmp), builder.getInt8(token.value)), tmp);
    }
    else if (token.type == TOK_MINUS)
    {
      Value *tmp = builder.CreateLoad(builder.getPtrTy(), pointer);
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(builder.getInt8Ty(), tmp), builder.getInt8(token.value)), tmp);
    }
    else if (token.type == TOK_RIGHT)
    {
      builder.CreateStore(builder.CreateAdd(builder.CreateLoad(builder.getInt64Ty(), pointer), builder.getInt64(token.value)), pointer);
    }
    else if (token.type == TOK_LEFT)
    {
      builder.CreateStore(builder.CreateSub(builder.CreateLoad(builder.getInt64Ty(), pointer), builder.getInt64(token.value)), pointer);
    }
    else if (token.type == TOK_OUT)
    {
      Value *ch = builder.CreateLoad(builder.getInt8Ty(), builder.CreateLoad(builder.getPtrTy(), pointer));
      for (int i = 0; i < token.value; i++)
      {
        builder.CreateCall(llvm_putchar, {ch});
      }
    }
    else if (token.type == TOK_IN)
    {
      Value *tmp = builder.CreateLoad(builder.getPtrTy(), pointer);
      Value *ptr = builder.CreateLoad(builder.getInt8Ty(), tmp);
      Value *input = builder.CreateCall(llvm_getchar);
      builder.CreateStore(input, tmp);
    }
    else if (token.type == TOK_WHILE)
    {
      WhileBlock while_block = {0};
      while_block.cond = BasicBlock::Create(context, std::format("while_cond_{}", loops.size()), mainFunction);
      while_block.body = BasicBlock::Create(context, std::format("while_body_{}", loops.size()), mainFunction);
      while_block.end = BasicBlock::Create(context, std::format("while_end_{}", loops.size()), mainFunction);
      builder.CreateBr(while_block.cond);
      builder.SetInsertPoint(while_block.cond);
      builder.CreateCondBr(
          builder.CreateICmpNE(
              builder.CreateLoad(builder.getInt8Ty(), builder.CreateLoad(builder.getPtrTy(), pointer)),
              builder.getInt8(0)),

          while_block.body,
          while_block.end);
      builder.SetInsertPoint(while_block.body);
      loops.push(while_block);
    }
    else if (token.type == TOK_END_WHILE)
    {
      WhileBlock while_block = loops.top();
      builder.CreateBr(while_block.cond);
      builder.SetInsertPoint(while_block.end);
      loops.pop();
    }

    // printf("Token: type: %d value: %d\n", token.type, token.value);
    token = lexer.NextToken();
  } while (token.type != TOK_EOF);

  // end of main
  builder.CreateCall(llvm_free, {tape_ptr});

  // return
  builder.CreateRet(builder.getInt32(0));

  verifyFunction(*mainFunction);
  verifyModule(mod);

  std::string rawFileName = settings.getFileNameNoExtenstion();

  std::error_code EC;
  raw_fd_ostream dest(rawFileName + ".ll", EC);
  if (EC)
  {
    std::cout << "\x1b[1mbfcc:\x1b[0m \x1b[1;31mfatal error:\x1b[0m failed to open " << rawFileName + ".ll" << ": " << EC << "\n";
    exit(1);
  }

  mod.print(dest, nullptr);
  if (settings.compilation_level == CL_IR)
  {
    return;
  }

  int exitcode = 0;
  exitcode = system(std::format("llc {}.ll -o {}.s", rawFileName, rawFileName).c_str());
  if (exitcode != 0)
  {
    std::cout << "\x1b[1mbfcc:\x1b[0m \x1b[1;31mfatal error:\x1b[0m failed to compile IR (exit code: " << exitcode << ")\n";
    exit(1);
  }

  if (settings.compilation_level == CL_ASM)
  {
    goto cleanupLevel1;
  }

  exitcode = system(std::format("as {}.s -o {}.o", rawFileName, rawFileName).c_str());
  if (exitcode != 0)
  {
    std::cout << "\x1b[1mbfcc:\x1b[0m \x1b[1;31mfatal error:\x1b[0m failed to assemble (exit code: " << exitcode << ")\n";
    exit(1);
  }

  if (settings.compilation_level == CL_OBJ)
  {
    goto cleanupLevel2;
  }

  exitcode = system(std::format("gcc {}.o -o {}", rawFileName, rawFileName).c_str());
  if (exitcode != 0)
  {
    std::cout << "\x1b[1mbfcc:\x1b[0m \x1b[1;31mfatal error:\x1b[0m failed to compile object (exit code: " << exitcode << ")\n";
    exit(1);
  }

cleanupLevel3:
  remove((rawFileName + ".o").c_str());
cleanupLevel2:
  remove((rawFileName + ".s").c_str());
cleanupLevel1:
  remove((rawFileName + ".ll").c_str());
}